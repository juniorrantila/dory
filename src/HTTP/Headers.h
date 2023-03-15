#pragma once
#include <Ty/Concepts.h>
#include <Ty/ErrorOr.h>
#include <Ty/Optional.h>

namespace HTTP {

struct Get {
    StringView slug;

    constexpr Get(StringView slug)
        : slug(slug)
    {
    }
};

struct Post {
    StringView slug;

    constexpr Post(StringView slug)
        : slug(slug)
    {
    }
};

struct Headers {
    static ErrorOr<Headers> create_from(StringView source);
    ErrorOr<Optional<Get>> get() const;
    ErrorOr<Optional<Post>> post() const;
    ErrorOr<Optional<StringView>> body() const;

private:
    constexpr Headers(StringView source)
        : source(source)
    {
    }

    StringView source;
};

}

template <>
struct Ty::Formatter<HTTP::Get> {
    template <typename U>
    requires Writable<U>
    static constexpr ErrorOr<u32> write(U& to, HTTP::Get const& get)
    {
        return TRY(to.write("HTTP::Get(\""sv, get.slug, "\")"sv));
    }
};
