#include "TCPListener.h"
#include "System.h"
#include "TCPClientConnection.h"
#include "Print.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace Core {

ErrorOr<TCPListener> TCPListener::create(u16 port,
    u16 queued_connections)
{
    int socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) {
        return Error::from_errno();
    }

    struct sockaddr_in address {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY,
    };

    auto rv = bind(socket, (struct sockaddr*)&address, sizeof(address));
    if (rv < 0) {
        return Error::from_errno();
    }

    if (listen(socket, queued_connections) < 0) {
        return Error::from_errno();
    }

    return TCPListener(socket, port);
}

void TCPListener::destroy() const
{
    if (shutdown(m_socket, SHUT_RDWR) < 0) {
        dbgln("Error: "sv, Error::from_errno());
    }
    if (close(m_socket) < 0) {
        dbgln("Error: "sv, Error::from_errno());
    }
}

ErrorOr<TCPClientConnection> TCPListener::accept() const
{
    int client_socket = ::accept(m_socket, nullptr, nullptr);
    if (client_socket < 0) {
        return Error::from_errno();
    }
    return TRY(TCPClientConnection::create(client_socket));
}

}
