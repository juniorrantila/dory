#pragma once
#include <Ty/Formatter.h>

namespace Web {

struct MimeType {
    enum Type : u8 {
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
    constexpr MimeType(Type type)
        : m_type(type)
    {
    }

    StringView name() const;
    constexpr Type type() const { return m_type; }

private:
    Type m_type;
};

}

template <>
struct Ty::Formatter<Web::MimeType> {
    template <typename U>
    requires Writable<U>
    static constexpr ErrorOr<u32> write(U& to, Web::MimeType mime_type)
    {
        return TRY(to.write(mime_type.name()));
    }
};
