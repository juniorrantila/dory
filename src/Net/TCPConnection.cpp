#include "TCPConnection.h"
#include <Core/Print.h>
#include <Ty/StringBuffer.h>
#include <Ty/System.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace Net {

ErrorOr<TCPConnection> TCPConnection::create(int socket,
    struct sockaddr_storage address, socklen_t address_size)
{
    return TCPConnection {
        address,
        address_size,
        socket,
        TRY(StringBuffer::create()),
    };
}

void TCPConnection::destroy() const
{
    flush_write().ignore();
    ::close(socket);
}

ErrorOr<TCPConnection> TCPConnection::connect(StringView host,
    u16 port)
{
    auto* res = TRY(System::getaddrinfo(host, port,
        {
            .ai_flags = AI_PASSIVE,
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM,
        }));

    auto socket = TRY(System::socket(res->ai_family,
        res->ai_socktype, res->ai_protocol));
    TRY(System::connect(socket, res->ai_addr, res->ai_addrlen));

    struct sockaddr_storage addr { };
    __builtin_memcpy(&addr, res->ai_addr, res->ai_addrlen);
    return TRY(create(socket, addr, res->ai_addrlen));
}

ErrorOr<StringBuffer> TCPConnection::read() const
{
    auto result = TRY(StringBuffer::create());

    isize bytes_read = -1;
    while (true) {
        char buffer[1024];
        isize buffer_size = sizeof(buffer);
        bytes_read = TRY(System::recv(socket, buffer, buffer_size));
        buffer[bytes_read] = '\0';
        TRY(result.write(StringView::from_c_string(buffer)));
        if (bytes_read == 0)
            break; // NOTE: Client closed connection.
        if (bytes_read < buffer_size)
            break;
    }

    return result;
}

ErrorOr<u32> TCPConnection::write(StringView message)
{
    if (write_buffer.size_left() < message.size)
        TRY(flush_write());
    return TRY(write_buffer.write(message));
}

ErrorOr<void> TCPConnection::flush_write() const
{
    u32 bytes_written = 0;

    while (bytes_written < write_buffer.size()) {
        auto view
            = write_buffer.view().shrink_from_start(bytes_written);
        bytes_written
            += TRY(System::send(socket, view, MSG_NOSIGNAL));
    }
    write_buffer.clear();

    return {};
}

ErrorOr<StringBuffer> TCPConnection::printable_address() const
{
    return TRY(System::inet_ntop(address));
}

}
