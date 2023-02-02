#pragma once
#include "TCPClientConnection.h"
#include <Ty/Base.h>
#include <Ty/ErrorOr.h>

namespace Core {

struct TCPListener {
    static ErrorOr<TCPListener> create(u16 port,
        u16 queued_connections = 8);
    constexpr TCPListener(TCPListener&& other)
        : m_socket(other.m_socket)
        , m_port(other.m_port)
    {
        other.invalidate();
    }

    ~TCPListener()
    {
        if (is_valid()) {
            destroy();
            invalidate();
        }
    }

    u16 port() const { return m_port; }

    void destroy() const;
    bool is_valid() const { return m_socket != -1; }
    void invalidate() { m_socket = -1; }

    ErrorOr<TCPClientConnection> accept() const;

private:
    constexpr TCPListener(int socket, u16 port)
        : m_socket(socket)
        , m_port(port)
    {
    }

    int m_socket;
    u16 m_port;
};

}
