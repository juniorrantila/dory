#include "Headers.h"
#include <Ty/Vector.h>

namespace HTTP {

ErrorOr<Headers> Headers::create_from(StringView source)
{
    return Headers(source);
}

ErrorOr<Optional<Get>> Headers::get() const
{
    auto lines = TRY(source.split_on('\n'));
    for (auto line : lines) {
        if (line.starts_with("GET "sv)) {
            auto parts = TRY(line.split_on(' '));
            if (parts.size() < 2) {
                return Error::from_string_literal(
                    "not enough arguments in GET");
            }
            return Optional<Get>(Get(parts[1]));
        }
    }
    return Optional<Get> {};
}

ErrorOr<Optional<Post>> Headers::post() const
{
    auto lines = TRY(source.split_on('\n'));
    for (auto line : lines) {
        if (line.starts_with("POST "sv)) {
            auto parts = TRY(line.split_on(' '));
            if (parts.size() < 2) {
                return Error::from_string_literal(
                    "not enough arguments in POST");
            }
            return Optional<Post>(Post(parts[1]));
        }
    }
    return Optional<Post> {};
}

ErrorOr<Optional<StringView>> Headers::body() const
{
    auto lines = TRY(source.split_on("\r\n\r\n"sv));
    if (lines.size() < 2)
        return Optional<StringView> {};
    return Optional<StringView>(lines[1]);
}

}
