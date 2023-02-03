#pragma once
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

struct Headers {
    static ErrorOr<Headers> create_from(StringView source);
    ErrorOr<Optional<Get>> get() const;

private:
    constexpr Headers(StringView source)
        : source(source)
    {
    }

    StringView source;
};

}
