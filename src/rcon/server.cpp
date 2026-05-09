#include "rcon/server.h"
#include "rcon/session.h"

#include <algorithm>

namespace harryitz::rcon_server {

RconServer::RconServer(endstone::Plugin &plugin, std::string password, uint16_t port, int maxConnections, bool bindAll)
    : plugin_(plugin), password_(std::move(password)), port_(port), maxConnections_(maxConnections), bindAll_(bindAll)
{
}

RconServer::~RconServer()
{
    stop();
}

bool RconServer::start()
{
    listenFd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ == invalid_socket) {
        plugin_.getLogger().error("RconServer: socket() failed: {}", socket_error());
        return false;
    }

    int opt = 1;
    ::setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = bindAll_ ? htonl(INADDR_ANY) : htonl(INADDR_LOOPBACK);
    addr.sin_port        = htons(port_);

    if (::bind(listenFd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        plugin_.getLogger().error("RconServer: bind() on port {} failed: {}", port_, socket_error());
        close_socket(listenFd_);
        listenFd_ = invalid_socket;
        return false;
    }

    if (::listen(listenFd_, 4) < 0) {
        plugin_.getLogger().error("RconServer: listen() failed: {}", socket_error());
        close_socket(listenFd_);
        listenFd_ = invalid_socket;
        return false;
    }

    running_ = true;
    acceptThread_ = std::thread([this] { acceptLoop(); });
    plugin_.getLogger().info("RconServer listening on {}:{}.", bindAll_ ? "0.0.0.0" : "127.0.0.1", port_);
    return true;
}

void RconServer::stop()
{
    running_ = false;

    if (listenFd_ != invalid_socket) {
        shutdown_socket(listenFd_);
        close_socket(listenFd_);
        listenFd_ = invalid_socket;
    }

    if (acceptThread_.joinable()) {
        acceptThread_.join();
    }

    std::lock_guard lock(sessionsMutex_);
    sessions_.clear();
}

void RconServer::acceptLoop()
{
    while (running_) {
        sockaddr_in clientAddr{};
        socklen_t len = sizeof(clientAddr);
        socket_t clientFd = ::accept(listenFd_, reinterpret_cast<sockaddr *>(&clientAddr), &len);

        if (clientFd == invalid_socket) {
            if (running_) {
                plugin_.getLogger().warning("RconServer: accept() error: {}", socket_error());
            }
            break;
        }

        reapSessions();

        {
            std::lock_guard lock(sessionsMutex_);
            if (static_cast<int>(sessions_.size()) >= maxConnections_) {
                plugin_.getLogger().warning("RconServer: max connections reached, rejecting client.");
                close_socket(clientFd);
                continue;
            }

            char remoteIp[INET_ADDRSTRLEN] = {};
            ::inet_ntop(AF_INET, &clientAddr.sin_addr, remoteIp, sizeof(remoteIp));
            std::string remote = std::string(remoteIp) + ":" + std::to_string(ntohs(clientAddr.sin_port));

            auto session = std::make_unique<RconSession>(clientFd, std::move(remote), password_, plugin_);
            session->start();
            sessions_.push_back(std::move(session));
        }
    }
}

void RconServer::reapSessions()
{
    std::lock_guard lock(sessionsMutex_);
    sessions_.remove_if([](const auto &s) { return s->isDone(); });
}

}  // namespace harryitz::rcon_server
