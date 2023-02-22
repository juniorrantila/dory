#pragma once
#include "Ty/Forward.h"
#include <Ty/StringView.h>
#include <Ty/Concepts.h>
#include <Ty/ErrorOr.h>

namespace HTTP {

struct Method {
    enum Type : u8 {
        Get,
        Post,
    };
    constexpr Method(Type type)
        : m_type(type)
    {
    }

    StringView name() const;
    Type type() const { return m_type; }

private:
    Type m_type;
};

struct Request {
    StringView slug;
    Method method;
};

}

template <>
struct Ty::Formatter<HTTP::Method> {
    template <typename U>
    requires Writable<U>
    static constexpr ErrorOr<u32> write(U& to, HTTP::Method method)
    {
        return TRY(to.write(method.name()));
    }
};

template <>
struct Ty::Formatter<HTTP::Request> {
    template <typename U>
    requires Writable<U>
    static constexpr ErrorOr<u32> write(U& to, HTTP::Request request)
    {
        return TRY(to.write(request.method, " "sv, request.slug, "\r\n\r\n"sv));
    }
};
