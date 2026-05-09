#pragma once

#include <memory>
#include <string>
#include <vector>

#include <endstone/endstone.hpp>

#include "rcon/server.h"

namespace harryitz::rcon_server {

class RconPlugin : public endstone::Plugin {
public:
    void onEnable() override;
    void onDisable() override;
    bool onCommand(endstone::CommandSender &sender, const endstone::Command &cmd,
                   const std::vector<std::string> &args) override;

private:
    struct Config {
        uint16_t    port           = 25575;
        std::string password       = "";
        int         maxConnections = 10;
        bool        bindAll        = false;
    };

    Config loadConfig();
    void   saveDefaultConfig();

    std::unique_ptr<RconServer> server_;
};

}
