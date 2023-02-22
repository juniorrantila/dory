#include "MimeType.h"

namespace Web {

StringView MimeType::name() const
{
    switch(m_type) {
    case ApplicationJson:
        return "application/json"sv;
    case ApplicationOctetStream:
        return "application/octet-stream"sv;
    case ImageIco:
        return "image/vnd.microsoft.icon"sv;
    case ImagePng:
        return "image/png"sv;
    case TextCss:
        return "text/css"sv;
    case TextHtml:
        return "text/html"sv;
    case TextJavascript:
        return "text/javascript"sv;
    case TextMarkdown:
        return "text/markdown"sv;
    case TextPlain:
        return "text/plain"sv;
    }
}

}
