# TAP TUI Client (Terminal User Interface)

A rich interactive terminal client in C++17 developed for **The Answer Protocol (TAP)**, a text-based multiplayer RPG/MUD universe. This client offers a full graphical terminal interface powered by **FTXUI**, featuring 24-bit TrueColor minimap rendering (Unicode half-blocks), multi-channel chat management, dynamic inventory, quest tracking, and an asynchronous non-blocking TCP client.

The project overview, the server, and the protocol specifications are described in the [root README](../README.md) and the [server README](../server/README.md).

---

## Table of Contents

1. [Overview](#1-overview)
   - [Goals and Features](#goals-and-features)
   - [Technologies Used](#technologies-used)
   - [Interface Layout (3-Column Layout)](#interface-layout-3-column-layout)
2. [Architecture & Project Structure](#2-architecture--project-structure)
   - [Directory Structure](#directory-structure)
   - [Network Connection (`network_client`)](#network-connection-network_client)
   - [Protocol & JSON Parsing (`protocol`)](#protocol--json-parsing-protocol)
   - [Hub & Map Management (`map_resolver`, `image_renderer`, `assets/maps`)](#hub--map-management-map_resolver-image_renderer-assetsmaps)
   - [Centralized State Management (`app_state`)](#centralized-state-management-app_state)
   - [User Interfaces (`login_view`, `game_view`, `ui_helpers`)](#user-interfaces-login_view-game_view-ui_helpers)
   - [Entry Point & Lifecycle (`main.cpp`)](#entry-point--lifecycle-maincpp)
3. [Getting Started (Practical Guide)](#3-getting-started-practical-guide)
   - [System Requirements](#system-requirements)
   - [Make Commands](#make-commands)
   - [Running & In-Game Usage](#running--in-game-usage)
   - [Supported TAP Protocol Commands](#supported-tap-protocol-commands)
4. [Connection & Integration with Other Projects](#4-connection--integration-with-other-projects)
   - [TAP Network Protocol Specification](#tap-network-protocol-specification)
   - [Connecting to the Official TAP Server](#connecting-to-the-official-tap-server)
   - [How to Reuse or Adapt this Module](#how-to-reuse-or-adapt-this-module)

---

## 1. Overview

### Goals and Features

The **TAP TUI** module is a complete client that connects over TCP to a game server adhering to *The Answer Protocol* RFC specification. It translates raw text commands and JSON server responses into a modern, ergonomic, and responsive visual experience directly within a terminal emulator.

Key features:
- **Interactive login**: Input modal for server IP, port, and nickname with connection error handling.
- **Dynamic room and map display (Hub & rooms)**: 24-bit ANSI TrueColor rendering of PNG images representing explored rooms (e.g., `hub.png`, `fejka.png`, `smycka.png`, etc.).
- **Auto-scrolling terminal log**: Syntax-highlighted, colored output for server messages, room descriptions, combat logs, and local notifications.
- **Smart input bar**: Interactive visual cursor, command history navigation (Up / Down arrow keys), game command and chat input.
- **Multi-channel chat**: Visual separation of global, local (`ROOM`), and party (`PARTY` / `GROUP`) discussions.
- **Inventory & Quest tracking**: Dedicated side panels automatically synchronized with server responses.

### Technologies Used

| Component | Technology / Library | Role |
|---|---|---|
| **Language** | C++17 (`-std=c++17`, `-pthread`) | High performance, thread safety, modern standard |
| **TUI Interface** | [FTXUI](https://github.com/ArthurSonzogni/FTXUI) (Arthur Sonzogni) | Declarative C++ framework for terminal UIs (DOM, components, events) |
| **JSON Parser** | [nlohmann/json](https://github.com/nlohmann/json) (`lib/json.hpp`) | Robust deserialization of server JSON payloads |
| **Image Decoder** | [stb_image.h](https://github.com/nothings/stb) (`lib/stb_image.h`) | In-memory loading and decoding of PNG/JPEG images |
| **Image Renderer** | Custom Half-Block Unicode (`▀`) + TrueColor ANSI | Renders two vertical pixels per terminal cell with 24-bit RGB colors |
| **Networking** | POSIX TCP Sockets (`sys/socket.h`, `arpa/inet.h`) | Asynchronous, non-blocking communication with the server |

### Interface Layout (3-Column Layout)

The main game screen (`runGameView`) is organized into three responsive columns that adapt dynamically to the terminal window dimensions:

```
+-------------------+-----------------------------------------+--------------------+
| LEFT COLUMN       | CENTER COLUMN                           | RIGHT COLUMN       |
+-------------------+-----------------------------------------+--------------------+
| [ Items ]         | [ Terminal - Nickname @ IP:Port ]       | [ Map ]            |
| - flour           | > Arthur: LOOK                          |   ▄▄▄▄▄▄▄▄▄▄▄▄     |
| - egg             | Room: Hub - The Restaurant and Farm     |   █ Minimap  █     |
|                   | Description: The central hub of...      |   ▀▀▀▀▀▀▀▀▀▀▀▀     |
| [ Quests ]        |                                         | [ Chat ]           |
| - fetch_beef      |                                         | Alice (GLOBAL): hi |
|   (1 required)    +-----------------------------------------+ Bob (ROOM): hello  |
|                   | > command_here... [cursor]              |                    |
+-------------------+-----------------------------------------+--------------------+
```

---

## 2. Architecture & Project Structure

### Directory Structure

```
tui/
├── Makefile                     # Build system, compilation, and linting
├── assets/
│   └── maps/                    # PNG room map images (hub.png, smycka.png, default.png, ...)
├── includes/                    # Public C++ headers (.hpp)
│   ├── app_state.hpp            # Centralized thread-safe state manager
│   ├── game_view.hpp            # Main FTXUI game view (3-column layout)
│   ├── image_renderer.hpp       # Terminal TrueColor graphics rendering engine
│   ├── login_view.hpp           # Interactive credential input modal
│   ├── map_resolver.hpp         # Image path resolution logic per room
│   ├── models.hpp               # Protocol data structures, DTOs, and enums
│   ├── network_client.hpp       # TCP socket client and asynchronous listener
│   ├── protocol.hpp             # JSON parsing and text formatting functions
│   └── ui_helpers.hpp           # Visual utilities (shrinkable, wrapLine, string helpers)
├── lib/                         # Third-party dependencies
│   ├── FTXUI/                   # Embedded FTXUI library sources
│   ├── json.hpp                 # nlohmann/json
│   └── stb_image.h              # stb_image
└── src/                         # Implementation source files (.cpp)
    ├── app_state.cpp
    ├── game_view.cpp
    ├── image_renderer.cpp
    ├── login_view.cpp
    ├── main.cpp                 # Entry point, lifecycle, and signal handling
    ├── map_resolver.cpp
    ├── network_client.cpp
    ├── protocol.cpp
    └── ui_helpers.cpp
```

---

### Network Connection (`network_client`)
- Files: `includes/network_client.hpp`, `src/network_client.cpp`

This module manages all low-level TCP network communications:
1. **Connection & Authentication** (`connectToServer`):
   - Creates the TCP socket connecting to the specified IP address and port.
   - Immediately sends the initial protocol handshake: `CONNECT <nickname>\n`.
   - Reads two lines: the server greeting (`OK hello proto=1`), then the answer to `CONNECT` (`OK connected`, or an error such as `ERR 201 NAME_IN_USE`, shown on the login screen).
2. **Asynchronous Listening** (`startListener` / `listenerLoop`):
   - Runs a dedicated background thread (`std::thread`) continuously receiving the TCP stream.
   - Reassembles received bytes into complete `\n`-terminated lines (framing).
   - Forwards each line to `processIncomingLine`.
   - Notifies the FTXUI rendering engine via `screen.PostEvent(ftxui::Event::Custom)` to trigger redraws without blocking.
3. **Server Response Dispatching**:
   - Movement responses (`handleMoveResponse`): the server answers `OK room=<id>`, so the client sends an automatic `LOOK` to refresh the room and the map.
   - Inventory responses (`handleInventoryResponse`): updates the inventory following `TAKE`, `DROP`, or `INVENTORY`.
   - Combat responses (`handleAttackResponse`, `EVT ROOM COMBAT`).
   - Chat responses (`handleChatAck`, `EVT ... CHAT`).
   - Clean shutdown (`OK bye`) or unexpected disconnection handling (`isConnectionLost`).

---

### Protocol & JSON Parsing (`protocol`)
- Files: `includes/protocol.hpp`, `src/protocol.cpp`, `includes/models.hpp`

This module converts raw TAP protocol strings into typed C++ objects:
1. **JSON Deserialization**:
   - `parseLookResponse`: Deserializes `OK { "room": { "id", "name", "description", "exits": {...} }, "players": [...], "items": [...], "npcs": [...] }` into the `RoomInfo` struct.
   - `parseQuestResponse` / `parseQuestsResponse`: Parses individual quests or quest lists (`QuestInfo`), automatically extracting goals (e.g., required item counts, target NPC).
   - `parseInventoryResponse`: Parses `OK [ "flour", "egg" ]` into `std::vector<std::string>`.
   - `parseUseResponse`: Parses consumable item effects (`UseInfo`: HP, max HP, attack bonus, received item).
   - `parseCombatResponse`: Parses attack rounds and damage (`CombatInfo`).
   - `parseGenericOkJson`: Recursive fallback parser that flattens any unknown JSON object for clean display.
2. **Visual Formatting**:
   - `formatRoomInfo`, `formatQuestInfo`, `formatUseInfo`, `formatCombatInfo` produce clean, formatted lines ready to be pushed to the terminal log.
3. **Chat Commands**:
   - `parseChatCommand`: Detects user commands such as `CHAT ROOM <msg>`, `CHAT GLOBAL <msg>`, or `CHAT PARTY <msg>` and prepares the corresponding network payload.

---

### Hub & Map Management (`map_resolver`, `image_renderer`, `assets/maps`)
- Files: `includes/map_resolver.hpp`, `src/map_resolver.cpp`, `includes/image_renderer.hpp`, `src/image_renderer.cpp`, `assets/maps/`

This subsystem handles graphical map rendering for the current room:
1. **Map Resolution (`map_resolver`)**:
   - When a `LOOK` or movement occurs, the client searches `assets/maps/` using a 4-level fallback cascade:
     1. Normalized full room name (e.g., `assets/maps/hub.png` or `room_hub.png`).
     2. Short identifier following the dot (e.g., `zone.smycka` -> `smycka.png`).
     3. Cleaned room display name.
     4. Default fallback image: `assets/maps/default.png`.
2. **High-Precision Terminal Rendering (`image_renderer`)**:
   - Decodes PNG images via `stb_image`.
   - Premultiplies RGB channels by alpha to eliminate dark edge artifacts.
   - Applies an area-averaging box filter downsampling with caching.
   - Uses the Unicode upper half-block character `▀`:
     - Foreground color = top pixel.
     - Background color = bottom pixel.
     - Displays **2 vertical pixels per terminal text row** in full 24-bit TrueColor!

---

### Centralized State Management (`app_state`)
- Files: `includes/app_state.hpp`, `src/app_state.cpp`

`AppState` serves as the thread-safe bridge between the background networking thread and the FTXUI UI thread:
- **Dedicated Mutex Protection**: Prevents lock contention across log entries, chat messages, quests, minimap, and command history.
- **Quest Cache & Incremental Merging**: `setActiveQuests` and `upsertQuest` preserve previously fetched detailed descriptions while updating quest progress incrementally.
- **Chat Anti-Echo Mechanism**: `recordLastChat` and `checkAndClearIfLastChat` prevent locally sent messages from being duplicated when the server echoes them back via `EVT ... CHAT`.
- **Command History**: Stores up to 100 commands with consecutive duplicate suppression.

---

### User Interfaces (`login_view`, `game_view`, `ui_helpers`)
- Files:
- `includes/login_view.hpp`, `src/login_view.cpp`: Login modal with input validation, Enter key to submit, and Escape key to cancel.
- `includes/game_view.hpp`, `src/game_view.cpp`: Main FTXUI view. Assembles widgets, handles local shortcuts (`CLEAR` / `CLEAN` to clear screen logs, `QUIT` / `EXIT` to disconnect cleanly), and manages directional arrow history navigation.
- `includes/ui_helpers.hpp`, `src/ui_helpers.cpp`:
  - `shrinkable(Element child)`: Custom DOM node reducing `min_x` to 1, allowing the terminal window to shrink gracefully without breaking FTXUI's layout solver.
  - `wrapLine()`: Formats log and chat messages with contextual colors based on sender and channel.

---

### Entry Point & Lifecycle (`main.cpp`)
- File: `src/main.cpp`

- Ignores `SIGPIPE` (`signal(SIGPIPE, SIG_IGN)`) to prevent abrupt crashes if the socket is closed by the server during write operations.
- Main session loop: if the connection drops during gameplay, the client automatically falls back to the login modal with a `"Connection lost"` error message.
- Exits with return code `0` when the user explicitly requests an exit (`QUIT` or Esc on the login screen).

---

## 3. Getting Started (Practical Guide)

### System Requirements

To build and run the client, ensure the following tools are installed:
- **OS**: Linux or macOS.
- **C++ Compiler**: `g++` or `clang++` with C++17 support.
- **Build Tools**: `make`, `cmake` (used by Makefile to build FTXUI).
- **Terminal Emulator**: A terminal emulator supporting 24-bit TrueColor and Unicode (e.g., Alacritty, Kitty, WezTerm, iTerm2, GNOME Terminal, xterm).

### Make Commands

The project includes a complete `Makefile` with clean output and modular build steps:

```bash
# Build the client (compiles FTXUI once if needed, then builds the client)
make

# Build in verbose mode (display compiler commands and warnings in terminal)
make OUT=1

# Build and log all output to a file
make OUT=build.log

# Build and directly run the client
make run

# Compile only the FTXUI library
make install

# Clean project object files (.obj)
make clean

# Clean object files (.obj) and the tap_client_tui binary (preserves FTXUI build)
make fclean

# Fast rebuild of the client (fclean + all, without rebuilding FTXUI)
make re

# Run code style conformance checks (cpplint)
make lint
```

---

### Running & In-Game Usage

1. **Launch**:
   ```bash
   ./tap_client_tui
   ```
2. **Login Screen**:
   - Enter your **Nickname** (e.g. `Arthur`).
   - Enter the server **IP** (defaults to `127.0.0.1`).
   - Enter the server **Port** (defaults to `4242`).
   - Press **Enter** or click **Connect**.
3. **In-Game Controls**:
   - **Type a command**: Type directly on the keyboard; the cursor is displayed in the bottom input bar `> [text]`.
   - **Up / Down Arrow**: Cycle through previous command history.
   - **Enter**: Submit command.

---

### Supported TAP Protocol Commands

| Command | Example Usage | Description |
|---|---|---|
| `LOOK` | `LOOK` | Inspects the current room, refreshing exits, items, players, and the minimap. |
| `MOVE <direction>` | `MOVE north` | Moves the player toward an exit (e.g., `north`, `south`, `east`, `west`). |
| `INVENTORY` | `INVENTORY` | Requests the inventory item list (updates the left panel). |
| `TAKE <item>` | `TAKE flour` | Picks up an item from the floor and updates the inventory. |
| `DROP <item>` | `DROP flour` | Drops an item onto the floor and updates the inventory. |
| `USE <item>` | `USE egg` | Consumes or activates an item from the inventory (+25 HP). |
| `QUESTS` | `QUESTS` | Lists the player's quests (`active` with their progress, and `completed`). |
| `QUEST <npc>` | `QUEST chef` | Asks an NPC of the room for a quest: gives the next one, shows the current one, or completes it if its goal is reached. |
| `ATTACK <npc>` | `ATTACK queen` | Attacks a hostile NPC in the current room. |
| `TALK <npc>` | `TALK waiter` | Talks to an NPC in the current room. |
| `STATUS` | `STATUS` | Shows HP, max HP, attack, defense and state (`healthy`, `wounded`, `combat`). |
| `WHO` | `WHO` | Number of connected players. |
| `GROUP CREATE` / `INVITE <player>` / `JOIN <player>` / `LEAVE` | `GROUP INVITE bob` | Creates a group, invites a player, joins the group of the player who invited you, leaves the group. |
| `CHAT ROOM <msg>` | `CHAT ROOM Hello everyone!` | Sends a chat message visible only in the current room. |
| `CHAT GLOBAL <msg>`| `CHAT GLOBAL Greetings all` | Sends a broadcast message to all connected players. |
| `CHAT GROUP <msg>` | `CHAT GROUP Let's go` | Sends a message to your group. `CHAT PARTY <msg>` also works: the client sends it as `CHAT GROUP`. |
| `CLEAR` / `CLEAN` | `CLEAR` | *(TUI client command)* Clears the terminal log buffer and completed quests. |
| `QUIT` / `EXIT` | `QUIT` | Sends `QUIT` to the server and exits the application cleanly. |

---

## 4. Connection & Integration with Other Projects

### TAP Network Protocol Specification

The TUI client is completely agnostic to server implementation details, as long as the server conforms to the text-based TCP protocol defined below:

1. **Frame Format**:
   - Each message is a UTF-8 encoded string terminated by a newline character `\n`.
2. **Initial Handshake Sequence**:
   ```
   Client                             Server
     |                                   |
     | ------ TCP Connection (SYN) ----> |
     | <--- 1. OK hello proto=1\n ------ | (greeting, sent on connection)
     | ------ 2. CONNECT <nickname>\n -> | (e.g. "CONNECT alice\n")
     | <--- 3. OK connected\n ---------- | (or "ERR 201 NAME_IN_USE\n")
     |                                   |
   ```
3. **Synchronous Response Format**:
   - Success: `OK <json_payload_or_text>\n`
     - Room example (`LOOK`): `OK {"room":{"id":"hub","name":"Hub - The Restaurant and Farm","description":"...","exits":{"east":"fugoumitak","north":"jeetomal","west":"komolexos"}},"players":["alice"],"items":["flour"],"npcs":["chef","waiter"],"names":{"chef":"Chef Gusto","flour":"Flour",...}}\n`
     - Inventory example: `OK ["flour","egg"]\n`
     - Quest example (`QUEST chef`): `OK {"quest_id":"fetch_beef","description":"...","reward":"...","status":"active","progress":"..."}\n`
     - Quest list example (`QUESTS`): `OK [{"quest_id":"defeat_cow_queen","status":"completed"},{"quest_id":"fetch_beef","status":"active","progress":"..."}]\n`
   - Error: `ERR <code> <NAME>\n` (e.g. `ERR 301 NO_EXIT`)
4. **Asynchronous Events (Broadcasts)**:
   - Chat: `EVT <SCOPE> CHAT <nickname> <message>\n` (where `<SCOPE>` is `ROOM`, `GLOBAL`, or `GROUP`)
   - Combat: `EVT ROOM COMBAT {"attacker":"alice","target":"queen","attacker_hp":85,"target_hp":18,"damage":12,"damage_taken":5,"status":"combat"}\n` (`status` is `combat`, `victory` or `defeated`)

---

### Connecting to the Official TAP Server

If you have access to the complete TAP repository (including the Go server in `../server/`):

1. **Start the Go server**:
   ```bash
   cd ../server
   go run ./cmd/server
   # The server listens on port 4242 (all interfaces) by default
   ```
   *(Or via the root Makefile: `make run-server`)*

2. **Start the TUI client**:
   ```bash
   cd ../tui
   make run
   ```
   *(Or via the root Makefile: `make run-client`)*
3. In the login modal, keep `127.0.0.1` and `4242`, choose a nickname, and confirm.

---

### How to Reuse or Adapt this Module

To integrate this client into another project or software suite:

1. **Adding new graphical maps**:
   - Simply place a PNG file named `<room_id>.png` or `<room_name>.png` inside `assets/maps/` (e.g. `assets/maps/dungeon.png`). The resolver `resolveMapPath()` will automatically discover it as soon as the corresponding room is visited.
2. **Adapting the Protocol**:
   - Exchanged JSON formats are centralized in `src/protocol.cpp` (`from_json` and `parse*Response`). Modify these functions to map fields to your own data structures.
3. **Usage as a Submodule or Library**:
   - The `NetworkClient` and `AppState` classes are completely decoupled from the terminal UI. You can reuse them to power headless bots, 2D/3D graphical frontends (e.g. Raylib, Unreal, WebAssembly), or integration test suites.
