#include "File.h"
#include <Core/MappedFile.h>

namespace Web {

ErrorOr<File> File::create(StringView path)
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

StringView File::mime_type() const
{
    if (m_path.ends_with(".png"sv))
        return "image/png"sv;
    if (m_path.ends_with(".html"sv))
        return "text/html"sv;
    if (m_path.ends_with(".css"sv))
        return "text/css"sv;
    if (m_path.ends_with(".js"sv))
        return "text/javascript"sv;
    if (m_path.ends_with(".json"sv))
        return "application/json"sv;
    if (m_path.ends_with(".ico"sv))
        return "image/vnd.microsoft.icon"sv;
    if (m_path.ends_with(".map"sv))
        return "application/json"sv;
    if (m_path.ends_with(".txt"sv))
        return "text/plain"sv;
    return "application/octet-stream"sv;
}

}
