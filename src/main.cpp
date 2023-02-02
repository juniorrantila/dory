#include <CLI/ArgumentParser.h>
#include <Core/Print.h>
#include <Core/TCPListener.h>
#include <HTML/Headers.h>
#include <Main/Main.h>
#include <Ty/Parse.h>
#include <Ty/SmallCapture.h>
#include <Ty/SmallMap.h>

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

    if (auto result = argument_parser.run(argc, argv);
        result.is_error()) {
        TRY(result.error().show());
        return 1;
    }
    auto port = TRY(port_or_error);

    auto http_header = "HTTP/1.0 200 OK\r\n\r\n"sv;
    auto http_footer = "\r\n\r\n"sv;

    using PageRenderer = SmallCapture<StringView()>;
    using Router = Ty::SmallMap<StringView, PageRenderer>;
    auto router = Router();
    TRY(router.append("/404"sv, [] {
        return (
            "<html>"
            "<head>"
                "<link"
                    " rel=\"stylesheet\""
                    " href=\"https://unpkg.com/@picocss/pico@1.*/css/pico.min.css\""
                ">"
            "</head>"
            "<body>"
                "<h1 style=\"text-align: center;\">"
                    "Could not find page :("
                "</h1>"
            "</body>"
            "</html>"sv
        );
    }));
    TRY(router.append("/"sv, [] {
        return (
            "<html>"
            "<head>"
                "<link"
                    " rel=\"stylesheet\""
                    " href=\"https://unpkg.com/@picocss/pico@1.*/css/pico.min.css\""
                ">"
            "</head>"
            "<body>"
                "<h1 style=\"text-align: center;\">"
                    "Hello, World!"
                "</h1>"
            "</body>"
            "</html>"sv
        );
    }));

    dbgln("Serving on port: "sv, port);
    auto server = TRY(Core::TCPListener::create(port));
    while (true) {
        auto client = TRY(server.accept());
        auto message = TRY(client.read());
        auto headers = TRY(HTML::Headers::create_from(message));
        auto get = TRY(headers.get()).or_else("/404"sv);
        auto page_id = TRY(router.find(get.slug)
                          .or_else(router.find("/404"sv))
                          .or_throw(
                              Error::from_string_literal("404 page is missing")
                          ));
        TRY(client.write(
            http_header,
            router[page_id](),
            http_footer
        ));
        TRY(client.flush_write());
    }

    return 0;
}
