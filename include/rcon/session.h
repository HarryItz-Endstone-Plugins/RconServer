#pragma once

#include <atomic>
#include <future>
#include <string>
#include <thread>

#include <endstone/plugin/plugin.h>

#include "rcon/platform.h"

namespace harryitz::rcon_server {

class RconSession {
public:
    RconSession(socket_t fd, std::string remote, const std::string &password, endstone::Plugin &plugin);
    ~RconSession();

    RconSession(const RconSession &)            = delete;
    RconSession &operator=(const RconSession &) = delete;

    void start();
    void stop();

    [[nodiscard]] bool isDone() const { return done_.load(); }

private:
    void run();
    bool sendPacket(int32_t id, int32_t type, const std::string &payload);
    std::string execCommand(const std::string &cmd);

    socket_t fd_;
    std::string remote_;
    const std::string &password_;
    endstone::Plugin &plugin_;
    std::thread thread_;
    std::atomic<bool> done_{false};
};

}
