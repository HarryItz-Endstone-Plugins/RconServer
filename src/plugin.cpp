#include "plugin.h"
#include "rcon/platform.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

using namespace harryitz::rcon_server;

ENDSTONE_PLUGIN("rcon_server", "1.0.0", harryitz::rcon_server::RconPlugin)
{
    description = "RconServer — remote console access over TCP for Endstone BDS.";
    authors     = {"harryitz"};
    prefix      = "rcon_server";

    command("rcon")
        .description("RconServer management.")
        .usages("/rcon <status|reload>")
        .permissions("rconserver.admin");

    permission("rconserver.admin")
        .description("Access to /rcon management commands.")
        .default_(endstone::PermissionDefault::Operator);
}

static std::string trim(const std::string &s)
{
    auto b = s.find_first_not_of(" \t\r\n");
    auto e = s.find_last_not_of(" \t\r\n");
    return (b == std::string::npos) ? "" : s.substr(b, e - b + 1);
}

void RconPlugin::saveDefaultConfig()
{
    fs::path path = fs::path(getDataFolder()) / "rcon.properties";
    if (fs::exists(path)) return;

    fs::create_directories(getDataFolder());
    std::ofstream f(path);
    f << "# RconServer configuration\n"
      << "# port: TCP port to listen on (default: 25575)\n"
      << "port=25575\n\n"
      << "# password: authentication password — CHANGE THIS!\n"
      << "password=changeme\n\n"
      << "# max_connections: maximum simultaneous RCON clients\n"
      << "max_connections=10\n\n"
      << "# bind_all: set to true to listen on 0.0.0.0 instead of 127.0.0.1\n"
      << "# Only enable if your firewall restricts access to trusted IPs!\n"
      << "bind_all=false\n";
}

RconPlugin::Config RconPlugin::loadConfig()
{
    saveDefaultConfig();

    Config cfg;
    fs::path path = fs::path(getDataFolder()) / "rcon.properties";
    std::ifstream f(path);
    if (!f) return cfg;

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));

        if (key == "port") {
            cfg.port = static_cast<uint16_t>(std::stoi(val));
        }
        else if (key == "password") {
            cfg.password = val;
        }
        else if (key == "max_connections") {
            cfg.maxConnections = std::stoi(val);
        }
        else if (key == "bind_all") {
            cfg.bindAll = (val == "true" || val == "1");
        }
    }
    return cfg;
}

void RconPlugin::onEnable()
{
    auto cfg = loadConfig();

    if (cfg.password.empty() || cfg.password == "changeme") {
        getLogger().warning("RconServer: password is not set. Edit plugins/RconServer/rcon.properties and set a strong password.");
        return;
    }

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        getLogger().error("RconServer: WSAStartup failed.");
        return;
    }
#endif

    server_ = std::make_unique<RconServer>(*this, cfg.password, cfg.port, cfg.maxConnections, cfg.bindAll);
    if (!server_->start()) {
        getLogger().error("RconServer: failed to start. Check port {} is available.", cfg.port);
        server_.reset();
    }
}

void RconPlugin::onDisable()
{
    server_.reset();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool RconPlugin::onCommand(endstone::CommandSender &sender, const endstone::Command &,
                           const std::vector<std::string> &args)
{
    std::string sub = args.empty() ? "status" : args[0];

    if (sub == "status") {
        if (server_) {
            sender.sendMessage("§aRCON is running.");
        }
        else {
            sender.sendMessage("§cRCON is stopped. Check password config or console for errors.");
        }
        return true;
    }

    if (sub == "reload") {
        server_.reset();

        auto cfg = loadConfig();
        if (cfg.password.empty() || cfg.password == "changeme") {
            sender.sendErrorMessage("§cRCON password is not configured. Edit rcon.properties first.");
            return true;
        }

        server_ = std::make_unique<RconServer>(*this, cfg.password, cfg.port, cfg.maxConnections, cfg.bindAll);
        if (server_->start()) {
            sender.sendMessage("§aRCON reloaded on port {}.", cfg.port);
        }
        else {
            sender.sendErrorMessage("§cFailed to start RCON. See console.");
            server_.reset();
        }
        return true;
    }

    sender.sendMessage("§7  /rcon status");
    sender.sendMessage("§7  /rcon reload");
    return true;
}
