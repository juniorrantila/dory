#include "File.h"
#include <Core/MappedFile.h>

namespace Web {

ErrorOr<File> File::open(StringView path)
{
    return File {
        TRY(Core::MappedFile::open(path)),
        path,
    };
}

File::File(Core::MappedFile&& file, StringView path)
    : m_file(move(file))
    , m_path(path)
{
}

ErrorOr<void> File::reload()
{
    m_file = TRY(Core::MappedFile::open(m_path));
    return {};
}

MimeType File::mime_type() const
{
    if (m_path.ends_with(".png"sv))
        return MimeType::ImagePng;
    if (m_path.ends_with(".html"sv))
        return MimeType::TextHtml;
    if (m_path.ends_with(".css"sv))
        return MimeType::TextCss;
    if (m_path.ends_with(".js"sv))
        return MimeType::TextJavascript;
    if (m_path.ends_with(".json"sv))
        return MimeType::ApplicationJson;
    if (m_path.ends_with(".ico"sv))
        return MimeType::ImageIco;
    if (m_path.ends_with(".map"sv))
        return MimeType::ApplicationJson;
    if (m_path.ends_with(".txt"sv))
        return MimeType::TextPlain;
    return MimeType::ApplicationOctetStream;
}

}
