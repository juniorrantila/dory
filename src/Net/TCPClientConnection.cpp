#include "TCPClientConnection.h"
#include <Core/Print.h>
#include <sys/socket.h>
#include <unistd.h>

namespace Net {

ErrorOr<TCPClientConnection> TCPClientConnection::create(int socket)
{
    return TCPClientConnection(socket);
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
        static char buffer[1024];
        isize buffer_size = sizeof(buffer);
        bytes_read = ::read(socket, buffer, buffer_size);
        buffer[bytes_read] = '\0';
        if (bytes_read < 0)
            return Error::from_errno();
        TRY(result.write(StringView::from_c_string(buffer)));
        if (bytes_read == 0 || bytes_read < buffer_size)
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
    auto bytes_written = send(socket, write_buffer.data(), write_buffer.size(), MSG_NOSIGNAL);
    if (bytes_written < 0) {
        return Error::from_errno();
    }
    write_buffer.clear();

    return {};
}

}
