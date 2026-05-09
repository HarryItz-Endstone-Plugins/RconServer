# RconServer

A remote console (RCON) plugin for [Endstone](https://endstone.dev) Bedrock Dedicated Server. Lets you send commands to your server over a TCP connection using the standard Minecraft RCON protocol — compatible with tools like `mcrcon`, `RCON-CLI`, and most server management panels.

---

## Why RconServer?

Endstone has no built-in RCON support. Without this plugin, administering a server remotely requires direct SSH access or a custom solution. RconServer fills that gap using the well-established Minecraft RCON protocol, so you can use existing tooling without writing integration code.

---

## Features

- Minecraft-protocol-compatible RCON server (packet types: Auth, ExecCmd, Response)
- Password authentication — server refuses to start if password is unset or default
- Per-client session running on its own thread; commands are dispatched to the main server thread
- Configurable port, connection limit, and bind address
- `/rcon status` and `/rcon reload` in-game management commands

---

## Requirements

- Endstone ≥ 0.11
- Linux or Windows (x64)

---

## Installation

1. Copy `endstone_rcon-server.so` to your server's `plugins/` folder.
2. Start the server once to generate the default config at `plugins/RconServer/rcon.properties`.
3. **Set a strong password** in `rcon.properties` (the plugin will not start with the default `changeme` value).
4. Restart the server or run `/rcon reload`.

---

## Configuration

Config file: `plugins/RconServer/rcon.properties`

```properties
# TCP port to listen on
port=25575

# Authentication password — REQUIRED, must not be "changeme"
password=changeme

# Maximum simultaneous RCON connections
max_connections=10

# Set to true to bind on 0.0.0.0 instead of 127.0.0.1
# Only enable this if your firewall blocks untrusted IPs
bind_all=false
```

> **Security note:** Never expose the RCON port to the public internet without firewall rules. Use `bind_all=false` (default) to restrict access to localhost, then use SSH tunnelling or a VPN for remote access.

---

## Usage

### Connecting with mcrcon

```bash
mcrcon -H 127.0.0.1 -P 25575 -p "yourpassword" "say Hello from RCON"
```

### In-game management commands

| Command        | Permission         | Description                                 |
| -------------- | ------------------ | ------------------------------------------- |
| `/rcon status` | `rconserver.admin` | Show whether the RCON server is running     |
| `/rcon reload` | `rconserver.admin` | Reload config and restart the RCON listener |

The `rconserver.admin` permission is granted to operators by default.

---

## Permissions

| Permission         | Default | Description                           |
| ------------------ | ------- | ------------------------------------- |
| `rconserver.admin` | OP      | Access to `/rcon` management commands |

---

## License

MIT — see [LICENSE](LICENSE)
