#pragma once
#include "File.h"
#include <Core/File.h>
#include <HTTP/Response.h>
#include <Ty/ErrorOr.h>
#include <Ty/LinearMap.h>
#include <Ty/SmallCapture.h>

namespace Web {

struct FileRouter {
    using Renderer = SmallCapture<ErrorOr<HTTP::Response>>;

    using StaticRoutes = LinearMap<StringView, StringView>;
    using WatchFileMap = LinearMap<int, StringView>;
    using Files = LinearMap<StringView, File>;

    static ErrorOr<FileRouter> create();

    constexpr FileRouter(FileRouter&& other)
        : m_static_routes(move(other.m_static_routes))
        , m_files(move(other.m_files))
        , m_watch_file_map(move(other.m_watch_file_map))
        , m_filewatch_fd(other.m_filewatch_fd)
    {
        other.invalidate();
    }

    constexpr ~FileRouter()
    {
        if (is_valid()) {
            destroy();
            invalidate();
        }
    }


    // ErrorOr<void> add_route(StringView slug, Renderer renderer);
    ErrorOr<void> add_route(StringView route, StringView filename);

    ErrorOr<void> reload_files_if_needed(Core::File& log);
    Optional<Id<File>> find(StringView route) const;

    File const& operator[](Id<File> id) { return m_files[id]; }

private:
    constexpr FileRouter(StaticRoutes&& static_routes, Files&& files, WatchFileMap&& watch_file_map, int filewatch_fd)
        : m_static_routes(move(static_routes))
        , m_files(move(files))
        , m_watch_file_map(move(watch_file_map))
        , m_filewatch_fd(filewatch_fd)
    {
    }

    void destroy() const;
    bool is_valid() const { return m_filewatch_fd != -1; }
    void invalidate() { m_filewatch_fd = -1; }

    ErrorOr<void> reload_file(StringView path);

    StaticRoutes m_static_routes;
    Files m_files;
    WatchFileMap m_watch_file_map;
    int m_filewatch_fd;
};

}
