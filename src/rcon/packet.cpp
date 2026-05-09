#include "rcon/packet.h"

#include <cstring>

namespace harryitz::rcon_server {

static void writeLE32(std::vector<uint8_t> &buf, int32_t v)
{
    buf.push_back(static_cast<uint8_t>(v));
    buf.push_back(static_cast<uint8_t>(v >> 8));
    buf.push_back(static_cast<uint8_t>(v >> 16));
    buf.push_back(static_cast<uint8_t>(v >> 24));
}

static int32_t readLE32(const uint8_t *p)
{
    return static_cast<int32_t>(p[0])
         | static_cast<int32_t>(p[1]) << 8
         | static_cast<int32_t>(p[2]) << 16
         | static_cast<int32_t>(p[3]) << 24;
}

std::vector<uint8_t> encode(const Packet &pkt)
{
    int32_t len = 4 + 4 + static_cast<int32_t>(pkt.payload.size()) + 2;

    std::vector<uint8_t> buf;
    buf.reserve(4 + len);
    writeLE32(buf, len);
    writeLE32(buf, pkt.id);
    writeLE32(buf, static_cast<int32_t>(pkt.type));
    buf.insert(buf.end(), pkt.payload.begin(), pkt.payload.end());
    buf.push_back(0);
    buf.push_back(0);
    return buf;
}

bool recvAll(socket_t fd, void *buf, std::size_t n)
{
    auto *ptr = static_cast<char *>(buf);
    while (n > 0) {
        int got = ::recv(fd, ptr, static_cast<int>(n), MSG_WAITALL);
        if (got <= 0) return false;
        ptr += got;
        n   -= static_cast<std::size_t>(got);
    }
    return true;
}

bool read(socket_t fd, Packet &out)
{
    uint8_t lenBuf[4];
    if (!recvAll(fd, lenBuf, 4)) return false;

    int32_t len = readLE32(lenBuf);
    if (len < 10 || len > 4110) return false;

    std::vector<uint8_t> body(len);
    if (!recvAll(fd, body.data(), len)) return false;

    out.id      = readLE32(body.data());
    out.type    = static_cast<PacketType>(readLE32(body.data() + 4));
    std::size_t payloadLen = len - 4 - 4 - 2;
    out.payload.assign(reinterpret_cast<char *>(body.data() + 8), payloadLen);
    return true;
}

}  // namespace harryitz::rcon_server
