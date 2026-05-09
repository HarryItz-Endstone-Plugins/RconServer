#pragma once

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  include <string>
#  ifndef MSG_NOSIGNAL
#    define MSG_NOSIGNAL 0
#  endif
namespace harryitz::rcon_server {
using socket_t = SOCKET;
inline constexpr socket_t invalid_socket = INVALID_SOCKET;
inline void close_socket(socket_t s) { ::closesocket(s); }
inline void shutdown_socket(socket_t s) { ::shutdown(s, SD_BOTH); }
inline std::string socket_error()
{
    char buf[256] = {};
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   nullptr, WSAGetLastError(), 0, buf, sizeof(buf), nullptr);
    return buf;
}
}
#else
#  include <arpa/inet.h>
#  include <cerrno>
#  include <cstring>
#  include <netinet/in.h>
#  include <sys/socket.h>
#  include <unistd.h>
namespace harryitz::rcon_server {
using socket_t = int;
inline constexpr socket_t invalid_socket = -1;
inline void close_socket(socket_t s) { ::close(s); }
inline void shutdown_socket(socket_t s) { ::shutdown(s, SHUT_RDWR); }
inline std::string socket_error() { return std::strerror(errno); }
}
#endif
