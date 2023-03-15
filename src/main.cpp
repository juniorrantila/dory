#include <CLI/ArgumentParser.h>
#include <Core/File.h>
#include <HTTP/Headers.h>
#include <HTTP/Response.h>
#include <Main/Main.h>
#include <Net/TCPConnection.h>
#include <Net/TCPListener.h>
#include <Ty/Defer.h>
#include <Ty/Parse.h>
#include <Ty/SmallCapture.h>
#include <Ty/SmallMap.h>
#include <Ty/StringView.h>
#include <Web/FileRouter.h>

using Renderer
    = SmallCapture<ErrorOr<HTTP::Response>(HTTP::Headers const&)>;
using DynamicRouter = Ty::SmallMap<StringView, Renderer>;

static ErrorOr<void> setup_zombie_reaper();

struct Connection {
    Net::TCPConnection& client;
    Core::File& log;
    Web::FileRouter& file_router;
    DynamicRouter const& dynamic_router;
    StringView static_folder_path;
};
static ErrorOr<void> handle_connection(Connection const& args);

ErrorOr<int> Main::main(int argc, c_string argv[])
{
    auto argument_parser = CLI::ArgumentParser();

    c_string program_name = argv[0];
    TRY(argument_parser.add_flag("--help"sv, "-h"sv,
        "show help message"sv, [&] {
            argument_parser.print_usage_and_exit(program_name, 0);
        }));

    auto port_or_error = ErrorOr<u16>(8080);
    TRY(argument_parser.add_option("--port"sv, "-p"sv, "number"sv,
        "Port to use (default: 8080)"sv, [&](auto argument) {
            auto port = StringView::from_c_string(argument);
            port_or_error = Parse<u16>::from(port).or_throw([] {
                return Error::from_string_literal(
                    "invalid port number", "argument_parser");
            });
        }));

    auto static_folder_path = StringView();
    TRY(argument_parser.add_positional_argument("static-folder"sv,
        [&](auto argument) {
            static_folder_path
                = StringView::from_c_string(argument);
        }));

    if (auto result = argument_parser.run(argc, argv);
        result.is_error()) {
        TRY(result.error().show());
        return 1;
    }
    auto port = TRY(port_or_error);

    auto file_router = TRY(Web::FileRouter::create());

    auto index_path = TRY(StringBuffer::create_fill(
        static_folder_path, "/index.html"sv));
    TRY(file_router.add_route("/"sv, index_path.view()));

    auto script = TRY(StringBuffer::create_fill(static_folder_path,
        "/script.js"sv));
    TRY(file_router.add_route("/script.js"sv, script.view()));

    auto dynamic_router = DynamicRouter();

    TRY(dynamic_router.append("/panic"sv,
        [&](auto&) -> ErrorOr<HTTP::Response> {
            return Error::from_string_literal("panic!");
        }));

    auto& log = Core::File::stderr();

    log.writeln("Serving on port: "sv, port).ignore();

    TRY(setup_zombie_reaper());
    auto server = TRY(Net::TCPListener::create(port));
    while (true) {
        auto client = TRY(server.accept());

        if (TRY(System::fork()) > 0)
            continue;
        server.destroy().ignore();

        // clang-format off
        handle_connection({
            .client = client,
            .log = log,
            .file_router = file_router,
            .dynamic_router = dynamic_router,
            .static_folder_path = static_folder_path,
        }).or_else([&](auto error) {
            log.writeln("Error: "sv, error).ignore();
        });
        // clang-format on
        return 0;
    }

    return 0;
}

static ErrorOr<void> handle_connection(Connection const& args)
{
    auto client_name = TRY(args.client.printable_address());
    args.log.writeln(client_name.view(), " connected"sv).ignore();
    Defer print_disconnect = [&] {
        args.log.writeln("dropped "sv, client_name.view()).ignore();
    };

    // clang-format off
    auto raw_request = TRY(args.client.read());
    args.log.writeln("\nrequest:\n"sv, raw_request.view(), "request end\n"sv).ignore();
    // clang-format on

    if (raw_request.view().is_empty()) {
        TRY(args.client.write(HTTP::Response {
            .extra_headers = "Accept: */*\r\n"sv,
            .code = HTTP::ResponseCode::Continue,
        }));
        return {};
    }

    auto headers
        = TRY(HTTP::Headers::create_from(raw_request.view()));
    if (auto maybe_post = TRY(headers.post()); maybe_post) {
        auto post = maybe_post.value();
        if (auto id = args.dynamic_router.find(post.slug); id) {
            auto route = args.dynamic_router[id.value()];
            auto error_buffer = TRY(StringBuffer::create());
            // clang-format off
            TRY(args.client.write(TRY(route(headers).or_else([&](auto error) -> ErrorOr<HTTP::Response> {
                error_buffer.clear();
                TRY(error_buffer.write(error));
                return HTTP::Response {
                    .body = error_buffer.view(),
                    .extra_headers = (
                        "Access-Control-Allow-Origin: *\r\n"
                        "Access-Control-Allow-Methods: *\r\n"
                        "Access-Control-Allow-Headers: *\r\n"
                        ""sv
                    ),
                    .code = HTTP::ResponseCode::InternalServerError,
                };
            }).or_else([](auto) {
                return HTTP::Response {
                    .body = "Could not report error"sv,
                    .extra_headers = (
                        "Access-Control-Allow-Origin: *\r\n"
                        "Access-Control-Allow-Methods: *\r\n"
                        "Access-Control-Allow-Headers: *\r\n"
                        ""sv
                    ),
                    .code = HTTP::ResponseCode::InternalServerError,
                };
            }))));
            // clang-format on 
            return {};
        }

        auto not_found_path = TRY(StringBuffer::create_fill(args.static_folder_path, "/error/404.html"sv));
        auto not_found_file = TRY(Web::File::open(not_found_path.view()));
        TRY(args.client.write(HTTP::Response {
            .body = not_found_file.view(),
            .mime_type = not_found_file.mime_type(),
            .code = HTTP::ResponseCode::NotFound,
        }));
        return {};
    }

    auto maybe_get = TRY(headers.get());
    if (!maybe_get.has_value()) {
        TRY(args.client.write(HTTP::Response {
            .body = "not found"sv,
            .code = HTTP::ResponseCode::NotFound,
        }));
        return {};
    }
    auto get = maybe_get.release_value();
    TRY(args.log.writeln("parsed get: "sv, get));

    if (auto id = args.file_router.find(get.slug); id) {
        TRY(args.file_router.reload_files_if_needed(args.log));
        auto const& file = args.file_router[id.value()];
        TRY(args.client.write(HTTP::Response {
            .body = file.view(),
            .charset = file.charset(),
            .mime_type = file.mime_type(),
            .code = HTTP::ResponseCode::Ok,
        }));
        return {};
    }

    if (auto id = args.dynamic_router.find(get.slug); id) {
        auto route = args.dynamic_router[id.value()];
        auto error_buffer = TRY(StringBuffer::create());
        // clang-format off
        TRY(args.client.write(TRY(route(headers).or_else([&](auto error) -> ErrorOr<HTTP::Response> {
            error_buffer.clear();
            TRY(error_buffer.write(error));
            return HTTP::Response {
                .body = error_buffer.view(),
                .code = HTTP::ResponseCode::InternalServerError,
            };
        }).or_else([](auto) {
            return HTTP::Response {
                .body = "Could not report error"sv,
                .code = HTTP::ResponseCode::InternalServerError,
            };
        }))));
        // clang-format on 
        return {};
    }

    auto not_found_path = TRY(StringBuffer::create_fill(args.static_folder_path, "/error/404.html"sv));
    auto not_found_file = TRY(Web::File::open(not_found_path.view()));
    TRY(args.client.write(HTTP::Response {
        .body = not_found_file.view(),
        .mime_type = not_found_file.mime_type(),
        .code = HTTP::ResponseCode::NotFound,
    }));
    return {};
};

static ErrorOr<void> setup_zombie_reaper()
{
    struct sigaction sa;
    sa.sa_handler = [](auto) {
        // waitpid() might overwrite errno, so we save and restore
        // it:
        int saved_errno = errno;
        while (waitpid(-1, NULL, WNOHANG) > 0)
            ;
        errno = saved_errno;
    };
    TRY(System::sigemptyset(&sa.sa_mask));
    sa.sa_flags = SA_RESTART;
    TRY(System::sigaction(SIGCHLD, &sa, nullptr));
    return {};
}

