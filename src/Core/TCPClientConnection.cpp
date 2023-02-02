#include "TCPClientConnection.h"
#include "Print.h"
#include <unistd.h>

namespace Core {

ErrorOr<TCPClientConnection> TCPClientConnection::create(int socket)
{
    return TCPClientConnection {
        socket,
        TRY(StringBuffer::create_saturated(16 * 1024 * 1024)),
    };
}

void TCPClientConnection::destroy() const
{
    (void)const_cast<TCPClientConnection*>(this)->flush_write();
    ::close(socket);
}

ErrorOr<StringView> TCPClientConnection::read()
{
    read_buffer.clear();

    char buffer[4096];
    isize buffer_size = sizeof(buffer);

    isize bytes_read = -1;
    while (true) {
        bytes_read = ::read(socket, buffer, buffer_size);
        buffer[bytes_read] = '\0';
        if (bytes_read < 0)
            return Error::from_errno();
        TRY(read_buffer.write(StringView::from_c_string(buffer)));
        if (bytes_read == 0 || bytes_read < buffer_size)
            break;
    }

    return read_buffer.view();
}

ErrorOr<u32> TCPClientConnection::write(StringView message)
{
    if (write_buffer.size_left() < message.size)
        TRY(flush_write());
    return TRY(write_buffer.write(message));
}

ErrorOr<void> TCPClientConnection::flush_write()
{
    auto bytes_written = ::write(socket, write_buffer.data(), write_buffer.size());
    if (bytes_written < write_buffer.size()) {
        return Error::from_errno();
    }
    write_buffer.clear();

    return {};
}

}
