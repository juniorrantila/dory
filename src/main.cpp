#include <CLI/ArgumentParser.h>
#include <Core/MappedFile.h>
#include <Core/Print.h>
#include <Core/System.h>
#include <HTTP/Headers.h>
#include <HTTP/Response.h>
#include <Main/Main.h>
#include <Net/TCPListener.h>
#include <Ty/Defer.h>
#include <Ty/Parse.h>
#include <Ty/SmallCapture.h>
#include <Ty/SmallMap.h>
#include <Ty/StringBuffer.h>
#include <Ty/StringView.h>
#include <Web/File.h>
#include <Web/FileRouter.h>
#include <Web/MimeType.h>

using Renderer = SmallCapture<ErrorOr<HTTP::Response>()>;
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
            port_or_error = Parse<u16>::from(port).or_throw(
                Error::from_string_literal("invalid port number", "argument_parser"));
        }));

    auto static_folder_path = StringView();
    TRY(argument_parser.add_positional_argument("static-folder"sv, [&](auto argument){
        static_folder_path = StringView::from_c_string(argument);
    }));

    if (auto result = argument_parser.run(argc, argv);
        result.is_error()) {
        TRY(result.error().show());
        return 1;
    }
    auto port = TRY(port_or_error);

    auto file_router = TRY(Web::FileRouter::create());

    auto index_path = TRY(StringBuffer::create_fill(static_folder_path, "/index.html"sv));
    TRY(file_router.add_route("/"sv, index_path.view()));

    auto manifest_path = TRY(StringBuffer::create_fill(static_folder_path, "/manifest.json"sv));
    TRY(file_router.add_route("/manifest.json"sv, manifest_path.view()));

    auto favicon_path = TRY(StringBuffer::create_fill(static_folder_path, "/icons/favicon.ico"sv));
    TRY(file_router.add_route("/favicon.ico"sv, favicon_path.view()));

    auto favicon_192x192_path = TRY(StringBuffer::create_fill(static_folder_path, "/icons/favicon-192x192.png"sv));
    TRY(file_router.add_route("/icons/favicon-192x192.png"sv, favicon_192x192_path.view()));

    auto favicon_512x512_path = TRY(StringBuffer::create_fill(static_folder_path, "/icons/favicon-512x512.png"sv));
    TRY(file_router.add_route("/icons/icon-512x512.png"sv, favicon_512x512_path.view()));

    auto pico_css_path = TRY(StringBuffer::create_fill(static_folder_path, "/css/pico.slim.min.css"sv));
    TRY(file_router.add_route("/css/pico.slim.min.css"sv, pico_css_path.view()));

    auto pico_css_map_path = TRY(StringBuffer::create_fill(static_folder_path, "/css/pico.slim.min.css.map"sv));
    TRY(file_router.add_route("/css/pico.slim.min.css.map"sv, pico_css_map_path.view()));

    auto apple_touch_icon_path = TRY(StringBuffer::create_fill(static_folder_path, "/icons/apple-touch-icon.png"sv));
    TRY(file_router.add_route("/apple-touch-icon.png"sv, apple_touch_icon_path.view()));

    auto dynamic_router = DynamicRouter();

    auto foo_json_path = TRY(StringBuffer::create_fill(static_folder_path, "/json/foo.json"sv));
    auto foo_json_file = TRY(Web::File::open(foo_json_path.view()));
    TRY(dynamic_router.append("/json/foo.json"sv, [&]() -> ErrorOr<HTTP::Response> {
        TRY(foo_json_file.reload());
        return HTTP::Response {
            .body = json_file.view(),
            .charset = json_file.charset(),
            .extra_headers = (
                "Access-Control-Allow-Origin: *\r\n"
                "Access-Control-Allow-Methods: *\r\n"
                "Access-Control-Allow-Headers: *\r\n"
                ""sv
            ),
            .mime_type = json_file.mime_type(),
            .code = HTTP::ResponseCode::Ok,
        };
    }));

    TRY(dynamic_router.append("/panic"sv, [&]() -> ErrorOr<HTTP::Response> {
        return Error::from_string_literal("panic!");
    }));

    auto& log = Core::File::stderr();

    log.writeln("Serving on port: "sv, port).ignore();

    TRY(setup_zombie_reaper());
    auto server = TRY(Net::TCPListener::create(port));
    while (true) {
        auto client = TRY(server.accept());

        if (TRY(Core::System::fork()) > 0)
            continue;
        server.destroy().ignore();

        auto client_name = TRY(client.printable_address());
        log.writeln(client_name.view(), " connected"sv).ignore();
        Defer print_disconnect = [&] {
            log.writeln("dropped "sv, client_name.view()).ignore();
        };
        handle_connection({
            .client = client,
            .log = log,
            .file_router = file_router,
            .dynamic_router = dynamic_router,
            .static_folder_path = static_folder_path,
        }).or_else([&](auto error) {
            log.writeln("Error: "sv, error).ignore();
        });
        return 0;
    }

    return 0;
}

static ErrorOr<void> setup_zombie_reaper()
{
    struct sigaction sa;
    sa.sa_handler = [](auto) {
        // waitpid() might overwrite errno, so we save and restore it:
        int saved_errno = errno;
        while(waitpid(-1, NULL, WNOHANG) > 0);
        errno = saved_errno;
    };
    TRY(Core::System::sigemptyset(&sa.sa_mask));
    sa.sa_flags = SA_RESTART;
    TRY(Core::System::sigaction(SIGCHLD, &sa, nullptr));
    return {};
}

static ErrorOr<void> handle_connection(Connection const& args)
{
    auto raw_request = TRY(args.client.read());
    args.log.writeln("\nrequest:\n"sv, raw_request.view(), "request end\n"sv).ignore();

    if (raw_request.view().is_empty()) {
        TRY(args.client.write(HTTP::Response {
            .extra_headers = "Accept: */*\r\n"sv,
            .code = HTTP::ResponseCode::Continue,
        }));
        return {};
    }

    auto headers = TRY(HTTP::Headers::create_from(raw_request.view()));

    args.log.writeln("parsed get: "sv, headers.get()).ignore();

    auto maybe_get = TRY(headers.get());
    if (!maybe_get.has_value()) {
        TRY(args.client.write(HTTP::Response {
            .body = "not found"sv,
            .code = HTTP::ResponseCode::NotFound,
        }));
        return {};
    }
    auto get = maybe_get.release_value();

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
        TRY(args.client.write(TRY(route().or_else([&](auto error) -> ErrorOr<HTTP::Response> {
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
