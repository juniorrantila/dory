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

}
