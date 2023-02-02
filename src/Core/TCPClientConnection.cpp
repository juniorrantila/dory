#include "TCPClientConnection.h"
#include "Print.h"
#include <unistd.h>

namespace Core {

ErrorOr<TCPClientConnection> TCPClientConnection::create(int socket)
{
    return TCPClientConnection(socket);
}

void TCPClientConnection::destroy() const
{
    (void)const_cast<TCPClientConnection*>(this)->flush_write();
    ::close(socket);
}

ErrorOr<StringBuffer> TCPClientConnection::read()
{
    auto result = TRY(StringBuffer::create());

    isize bytes_read = -1;
    while (true) {
        static char buffer[16 * 1024 * 1024];
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

ErrorOr<void> TCPClientConnection::flush_write()
{
    auto bytes_written = ::write(socket, write_buffer.data(), write_buffer.size());
    if (bytes_written < 0) {
        return Error::from_errno();
    }
    write_buffer.clear();

    return {};
}

}
