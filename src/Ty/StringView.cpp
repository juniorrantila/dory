#include "StringView.h"
#include "ErrorOr.h"
#include "Vector.h"

namespace Ty {

ErrorOr<Vector<u32>> StringView::find_all(char character) const
{
    auto occurrences = TRY(Vector<u32>::create());

    for (u32 i = 0; i < size; i++) {
        if (data[i] == character)
            TRY(occurrences.append(i));
    }

    return occurrences;
}

ErrorOr<Vector<u32>> StringView::find_all(StringView sequence) const
{
    auto occurrences = TRY(Vector<u32>::create());

    for (u32 i = 0; i < size;) {
        if (sub_view(i, sequence.size) == sequence) {
            TRY(occurrences.append(i));
            i += sequence.size;
            continue;
        }
        i++;
    }

    return occurrences;
}

ErrorOr<Vector<StringView>> StringView::split_on(
    char character) const
{
    auto indexes = TRY(find_all(character));
    if (indexes.is_empty())
        return Vector<StringView>();

    auto splits = TRY(Vector<StringView>::create());

    u32 last_index = 0xFFFFFFFF; // Intentional overflow
    for (auto index : indexes) {
        TRY(splits.append(part(last_index + 1, index)));
        last_index = index;
    }
    TRY(splits.append(part(last_index + 1, size)));

    return splits;
}

ErrorOr<Vector<StringView>> StringView::split_on(
    StringView sequence) const
{
    auto indexes = TRY(find_all(sequence));
    if (indexes.is_empty())
        return Vector<StringView>();

    auto splits = TRY(Vector<StringView>::create());

    u32 last_index = 0xFFFFFFFF; // Intentional overflow
    for (auto index : indexes) {
        TRY(splits.append(part(last_index + sequence.size, index)));
        last_index = index;
    }
    TRY(splits.append(part(last_index + sequence.size, size)));

    return splits;
}

}
