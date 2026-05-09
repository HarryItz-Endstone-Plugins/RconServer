#include "rcon/sender.h"

#include <string>
#include <variant>

namespace rcon {

static std::string stripColors(std::string s)
{
    std::string out;
    out.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\xc2' && i + 1 < s.size() && s[i + 1] == '\xa7') {
            // UTF-8 encoding of §: skip § + next char
            i += 2;
            continue;
        }
        if (s[i] == '\xc2' || s[i] == '\xa7') {
            // legacy §
            ++i;
            continue;
        }
        out += s[i];
    }
    return out;
}

static std::string toString(const endstone::Message &msg)
{
    return std::visit([](const auto &v) -> std::string {
        if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::string>) {
            return v;
        }
        else {
            return v.getText();  // Translatable::getText()
        }
    }, msg);
}

void RconSender::sendMessage(const endstone::Message &msg) const
{
    if (!buf_.empty()) buf_ += '\n';
    buf_ += stripColors(toString(msg));
}

void RconSender::sendErrorMessage(const endstone::Message &msg) const
{
    if (!buf_.empty()) buf_ += '\n';
    buf_ += stripColors(toString(msg));
}

std::string RconSender::flush() const
{
    std::string out = std::move(buf_);
    buf_.clear();
    return out;
}

}  // namespace rcon
