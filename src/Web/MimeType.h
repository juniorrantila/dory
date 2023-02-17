#pragma once
#include <Ty/Formatter.h>

namespace Web {

enum class MimeType {
    ApplicationJson,
    ApplicationOctetStream,
    ImageIco,
    ImagePng,
    TextCss,
    TextHtml,
    TextJavascript,
    TextMarkdown,
    TextPlain,
};
StringView mime_type_string(MimeType type);

}

template <>
struct Ty::Formatter<Web::MimeType> {
    template <typename U>
    requires Writable<U>
    static constexpr ErrorOr<u32> write(U& to, Web::MimeType mime_type)
    {
        return TRY(to.write(Web::mime_type_string(mime_type)));
    }
};
