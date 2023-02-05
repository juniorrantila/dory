#pragma once
#include <Ty/Forward.h>
#include <Ty/Concepts.h>
#include <Ty/StringView.h>
#include <Ty/ErrorOr.h>

namespace HTTP {

enum class ResponseCode : u16 {
    Ok = 200,
    NotFound = 404,
    InternalServerError = 500,
};
StringView response_code_string(ResponseCode);

struct Response {
    StringView mime_type { "text/plain"sv };
    StringView body { ""sv };
    StringView extra_headers { ""sv };
    ResponseCode code { ResponseCode::Ok };
};

}

template <>
struct Ty::Formatter<HTTP::ResponseCode> {
    template <typename U>
    requires Writable<U>
    static constexpr ErrorOr<u32> write(U& to, HTTP::ResponseCode code)
    {
        return TRY(to.write((u16)code, " "sv, HTTP::response_code_string(code)));
    }
};

template <>
struct Ty::Formatter<HTTP::Response> {
    template <typename U>
    requires Writable<U>
    static constexpr ErrorOr<u32> write(U& to, HTTP::Response const& response)
    {
        u32 size = 0;

        size += TRY(to.write("HTTP/1.1 "sv, response.code, "\r\n"sv));
        size += TRY(to.write("Content-Type: "sv, response.mime_type, "\r\n"sv));
        size += TRY(to.write("Content-Length: "sv, response.body.size, "\r\n"sv));
        size += TRY(to.write("Server: Nemo\r\n"sv));
        size += TRY(to.write(response.extra_headers));
        size += TRY(to.write("\r\n"sv, response.body));

        return size;
    }
};
