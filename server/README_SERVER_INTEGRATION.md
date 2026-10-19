# TAP Server – Design and Integration Notes

These notes cover the server only. They are written so that the relevant
sections can be copied into the root `README.md` (Architecture, Protocol
Implementation, Combat System, Quest System, World Design, Server Logging,
Testing).

## Building and running

- Go **1.18.1**, standard library only (no external dependency).
- Run: `cd server && go run ./cmd/server`
- Options: `-world <path>` (default `data/world.json`), `-port <port>` (default `4242`).
- Lint: `cd server && gofmt -l . && go vet ./...` (`gofmt -l` must print nothing).
- If the world file is invalid, the server logs the reason and exits with code 1.

## Architecture

```
cmd/server/      network layer (package main)
  main.go        accept loop, one goroutine per client, read loop, auth
  client.go      ClientHub: username -> connection, delivery of events
  monitor.go     abuse detection (command flooding, rapid connections)
internal/protocol/   text protocol, no game knowledge
  parser.go      line -> Command{Name, Args}
  validator.go   number/shape of arguments
  username.go    username rules
  line.go        UTF-8 + control character check, 1024-byte limit
  response.go    every reply string (RFC codes + local extensions)
internal/game/   game rules, no network knowledge
  world.go       world.json format, loading and validation
  state.go       live state, Event/Result, Join/Leave, recipient helpers
  handler.go     Handle() dispatcher, LOOK, MOVE, WHO, CHAT
  items.go       TAKE, DROP, INVENTORY, USE
  npc.go         NPC presence/respawn, TALK
  combat.go      ATTACK, STATUS
  quest.go       QUEST, QUESTS
  group.go       GROUP CREATE/INVITE/JOIN/LEAVE
internal/logger/ JSON logger
```

- **Concurrency**: one goroutine per client. The whole game state is behind
  one `sync.Mutex` inside `game.State`; every exported method takes it once,
  internal (lower-case) methods assume it is held.
- **Events**: `State.Handle` returns a `Result{Reply, Events}`. The game decides
  who receives what (`toRoom`, `toGroup`, `toAll`, `toPlayer`, each with an
  optional player to exclude); `ClientHub.Deliver` sends after the lock is
  released, so a slow client never blocks the game.
- **Disconnects**: a deferred cleanup runs whatever ends the connection (QUIT,
  network error, closed socket). The player is removed from the state
  **before** the LEAVE events are broadcast. Their items are dropped in the
  room and they leave their group. A failed write during a broadcast is
  logged and the other recipients still receive the event. Writes time out
  after 2 seconds.
- **Limits**: 1024 bytes per line (`ERR 400 LINE_TOO_LONG`), 64 simultaneous
  connections (`ERR 900 CONNECTION_FAILED`).

## Protocol implementation and deviations

All RFC commands and events are implemented. Command names and directions are
case-insensitive. `\r\n` line endings are accepted.

Additions and deviations (the RFC does not define these cases):

| Reply / event | When |
|---|---|
| `ERR 202 NOT_AUTHENTICATED` | any command other than CONNECT/QUIT before CONNECT |
| `ERR 400 BAD_REQUEST` | wrong arguments, invalid username, control characters, invalid UTF-8, CONNECT twice |
| `ERR 400 UNKNOWN_COMMAND` | unknown command name |
| `ERR 400 LINE_TOO_LONG` | line over 1024 bytes |
| `ERR 404 PLAYER_NOT_FOUND` | GROUP INVITE/JOIN with a player who is not online |
| `ERR 403 NOT_INVITED` | GROUP JOIN without an invitation |
| `ERR 408 ITEM_NOT_USABLE` | USE on an item that does nothing |
| `ERR 500 INTERNAL_ERROR` | should never happen |
| `EVT ROOM COMBAT <json>` | combat round, sent to the other players of the room |
| `USE <item>` | additional command: use a healing item |

- Usernames: 1–32 Unicode letters/digits or `_`, no more than 3 identical
  characters in a row, must not contain `admin`.
- Items and NPCs can be named by full ID (`item.1`), ID without prefix
  (`1`) or display name (`Object 1`), any case.
- `GROUP JOIN` accepts a username or a group ID (`group.1`).
- `CHAT` ROOM/GLOBAL/GROUP messages are also sent back to the sender.
- `EVT STATS players=<n>` is sent to everybody on each connection/disconnection.
- `ATTACK` replies with
  `{"attacker","target","attacker_hp","target_hp","damage","damage_taken","status"}`,
  plus `"room"` when the player was defeated and respawned.
  `status` is `combat`, `victory` or `defeated`.
- `STATUS` adds `attack`, `defense` and `target` (while fighting) to the RFC fields;
  `status` is `healthy`, `wounded` or `combat`.

## Combat system

- Players start with 100 HP, attack 10, defense 2.
- Turn-based, one `ATTACK <npc>` = one round. The player always acts first
  (initiative), then the enemy hits back in the same round if it is alive.
- `USE <item>` heals; during a fight the enemy then hits back.
- `MOVE` leaves the fight (no FLEE command needed).
- Damage = `attack + random(0..4) - defense`, minimum 1.
- An enemy's HP is shared: several players can fight it together. A defeated
  enemy disappears from LOOK and comes back with full HP after 30 seconds.
- At 0 HP the player respawns in the safe `respawn_room` (no enemy allowed
  there) with half of their max HP.
- Every round is logged (`combat_round`) and broadcast as `EVT ROOM COMBAT`.

Why: one command per round keeps the server simple (no timers, no combat
goroutine) and makes the outcome of each command visible immediately in the
reply. The small random part keeps fights from being fully predictable.

## Quest system

`QUEST <npc>` does everything:

1. If you have an unfinished quest from this NPC: if the objective is done the
   quest is completed and the reward applied, otherwise the progress is shown.
2. Otherwise the NPC gives its next quest you have not taken (file order).
3. Otherwise `ERR 406 NO_QUEST_AVAILABLE`.

Types (`target` field):

- **fetch** (`target` = item): have the item in your inventory when you report.
  It is handed over and goes back to its spawn room, so others can do it too.
- **defeat** (`target` = enemy NPC): defeat it. Group members in the same room
  also get the credit.

Rewards increase max HP and/or attack. Progress is tracked automatically;
completion is manual (report to the giver).

## World design

`data/world.json` holds four lists (`rooms`, `items`, `npcs`, `quests`); every
element has an explicit `"id"`. The IDs are the contract with the clients
(they map `room.N` to their own images), so they must stay stable.

The server checks at startup: unique IDs, every exit/item/NPC/quest reference
exists, each item and NPC is placed in one room only, exits use
north/south/east/west, the respawn room has no enemy, a fetch item is placed
somewhere.

The current world is a generic base for the team to fill in:

```
 room.2 ─── room.3 ─── room.4
   │          │          │
 room.1 ─── room.5 ─── room.6
 (start)                 │
                       room.7 ─── room.8   (optional branch)
```

- 8 rooms, two loops (1-2-3-5 and 3-4-6-5) and a branch (6-7-8).
- NPCs: `npc.1` dialogue (room.1), `npc.2` quest giver (room.3),
  `npc.3` enemy, 80 HP, strong enough to defeat a player (room.7).
- Items: `item.1` (heals 20), `item.2` (heals 40), `item.3` (fetch quest),
  `item.4`.
- Quests: `quest.1` fetch `item.3`, `quest.2` defeat `npc.3`, both from `npc.2`.

## Server logging

One JSON object per line, `time` (UTC, nanoseconds), `level`, `event` first:

```
{"time":"2026-10-18T16:04:45.423Z","level":"INFO","event":"command_received","args":["north"],"command":"MOVE","ip":"127.0.0.1:52344","player":"alice"}
```

- INFO/WARN on stdout, ERROR on stderr.
- Connections/disconnections with IP, every command with player and arguments,
  every message sent, world changes (`item_taken`, `item_dropped`, `item_used`,
  `npc_talk`, `combat_round`, `npc_defeated`, `player_defeated`, ...), quests
  (`quest_accepted`, `quest_progress`, `quest_completed`), groups.
- Abuse: `command_flood` (more than 20 commands per second from one client),
  `rapid_connections` (more than 5 connections in 10 seconds from one IP),
  `line_too_long`, `invalid_line`, `connection_refused`.
- Each line is built in memory and written with a single call, so logging
  stays cheap and lines never mix.

Example: `go run ./cmd/server | grep '"level":"WARN"'`.

## Testing

- `cd server/tests && ./smoke.sh` with a freshly started server and no other
  client: 43 checks covering every command, errors, items, quests, groups,
  control characters, several commands per packet, CRLF and long lines.
- Manual multi-client tests with `nc localhost 4242` in several terminals.
