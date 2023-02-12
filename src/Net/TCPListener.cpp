#include "TCPClientConnection.h"
#include "TCPListener.h"
#include <Core/Print.h>
#include <Core/System.h>
#include <asm-generic/socket.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>

namespace Net {

ErrorOr<TCPListener> TCPListener::create(u16 port,
    u16 queued_connections, IPVersion ip_version)
{
    auto port_buffer = TRY(StringBuffer::create_fill(port, "\0"sv));

    struct addrinfo hints = {
        .ai_flags = AI_PASSIVE,
        .ai_family = ip_version == IPVersion::V4 ? AF_INET : AF_INET6,
        .ai_socktype = SOCK_STREAM,
    };

    struct addrinfo* res = nullptr;
    if (getaddrinfo(nullptr, port_buffer.data(), &hints, &res) < 0)
        return Error::from_errno();

    int socket = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (socket < 0)
        return Error::from_errno();
    int yes = 1;
    if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
        return Error::from_errno();
    if (bind(socket, res->ai_addr, res->ai_addrlen) < 0)
        return Error::from_errno();
    if (listen(socket, queued_connections) < 0) {
        return Error::from_errno();
    }

    return TCPListener(socket, port);
}

void TCPListener::destroy() const
{
    if (close(m_socket) < 0) {
        dbgln("Error: "sv, Error::from_errno());
    }
}

ErrorOr<TCPClientConnection> TCPListener::accept() const
{
    sockaddr_storage address;
    socklen_t address_size;
    int client_socket = ::accept(m_socket, (struct sockaddr*)&address, &address_size);
    if (client_socket < 0) {
        return Error::from_errno();
    }
    return TRY(TCPClientConnection::create(client_socket, address, address_size));
}

}
