#include "TCPClientConnection.h"
#include "Ty/StringBuffer.h"
#include <Core/Print.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

namespace Net {

ErrorOr<TCPClientConnection> TCPClientConnection::create(
    int socket,
    struct sockaddr_storage address,
    socklen_t address_size
) {
    return TCPClientConnection {
        address,
        address_size,
        socket,
        TRY(StringBuffer::create()),
    };
}

void TCPClientConnection::destroy() const
{
    flush_write().ignore();
    ::close(socket);
}

ErrorOr<StringBuffer> TCPClientConnection::read() const
{
    auto result = TRY(StringBuffer::create());

    isize bytes_read = -1;
    while (true) {
        char buffer[1024];
        isize buffer_size = sizeof(buffer);
        bytes_read = ::recv(socket, buffer, buffer_size - 1, 0);
        if (bytes_read < 0)
            return Error::from_errno();
        buffer[bytes_read] = '\0';
        TRY(result.write(StringView::from_c_string(buffer)));
        if (bytes_read == 0)
            break; // NOTE: Client closed connection.
        if (bytes_read < buffer_size)
            break;
    }

    return result;
}

ErrorOr<u32> TCPClientConnection::write(StringView message)
{
    if (write_buffer.size_left() < message.size)
        TRY(flush_write());
    return TRY(write_buffer.write(message));
}

ErrorOr<void> TCPClientConnection::flush_write() const
{
    u32 total_bytes_written = 0;

    while (total_bytes_written < write_buffer.size()) {
        auto view = write_buffer.view().shrink_from_start(total_bytes_written);
        auto bytes_written = send(socket, view.data, view.size, MSG_NOSIGNAL);
        if (bytes_written < 0)
            return Error::from_errno();
        total_bytes_written += bytes_written;
    }
    write_buffer.clear();

    return {};
}

static void* get_in_addr(struct sockaddr* sa)
{
    if (sa->sa_family == AF_INET)
        return &((struct sockaddr_in*)sa)->sin_addr;
    return &((struct sockaddr_in6*)sa)->sin6_addr;
}

ErrorOr<StringBuffer> TCPClientConnection::printable_address() const
{
    char buf[INET6_ADDRSTRLEN + 1];
    c_string res = inet_ntop(
            address.ss_family,
            get_in_addr((struct sockaddr*)&address),
            buf,
            sizeof(buf)
        );
    if (res == nullptr)
        return Error::from_errno();
    return StringBuffer::create_fill(StringView::from_c_string(buf));
}

}
