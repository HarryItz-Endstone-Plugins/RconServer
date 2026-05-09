#include "rcon/session.h"
#include "rcon/packet.h"

#include <endstone/command/block_command_sender.h>
#include <endstone/command/command_sender_wrapper.h>
#include <endstone/command/console_command_sender.h>
#include <endstone/scheduler/scheduler.h>
#include <endstone/server.h>

namespace harryitz::rcon_server {

RconSession::RconSession(socket_t fd, std::string remote, const std::string &password, endstone::Plugin &plugin)
    : fd_(fd), remote_(std::move(remote)), password_(password), plugin_(plugin)
{
}

RconSession::~RconSession()
{
    stop();
}

void RconSession::start()
{
    thread_ = std::thread([this] { run(); });
}

void RconSession::stop()
{
    if (fd_ != invalid_socket) {
        shutdown_socket(fd_);
    }
    if (thread_.joinable()) {
        thread_.join();
    }
}

bool RconSession::sendPacket(int32_t id, int32_t type, const std::string &payload)
{
    Packet pkt;
    pkt.id      = id;
    pkt.type    = static_cast<PacketType>(type);
    pkt.payload = payload;

    auto buf = encode(pkt);
    int sent = ::send(fd_, reinterpret_cast<const char *>(buf.data()), static_cast<int>(buf.size()), MSG_NOSIGNAL);
    return sent == static_cast<int>(buf.size());
}

std::string RconSession::execCommand(const std::string &cmd)
{
    std::promise<std::string> promise;
    auto future = promise.get_future();

    plugin_.getServer().getScheduler().runTask(plugin_, [&]() {
        std::string buf;

        static const auto stripColors = [](const std::string &s) {
            std::string out;
            out.reserve(s.size());
            for (std::size_t i = 0; i < s.size(); ++i) {
                if (s[i] == '\xc2' && i + 1 < s.size() && s[i + 1] == '\xa7') { i += 2; continue; }
                if (s[i] == '\xa7') { ++i; continue; }
                out += s[i];
            }
            return out;
        };

        static const auto toStr = [](const endstone::Message &msg) -> std::string {
            return std::visit([](const auto &v) -> std::string {
                if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::string>) return v;
                else return v.getText();
            }, msg);
        };

        auto &console = plugin_.getServer().getCommandSender();
        endstone::CommandSenderWrapper::Callback onMsg = [&](const endstone::Message &msg) {
            if (!buf.empty()) buf += '\n';
            buf += stripColors(toStr(msg));
        };
        endstone::CommandSenderWrapper::Callback onErr = [&](const endstone::Message &msg) {
            if (!buf.empty()) buf += '\n';
            buf += stripColors(toStr(msg));
        };
        endstone::CommandSenderWrapper wrapper(console, std::move(onMsg), std::move(onErr));

        [[maybe_unused]] bool ok = plugin_.getServer().dispatchCommand(wrapper, cmd);
        promise.set_value(std::move(buf));
    });

    return future.get();
}

void RconSession::run()
{
    plugin_.getLogger().info("RCON connection from {}.", remote_);

    bool authed = false;

    Packet pkt;
    while (read(fd_, pkt)) {
        if (pkt.type == PacketType::Auth) {
            if (pkt.payload == password_) {
                authed = true;
                sendPacket(pkt.id, static_cast<int32_t>(PacketType::AuthResp), "");
                plugin_.getLogger().info("RCON {} authenticated.", remote_);
            }
            else {
                sendPacket(-1, static_cast<int32_t>(PacketType::AuthResp), "");
                plugin_.getLogger().warning("RCON {} failed authentication.", remote_);
            }
            continue;
        }

        if (pkt.type == PacketType::ExecCmd) {
            if (!authed) {
                sendPacket(-1, static_cast<int32_t>(PacketType::AuthResp), "");
                continue;
            }

            std::string output = execCommand(pkt.payload);
            if (output.size() > 4090) {
                output.resize(4090);
                output += "...";
            }
            sendPacket(pkt.id, static_cast<int32_t>(PacketType::Response), output);
        }
    }

    plugin_.getLogger().info("RCON {} disconnected.", remote_);
    close_socket(fd_);
    fd_ = invalid_socket;
    done_.store(true);
}

}  // namespace harryitz::rcon_server
