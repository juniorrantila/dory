#pragma once
#include <Core/MappedFile.h>
#include <Ty/StringBuffer.h>

namespace Web {

struct File {

    static ErrorOr<File> open(StringView path);

    ErrorOr<void> reload();

    StringView mime_type() const;
    StringView view() const { return m_file.view(); }

private:
    File(Core::MappedFile&& file, StringView path);

    Core::MappedFile m_file;
    StringView m_path;
};

}
