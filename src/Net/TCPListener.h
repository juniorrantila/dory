#pragma once
#include "TCPConnection.h"
#include <Ty/Base.h>
#include <Ty/ErrorOr.h>

namespace Net {

enum class IPVersion {
    V4,
    V6
};

struct TCPListener {
    static ErrorOr<TCPListener> create(u16 port,
        u16 queued_connections = 8, IPVersion = IPVersion::V4);
    constexpr TCPListener(TCPListener&& other)
        : m_socket(other.m_socket)
        , m_port(other.m_port)
    {
        other.invalidate();
    }
    ~TCPListener();

    u16 port() const { return m_port; }

    ErrorOr<void> destroy()
    {
        if (is_valid()) {
            TRY(close());
            invalidate();
        }
        return {};
    }

    ErrorOr<TCPConnection> accept() const;

private:
    constexpr TCPListener(int socket, u16 port)
        : m_socket(socket)
        , m_port(port)
    {
    }

    ErrorOr<void> close() const;
    bool is_valid() const { return m_socket != -1; }
    void invalidate() { m_socket = -1; }

    int m_socket;
    u16 m_port;
};

}
