#pragma once

#include <string>
#include <unordered_set>

#include <endstone/actor/actor.h>
#include <endstone/command/block_command_sender.h>
#include <endstone/command/command_sender.h>
#include <endstone/command/console_command_sender.h>
#include <endstone/permissions/permission_level.h>
#include <endstone/player.h>
#include <endstone/server.h>

namespace rcon {

// A CommandSender that captures all output into a string buffer.
// One instance is created per command execution.
class RconSender final : public endstone::CommandSender {
public:
    explicit RconSender(endstone::Server &server) : server_(server) {}

    // CommandSender
    void sendMessage(const endstone::Message &msg) const override;
    void sendErrorMessage(const endstone::Message &msg) const override;
    [[nodiscard]] endstone::Server &getServer() const override { return server_; }
    [[nodiscard]] std::string getName() const override { return "RCON"; }
    [[nodiscard]] endstone::ConsoleCommandSender *asConsole() const override { return nullptr; }
    [[nodiscard]] endstone::BlockCommandSender *asBlock() const override { return nullptr; }
    [[nodiscard]] endstone::Actor *asActor() const override { return nullptr; }
    [[nodiscard]] endstone::Player *asPlayer() const override { return nullptr; }

    // Permissible — RCON runs as console-level
    [[nodiscard]] endstone::PermissionLevel getPermissionLevel() const override
    {
        return endstone::PermissionLevel::Console;
    }
    [[nodiscard]] bool isPermissionSet(std::string) const override { return false; }
    [[nodiscard]] bool isPermissionSet(const endstone::Permission &) const override { return false; }
    [[nodiscard]] bool hasPermission(std::string) const override { return true; }
    [[nodiscard]] bool hasPermission(const endstone::Permission &) const override { return true; }

    endstone::PermissionAttachment *addAttachment(endstone::Plugin &, const std::string &, bool) override
    {
        return nullptr;
    }
    endstone::PermissionAttachment *addAttachment(endstone::Plugin &) override { return nullptr; }
    bool removeAttachment(endstone::PermissionAttachment &) override { return false; }
    void recalculatePermissions() override {}
    [[nodiscard]] std::unordered_set<endstone::PermissionAttachmentInfo *> getEffectivePermissions() const override
    {
        return {};
    }

    // Collect all captured output, stripping Minecraft color codes.
    [[nodiscard]] std::string flush() const;

private:
    endstone::Server &server_;
    mutable std::string buf_;
};

}  // namespace rcon
