#pragma once

#include <atomic>
#include <list>
#include <mutex>
#include <string>
#include <thread>

#include <endstone/plugin/plugin.h>

#include "rcon/platform.h"

namespace harryitz::rcon_server {

class RconServer {
public:
    RconServer(endstone::Plugin &plugin, std::string password, uint16_t port, int maxConnections, bool bindAll);
    ~RconServer();

    RconServer(const RconServer &)            = delete;
    RconServer &operator=(const RconServer &) = delete;

    [[nodiscard]] bool start();
    void stop();

private:
    void acceptLoop();
    void reapSessions();

    endstone::Plugin &plugin_;
    std::string password_;
    uint16_t port_;
    int maxConnections_;
    bool bindAll_;

    socket_t listenFd_{invalid_socket};
    std::thread acceptThread_;
    std::atomic<bool> running_{false};

    std::mutex sessionsMutex_;
    std::list<std::unique_ptr<class RconSession>> sessions_;
};

}
