#include "TCPListener.h"
#include <Core/Print.h>
#include <Ty/System.h>
#include <asm-generic/socket.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace Net {

ErrorOr<TCPListener> TCPListener::create(u16 port,
    u16 queued_connections, IPVersion ip_version)
{
    auto port_buffer = TRY(StringBuffer::create_fill(port, "\0"sv));

    auto* res = TRY(System::getaddrinfo(port,
        {
            .ai_flags = AI_PASSIVE,
            .ai_family
            = ip_version == IPVersion::V4 ? AF_INET : AF_INET6,
            .ai_socktype = SOCK_STREAM,
        }));

    int socket = TRY(System::socket(res->ai_family,
        res->ai_socktype, res->ai_protocol));
    TRY(System::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, true));
    TRY(System::bind(socket, res->ai_addr, res->ai_addrlen));
    TRY(System::listen(socket, queued_connections));

    return TCPListener(socket, port);
}

TCPListener::~TCPListener()
{
    destroy().or_else([&](auto error) {
        Core::File::stderr().writeln("Error: "sv, error).ignore();
    });
}

ErrorOr<void> TCPListener::close() const
{
    TRY(System::close(m_socket));
    return {};
}

ErrorOr<TCPConnection> TCPListener::accept() const
{
    sockaddr_storage address;
    socklen_t address_size;
    int client_socket = ::accept(m_socket,
        (struct sockaddr*)&address, &address_size);
    if (client_socket < 0) {
        return Error::from_errno();
    }
    return TRY(TCPConnection::create(client_socket, address,
        address_size));
}

}
