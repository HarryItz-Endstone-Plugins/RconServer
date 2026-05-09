#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "rcon/platform.h"

namespace harryitz::rcon_server {

enum class PacketType : int32_t {
    Response  = 0,
    ExecCmd   = 2,
    AuthResp  = 2,
    Auth      = 3,
};

struct Packet {
    int32_t    id   = 0;
    PacketType type = PacketType::Response;
    std::string payload;
};

std::vector<uint8_t> encode(const Packet &pkt);
bool recvAll(socket_t fd, void *buf, std::size_t n);
bool read(socket_t fd, Packet &out);

}
