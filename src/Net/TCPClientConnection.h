#pragma once
#include <Ty/StringBuffer.h>
#include <Ty/StringView.h>

namespace Net {

struct TCPClientConnection {
    int socket;
    mutable StringBuffer write_buffer;

    constexpr TCPClientConnection(TCPClientConnection&& other)
        : socket(other.socket)
    {
        other.invalidate();
    }

    ~TCPClientConnection()
    {
        if (is_valid()) {
            destroy();
            invalidate();
        }
    }
    void destroy() const;

    static ErrorOr<TCPClientConnection> create(int socket);
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

    bool is_valid() const { return socket != -1; }
    void invalidate() { socket = -1; }

private:
    constexpr TCPClientConnection(int socket)
        : socket(socket)
    {
    }
};

}
