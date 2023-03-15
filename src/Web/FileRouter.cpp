#include "FileRouter.h"
#include "Ty/StringBuffer.h"
#include <Core/MappedFile.h>
#include <Ty/System.h>
#include <sys/inotify.h>
#include <unistd.h>

namespace Web {

ErrorOr<FileRouter> FileRouter::create()
{
    auto filewatch_fd = inotify_init1(IN_NONBLOCK);
    if (filewatch_fd < 0) {
        return Error::from_errno();
    }

    return FileRouter {
        TRY(StaticRoutes::create()),
        TRY(Files::create()),
        TRY(WatchFileMap::create()),
        filewatch_fd,
    };
}

void FileRouter::destroy() const
{
    System::close(m_filewatch_fd).ignore();
}

ErrorOr<void> FileRouter::add_route(StringView route,
    StringView filename)
{
    TRY(m_static_routes.append(route, filename));
    TRY(m_files.append(filename, TRY(File::open(filename))));

    auto name_buf
        = TRY(StringBuffer::create_fill(filename, "\0"sv));
    auto watch_file = inotify_add_watch(m_filewatch_fd,
        name_buf.data(), IN_MODIFY);
    if (watch_file < 0) {
        return Error::from_errno();
    }
    TRY(m_watch_file_map.append(watch_file, filename));
    return {};
}

ErrorOr<void> FileRouter::reload_files_if_needed(Core::File& log)
{
    struct inotify_event event;
    while (true) {
        auto rv = ::read(m_filewatch_fd, &event, sizeof(event));
        if (rv < 0) {
            if (errno == EAGAIN) {
                return {};
            }
            return Error::from_errno();
        }
        auto path_id = m_watch_file_map.find(event.wd);
        if (!path_id.has_value()) {
            return Error::from_string_literal(
                "could not find name of watched file");
        }
        auto path = m_watch_file_map[path_id.value()];
        log.writeln("reloading \""sv, path, "\""sv).ignore();
        TRY(reload_file(path));
    }
}

ErrorOr<void> FileRouter::reload_file(StringView path)
{
    auto id = m_files.find(path);
    if (!id.has_value())
        return Error::from_string_literal("invalid file reload");
    TRY(m_files[id.value()].reload());
    return {};
}

Optional<Id<File>> FileRouter::find(StringView route) const
{
    auto filename_id = m_static_routes.find(route);
    if (!filename_id.has_value())
        return {};
    return m_files.find(m_static_routes[filename_id.value()]);
}

}
