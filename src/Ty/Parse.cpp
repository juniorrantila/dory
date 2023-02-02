#include "Parse.h"
#include "Limits.h"
#include "Optional.h"
#include "StringView.h"

namespace Ty {

namespace {

Optional<u8> character_to_number(char character);

}

template <>
Optional<u16> Parse<u16>::from(StringView from)
{
    u32 result = 0;
    for (u32 i = 0; i < from.size; i++) {
        result *= 10;
        auto maybe_number = character_to_number(from[i]);
        if (!maybe_number.has_value())
            return {};
        result += maybe_number.value();
        if (result > Limits<u16>::max())
            return {};
    }
    return result;
}

namespace {

Optional<u8> character_to_number(char character)
{
    switch (character) {
    case '0' ... '9': return character - '0';
    default: return {};
    }
}

}

}
