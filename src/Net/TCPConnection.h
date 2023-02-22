#pragma once
#include <Ty/StringBuffer.h>
#include <Ty/StringView.h>
#include <sys/socket.h>

namespace Net {

struct TCPConnection {
    struct sockaddr_storage address;
    socklen_t address_size;

    int socket;
    mutable StringBuffer write_buffer;

    constexpr TCPConnection(TCPConnection&& other)
        : address(other.address)
        , address_size(other.address_size)
        , socket(other.socket)
        , write_buffer(move(other.write_buffer))
    {
        other.invalidate();
    }

    ~TCPConnection()
    {
        if (is_valid()) {
            destroy();
            invalidate();
        }
    }

    static ErrorOr<TCPConnection> connect(StringView host, u16 port);
    static ErrorOr<TCPConnection> create(int socket, struct sockaddr_storage, socklen_t);
    ErrorOr<StringBuffer> read() const;
    ErrorOr<void> flush_write() const;
    ErrorOr<u32> write(StringView message);

    template <typename... Args>
    constexpr ErrorOr<u32> writeln(Args... args)
    {
        return TRY(write(args..., "\n"sv));
    }

    template <typename T>
    constexpr ErrorOr<u32> write(
        T value) requires is_trivially_copyable<T>
    {
        return TRY(Formatter<T>::write(*this, value));
    }

    template <typename T>
    constexpr ErrorOr<u32> write(T const& value) requires(
        !is_trivially_copyable<T>)
    {
        return TRY(Formatter<T>::write(*this, value));
    }

    template <typename... Args>
    constexpr ErrorOr<u32> write(Args... args) requires(
        sizeof...(Args) > 1)
    {
        constexpr auto args_size = sizeof...(Args);
        ErrorOr<u32> results[args_size] = {
            write(args)...,
        };
        u32 written = 0;
        for (u32 i = 0; i < args_size; i++)
            written += TRY(results[i]);
        return written;
    }

    ErrorOr<StringBuffer> printable_address() const;

private:
    constexpr TCPConnection(
        sockaddr_storage address,
        socklen_t address_size,
        int socket,
        StringBuffer&& write_buffer
    )
        : address(address)
        , address_size(address_size)
        , socket(socket)
        , write_buffer(move(write_buffer))
    {
    }

    void destroy() const;

    bool is_valid() const { return socket != -1; }
    void invalidate() { socket = -1; }

    constexpr TCPConnection(int socket)
        : socket(socket)
    {
    }
};

}
