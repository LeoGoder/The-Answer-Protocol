# TAP — GUI client

Graphical client for **The Answer Protocol (RFC 42TAP)**, written in C++ with
[raylib](https://www.raylib.com/). It connects to the TAP server over TCP and turns the
text protocol into a small top-down world: you walk around the islands, click NPCs and
items, fight, follow quests and chat with the other players.

The project overview, the server and the protocol are described in the
[root README](../README.md) and the [server README](../server/README.md).

## Build and run

### Requirements

- Linux with X11 and OpenGL
- `g++`, `make`
- raylib is **vendored** in `raylib/` (raylib 6.5-dev sources) and built by the Makefile:
  no download is needed at build time. It needs the usual X11 / OpenGL development
  headers (`libx11-dev`, `libxrandr-dev`, `libxinerama-dev`, `libxcursor-dev`,
  `libxi-dev`, `libgl1-mesa-dev`), already installed on the 42 machines.

### Commands

From the repository root:

```bash
make run-server             # terminal 1: start the server on port 4242
make run-client-gui         # terminal 2: build and start the GUI
make run-client-gui-quiet   # same, without the 24-per-second POSITION / ALIVE lines
```

From `new_gui/`:

| Target | Effect |
|---|---|
| `make` / `make all` | build raylib, then the `tap` binary |
| `make run` | build and start |
| `make clean` | remove the object files (raylib's too) |
| `make fclean` | `clean` + remove the binary |
| `make re` | rebuild everything |
| `make lint` | run `cpplint` on `src/` and `inc/` |

The code is compiled with `-Wall -Wextra -Werror`.

### Connecting to another machine

The server address is set in `inc/game_world.hpp`:

```cpp
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 4242
```

To join a server on another computer of the same network, replace `SERVER_IP` with that
computer's IP address and run `make` again (the Makefile tracks the headers, so only the
files that include it are rebuilt).

## How to play

1. **New game** → pick a free slot → type a name → the client connects with
   `CONNECT <name>`. **Load** reconnects with a name saved earlier; a slot can also be
   deleted there. Saves only keep the name (`saves/TAP_xx/save.bin`); the game state
   lives on the server.
2. In the menus: `Enter` / `Space` / left click to confirm, `Esc` / right click to go back.

### Controls in game

| Input | Action | Protocol |
|---|---|---|
| `W` `A` `S` `D` | walk inside the room (diagonals allowed) | `POSITION` (24 per second) |
| `E` on a door, or click an exit | go to the next island | `MOVE <direction>` |
| Left click on an item on the ground | pick it up | `TAKE <item>` |
| Left click on an NPC | open the dialog | `TALK <npc>` |
| Dialog buttons | accept / complete a quest, attack an enemy | `QUEST <npc>`, `ATTACK <npc>` |
| Left click on an inventory slot | use the item (food heals) | `USE <item>` |
| Right click on an inventory slot | drop the item | `DROP <item>` |
| `/` | open the chat input, `Enter` sends, `Esc` closes | `CHAT GLOBAL\|ROOM\|GROUP <msg>` |
| `QUIT` button (top left) | leave the server | `QUIT` |
| `Esc` | back to the main menu | |

The dialog closes by itself when you click outside it, or when its NPC is no longer in the
room (you moved, or the enemy was defeated).

### Screen layout

- **Top left**: HP and number of players online, `QUIT` button, inventory (7 slots,
  item name on hover).
- **Bottom left**: panel with three tabs
  - `LOG`: replies and events, plus `LOOK`, `STATUS`, `QUESTS`, `WHO` buttons;
  - `GROUP`: create, invite, join or leave a group, list of members;
  - `CHAT`: choose the channel (`GLOBAL`, `ROOM`, `GROUP`) and type.
- **Right**: minimap of the neighbouring islands and the quest log.
- Other players in the room are drawn at the position they send (`EVT ROOM POSITION`).

## Architecture

```
main.cpp        window, 640×480 render texture, CRT post-process shader
tap.cpp         scene manager: creates / deletes the current scene
src/scene/
  splash_screen/  main menu, new game, load, error screens
  game_world/     connection, network loop, map, player, HUD, dialogs, chat
src/server.cpp  send_message / receiv_msg (thin wrappers around send / recv)
src/save_load.cpp  save slots
res/            sprites, Tiled maps (*.json), shader
```

### Rendering

Every frame is drawn into a 640×480 `RenderTexture`, then scaled to the window through a
fragment shader (`res/old_screen_shaders.fs`) that adds the old-screen effect: curved
screen, colour split, vignette and scanlines. Pixel art therefore stays sharp at any
window size, and the mouse position is converted back to the 640×480 space
(`get_virtual_mouse_position`).

Each island is a [Tiled](https://www.mapeditor.org/) map exported as JSON (`res/*.json`):
it gives the tiles to draw, the obstacles the player collides with, and the places where
`E` moves to a neighbouring island.

### Networking

The client is **single-threaded**: the socket is non-blocking and is polled once per
frame, so a slow network never freezes the window.

- **Connection**: non-blocking `connect`, checked with `select` (100 ms); on failure the
  connection error screen is shown.
- **Reading**: the received bytes are appended to a buffer and only complete lines
  (ending with `\n`) are handled; the rest waits for the next `recv`.
- **After `CONNECT`**: the client asks `LOOK`, `INVENTORY`, `WORLD`, `STATUS`, `QUESTS`
  and `WHO` to build its first view.
- **Events**: `EVT ROOM PRESENCE`, `EVT ROOM ITEM TAKE / DROP / SPAWN / GONE`,
  `EVT ROOM NPC SPAWN`, `EVT ROOM COMBAT`, `EVT ROOM POSITION`, chat and group events
  update the view without asking the server again.
- **Keep-alive**: `ALIVE` is sent every 10 s; if no `OK Alive` comes back for 35 s, the
  client closes the socket and shows the connection error screen.

### Protocol extensions used

The GUI relies on some server extensions (documented in the
[server README](../server/README.md)); a server that only implements the RFC is not
enough for it:

| Extension | Why the GUI needs it |
|---|---|
| `WORLD` | which NPCs can be attacked, which give quests, which items can be used: the GUI shows only the buttons that make sense |
| `names` field of `LOOK` | display names ("Chef Gusto") instead of IDs ("chef") |
| `POSITION` / `EVT ROOM POSITION` | see the other players move in real time |
| `ALIVE` | detect a dead connection |
| `EVT ROOM ITEM …`, `EVT ROOM NPC SPAWN`, `EVT GROUP MEMBERS` | keep the room, the floor and the group up to date |

## Resources

- [raylib cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html) and
  [examples](https://www.raylib.com/examples.html)
- [Tiled documentation](https://doc.mapeditor.org/) — JSON map format
- [The Book of Shaders](https://thebookofshaders.com/) — fragment shaders
- `man 2 connect`, `man 2 recv`, `man 2 select` — non-blocking sockets
- TODO(Leo): credits of the sprite packs used in `res/` (author, link, licence)

### AI usage

TODO(Leo): which tasks and which parts of the GUI used AI.
