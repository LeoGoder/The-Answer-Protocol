# TAP — Server

Go server of **The Answer Protocol (RFC 42TAP)**. It owns the whole world state: players,
rooms, items, NPCs, fights, quests and groups. Clients only send commands and display
what the server answers and pushes.

This file is the developer guide of the server: how to run it, how the code is organised,
the format of `world.json` and a reference of every command.
**The design choices required by the subject are explained in the
[root README](../README.md)**, which this file does not repeat:

- [Architecture](../README.md#architecture) — dispatcher, concurrency model
- [Protocol Implementation](../README.md#protocol-implementation) — every deviation from
  and addition to the RFC, added error codes
- [Combat System](../README.md#combat-system) and [Quest System](../README.md#quest-system)
- [World Design](../README.md#world-design) — story, map, NPCs, items
- [Server Logging](../README.md#server-logging) — format, event types, abuse detection
- [Testing](../README.md#testing)

## Run

Requirements: **Go 1.18** or later. The server uses the **standard library only**: no
module to download.

```bash
make run-server               # from the repository root
make run-server-quiet         # full log in server/server.log, screen without POSITION / ALIVE

cd server && go run ./cmd/server                                 # same as make run-server
cd server && go run ./cmd/server -port 5000 -world data/world.json
```

| Flag | Default | Meaning |
|---|---|---|
| `-port` | `4242` | TCP port to listen on (all interfaces) |
| `-world` | `data/world.json` | world file, relative to `server/` |

Connect with any TCP client:

```
$ nc localhost 4242
OK hello proto=1
CONNECT alice
OK connected
LOOK
OK {"room":{"id":"hub","name":"Hub - The Restaurant and Farm",...},"players":["alice"],...}
```

Lint (also run by `make lint` at the root):

```bash
cd server && gofmt -l . && go vet ./...     # gofmt must print nothing
```

## Code layout

About 2 500 lines, in three layers that only depend downwards:

```
cmd/server/          network: connections, sending, monitoring
  main.go            flags, world loading, accept loop, 64-connection limit, ticker goroutines
  client.go          one goroutine per connection: greeting, line reading, CONNECT / QUIT, cleanup
  hub.go             username → connection map; Deliver sends events outside the game lock
  monitor.go         command flood (per connection) and rapid connections (per IP) detection
internal/protocol/   text: no game, no network
  parser.go          CheckLine (UTF-8, control characters), Parse (name, arguments, raw line), MaxLineLength
  validator.go       argument count of each command, CHAT scope, GROUP action
  username.go        username rules
  response.go        every OK / ERR line as a constant
internal/game/       rules: no network
  state.go           State and its mutex, Player / NPC / Group, Join, Leave, recipient helpers
  handler.go         Handle (the dispatcher), LOOK, WORLD, MOVE, WHO, CHAT
  world.go           world.json structures, loading and validation
  items.go           item instances, TAKE / DROP / USE, rewards, Tick (expiry, respawn)
  npc.go             TALK, ATTACK, damage, defeat and respawn, STATUS
  quest.go           QUEST, QUESTS, quest progress
  group.go           GROUP CREATE / INVITE / JOIN / LEAVE
  position.go        POSITION and the 24-per-second EVT ROOM POSITION
internal/logger/     JSON lines: INFO / WARN to stdout, ERROR to stderr
data/world.json      the world
```

Suggested reading order: `main.go` → `client.go` → `protocol/` → `hub.go` →
`state.go` → `handler.go` → `world.go` → `items.go` → `npc.go` → `quest.go` →
`group.go` → `position.go`.

## Life of a command

```
client.go   ReadSlice('\n')              no line for 60 s       → connection closed (before CONNECT only)
                                         line > 1024 bytes      → ERR 400 LINE_TOO_LONG
            protocol.CheckLine           bad UTF-8 / control    → ERR 400 BAD_REQUEST
            protocol.Parse               empty line             → ignored
            log command_received
            protocol.Validate            wrong arguments        → ERR 400 BAD_REQUEST
            QUIT                                                → OK bye, connection closed
            CONNECT                      state.Join             → OK connected / ERR 201
            not connected yet                                   → ERR 202 NOT_AUTHENTICATED
game        state.Handle  (lock) → handle: switch on the command name → one function (unlock)
client.go   reply to the sender, then hub.Deliver(events) to the other players
```

Rules that keep the code simple:

- **`game` never writes to the network.** A command returns a `Result`: the reply line
  for the sender and a list of `Event`s (a line and the usernames to send it to). Events
  raised deep in the logic (a reward taken from the floor…) are `queue`d and added by
  `Handle` / `Tick` with `flush`.
- **One mutex for the whole state.** Exported methods of `State` (`Handle`, `Join`,
  `Leave`, `Tick`, `PositionEvents`) lock it; unexported methods expect it held and never
  call an exported one, because Go mutexes are not reentrant.
- **Send after unlock.** `hub.Deliver` writes each event with a 2-second deadline. A
  failed or timed-out write closes that connection, so a client that stops reading
  delays the others once (at most 2 s), and its own goroutine removes the player.
- **Leave removes first, then announces.** On QUIT or disconnect, the player is removed
  from the state before the `PRESENCE LEAVE` and `STATS` events are built, so the count is
  right and nothing is sent to the closed connection.

## Game state

| Data | Where | Notes |
|---|---|---|
| World definitions | `world` | read-only after loading: rooms, `ItemDef`, `NPCDef`, `QuestDef` |
| Players | `State.players` | room, HP / max HP / attack / defense, inventory, fight target, group, quests, position |
| Items on the floor | `State.floor` | per room, sorted by ID then serial |
| Spawn points | `State.spawns` | one per item placed in a room by `world.json` |
| NPCs | `State.npcs` | current HP, respawn time, next dialogue line |
| Groups | `State.groups` | members (leader first), pending invitations |

Definitions and live state are kept apart: `NPCDef.HP` is the full HP, `npcState.HP` the
current one, so a respawn simply copies the first into the second.

**Item instances**: `Item{ID, Serial}`. `mint` creates one (serial 0, 1, 2… per item ID)
and logs `item_created`; `deleteItem` logs `item_deleted` with the reason (`used`,
`delivered`, `expired`, `owner_disconnected`). An instance is always in one place only:
a floor or one inventory. `grant` (rewards, drops, content of a used item) creates a new
instance while the item exists fewer times than there are players online; otherwise it
moves an instance lying on a floor.

**Timers** (in `Tick`, once per second): dropped items expire after 30 s, empty spawn
points refill after 30 s, defeated enemies come back after 30 s.

## world.json format

```json
{
  "start_room": "hub",
  "respawn_room": "hub",
  "rooms":  [{ "id": "hub", "name": "…", "description": "…",
               "exits": { "north": "jeetomal" }, "items": ["flour"], "npcs": ["chef"] }],
  "items":  [{ "id": "egg", "name": "Egg", "description": "…",
               "heal": 25, "contains": "" }],
  "npcs":   [{ "id": "queen", "name": "The Queen Of Cows", "description": "…",
               "dialogue": ["…"], "hostile": true, "hp": 42, "attack": 12, "defense": 4,
               "drops": "" }],
  "quests": [{ "id": "fetch_egg", "type": "fetch", "giver": "chef", "description": "…",
               "target": "egg",
               "reward": { "name": "…", "item": "", "max_hp": 15, "attack": 3 } }]
}
```

| Field | Meaning |
|---|---|
| `start_room` / `respawn_room` | where players appear / come back after a defeat (`respawn_room` defaults to `start_room`) |
| `rooms[].exits` | `north`, `south`, `east` or `west` → room ID |
| `rooms[].items` | spawn points: each item listed starts on this floor and comes back 30 s after being taken (while it exists fewer times than players online) |
| `items[].heal` | HP restored by `USE`; `> 0` makes the item usable |
| `items[].contains` | item given by `USE` (also makes the item usable) |
| `npcs[].hostile` | `true`: an enemy, can be attacked, needs `hp` and `attack` above 0 |
| `npcs[].drops` | item given to the players credited with the defeat (enemies only) |
| `quests[].type` | `fetch` (`target` = item to bring) or `defeat` (`target` = enemy) |
| `quests[].reward` | `name` shown to the player, optional `item`, `max_hp` and `attack` bonuses |

A giver hands out its quests **in the order of the file**.

**Checks at start** — the server refuses to start, with a clear message, if:

- an ID is missing or used twice, or `start_room` / `respawn_room` does not exist;
- an exit is not one of the four directions, or leads to an unknown room;
- a room lists an unknown item or NPC, or an item / NPC is placed in two rooms;
- an item or NPC has no name, an NPC has no dialogue, two items / NPCs share a name
  (`TAKE <name>` would be ambiguous);
- an enemy has no HP or attack, a non-enemy drops something, a drop or content item does
  not exist, the respawn room contains an enemy;
- a quest has no description or reward name, an unknown giver or reward item, a `fetch`
  item that can be found nowhere (no room, no drop), a `defeat` target that is not an
  enemy, or an unknown type.

## Command reference

Command names, CHAT scopes, GROUP actions and directions are case-insensitive. Items and
NPCs are named by ID or display name; names can contain spaces.

| Command | Success | Errors |
|---|---|---|
| `CONNECT <name>` | `OK connected` | `201 NAME_IN_USE`, `400` (invalid name, already connected) |
| `LOOK` | `OK {"room":{id,name,description,exits},"players":[…],"items":[…],"npcs":[…],"names":{…}}` | |
| `MOVE <direction>` | `OK room=<id>` | `301 NO_EXIT` |
| `WHO` | `OK players=<n>` | |
| `CHAT GLOBAL\|ROOM\|GROUP <msg>` | `OK` (the sender also gets the `EVT … CHAT`) | `401 NOT_IN_GROUP`, `400` |
| `GROUP CREATE` | `OK group=<id>` | `402 ALREADY_IN_GROUP` |
| `GROUP INVITE <name>` | `OK` | `401`, `402`, `404 PLAYER_NOT_FOUND`, `400` (yourself) |
| `GROUP JOIN <name>` | `OK group=<id>` | `402`, `404 PLAYER_NOT_FOUND`, `401` (that player has no group), `403 NOT_INVITED` |
| `GROUP LEAVE` | `OK` | `401` |
| `TAKE <item>` | `OK taken=<id>` | `404 ITEM_NOT_FOUND`, `409 ALREADY_HAVE_ITEM` |
| `DROP <item>` | `OK dropped=<id>` | `404 ITEM_NOT_IN_INVENTORY` |
| `INVENTORY` | `OK ["egg","beef"]` | |
| `USE <item>` | `OK {"used","hp","max_hp"}` | `404 ITEM_NOT_IN_INVENTORY`, `408 ITEM_NOT_USABLE` |
| `TALK <npc>` | `OK <dialogue line>` | `404 NPC_NOT_FOUND` |
| `ATTACK <npc>` | `OK {"attacker","target","attacker_hp","target_hp","damage","damage_taken","status"[,"room"]}` | `404 NPC_NOT_FOUND`, `405 NPC_NOT_HOSTILE` |
| `STATUS` | `OK {"hp","max_hp","status","attack","defense"[,"target"]}` | |
| `QUEST <npc>` | `OK {"quest_id","description","reward","status","progress"}` | `404 NPC_NOT_FOUND`, `406 NO_QUEST_AVAILABLE` |
| `QUESTS` | `OK [{"quest_id","status"[,"progress"]},…]` | |
| `WORLD` | `OK {"hostile":[…],"quest_givers":[…],"usable":[…]}` | |
| `POSITION <x> <y>` | `OK` (0 ≤ x, y ≤ 100000) | `400` |
| `ALIVE` | `OK Alive` | |
| `QUIT` | `OK bye`, then the connection closes | |

Any command: `400 BAD_REQUEST` (wrong arguments), `400 UNKNOWN_COMMAND`,
`400 LINE_TOO_LONG`, `202 NOT_AUTHENTICATED` before `CONNECT`.
The events sent by the server are listed in the
[root README](../README.md#protocol-implementation).

## Limits and constants

| Constant | Value | File |
|---|---|---|
| `MaxLineLength` | 1024 bytes | `protocol/parser.go` |
| `MaxClients` | 64 connections | `cmd/server/main.go` |
| `AuthTimeout` | 60 s without a line before `CONNECT` | `cmd/server/client.go` |
| `WriteTimeout` | 2 s | `cmd/server/hub.go` |
| `PositionRate` | 24 per second | `cmd/server/main.go` |
| `FloodLimit` / `FloodWindow` | 60 commands / 1 s | `cmd/server/monitor.go` |
| `RapidLimit` / `RapidWindow` | 5 connections / 10 s per IP | `cmd/server/monitor.go` |
| `StartHP` / `StartAttack` / `StartDefense` | 100 / 10 / 2 | `game/state.go` |
| `ItemRespawnDelay` / `DropLifetime` | 30 s / 30 s | `game/items.go` |
| `EnemyRespawnDelay` | 30 s | `game/npc.go` |
| `PosMax` | 100000 | `game/position.go` |
