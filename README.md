_This project has been created as part of the 42 curriculum by opernod, lgoderne, zqian._

# TAP — The Answer Protocol

## Description

**TAP** is a small multiplayer text adventure (a MUD, *Multi-User Dungeon*). Several
players connect to the same server, explore a shared world, chat, form groups, fight
enemies and complete quests together, in real time.

The project has three parts that talk over **TCP**, with one UTF-8 message per line, as
defined by **RFC 42TAP** (the protocol given with the subject):

| Part | Language | Folder | Role |
|---|---|---|---|
| Server | Go | [`server/`](server/) | owns the whole world state, applies the rules, pushes events |
| CLI client | C++ / [FTXUI](https://github.com/ArthurSonzogni/FTXUI) | [`tui/`](tui/) | terminal interface |
| GUI client | C++ / [raylib](https://www.raylib.com/) | [`gui/`](gui/) | graphical top-down interface |

The world is **the Culinary Archipelago**: eight islands around a restaurant. Chef Gusto
needs ingredients from the islands to cook his Beef Wellington; the players collect them,
defeat the creatures that guard them, and finally face the Supreme Critic.

More detail on each part:

- Server internals: [`server/README.md`](server/README.md)
- GUI client (controls, screens, rendering, networking): [`gui/README.md`](gui/README.md)
- CLI client: [`tui/README.md`](tui/README.md)

## Instructions

Quick start, on Linux (42 machines):

```bash
make install        # Go modules + FTXUI for the CLI client
make run-server     # terminal 1: server on port 4242
make run-client     # terminal 2: CLI client
make run-client-gui # terminal 3: GUI client
```

Any TCP client also works: `nc localhost 4242`, then type commands (`CONNECT alice`,
`LOOK`, `MOVE north`, …). The full list of targets, requirements and options is in
[Building and Running](#building-and-running).

## Architecture

### Server structure

```
server/
  cmd/server/
    main.go        accept loop, connection limit, the two ticker goroutines
    client.go      one goroutine per connection: read lines, CONNECT / QUIT, authentication
    hub.go         username → connection map, sends events outside the game lock
    monitor.go     abuse detection (command flooding, rapid connections)
  internal/protocol/   framing checks, parser, argument validation, username rules, reply constants
  internal/game/       world loading and validation, game state and every command
  internal/logger/     structured JSON logger
  data/world.json      the world
```

### Dispatcher

The server uses a **dispatcher**: every line goes through the same checks, from the
outside in, before any game code runs.

```
TCP line ─► length ≤ 1024 bytes ─► valid UTF-8, no control character ─► Parse (name + args)
        ─► Validate (argument count, CHAT scope, GROUP action, username rules)
        ─► CONNECT / QUIT handled by the connection, other commands need to be authenticated
        ─► State.Handle: one switch that calls the function of each command
```

`protocol/` knows nothing about the game, and `game/` never touches the network: it
returns a `Result` (the reply for the sender) plus a list of `Event`s (a line and its
recipients). This keeps every layer small and testable on its own.

### Concurrency model

- **One goroutine per connection** reads its client and handles its commands.
- **Two ticker goroutines**: `Tick` once per second (dropped items expire, spawn points
  refill, enemies respawn) and `PositionEvents` 24 times per second.
- **One mutex protects the whole game state.** Exported methods (`Handle`, `Join`,
  `Leave`, `Tick`, `PositionEvents`) take it; unexported ones expect it held and never
  call an exported one (Go mutexes are not reentrant). A single lock cannot deadlock on
  lock order, and a command holds it for microseconds only.
- **Sending happens after the lock is released.** The goroutine that caused an event
  writes it to the recipients' connections through the hub, with a 2-second write
  deadline. A slow or dead client only delays that one goroutine, never the game; a
  failed send is logged and delivery goes on to the other recipients.
- The number of open connections is an atomic counter (limit 64).

### Clients

Both clients stay responsive while events arrive:

- **CLI**: a background thread listens to the socket while FTXUI handles the input. The
  user types RFC commands directly (`LOOK`, `TAKE Beef`, `CHAT ROOM hi`…); the client
  turns the JSON replies into readable text, and adds a few local shortcuts (`CLEAR`,
  `CHAT PARTY` as an alias of `CHAT GROUP`). Details: [`tui/README.md`](tui/README.md).
- **GUI**: single thread, non-blocking socket polled once per frame, only complete lines
  are handled. → **Described in [`gui/README.md`](gui/README.md#architecture).**

## Protocol Implementation

The server implements **every command, event and error of RFC 42TAP**. Everything below
is either a detail the RFC leaves open, or an **addition**. Additions never change the
format of an existing reply: they only add commands, events, JSON fields or error
codes, so a client written only from the RFC still works with this server.

### Choices where the RFC is open

- **Framing**: lines end with `\n` (a trailing `\r` is accepted). Lines are limited to
  **1024 bytes** (RFC 9.4); a longer line is read to its end and answered
  `ERR 400 LINE_TOO_LONG`, and the connection stays usable.
- **Encoding**: invalid UTF-8 and control characters (except tab) are refused with
  `ERR 400 BAD_REQUEST` (RFC 9.2). Unicode usernames and messages are accepted.
- **Commands** are case-insensitive (RFC 4.2); so are chat scopes, group actions and
  directions. An empty line is ignored.
- **Usernames**: 1 to 32 Unicode letters, digits or `_`, at most 3 identical characters
  in a row, and no `admin` in any case. Otherwise `ERR 400 BAD_REQUEST`.
- **Items and NPCs** can be named by ID (`beef_wellington`) or display name
  (`Beef Wellington`), in any case, with any spacing (RFC 8.3, 8.4).
- **IDs** are plain words (`beef`, `chef`, `hub`); the RFC's `item.` / `npc.` prefixes are
  only examples.
- **WHO** follows the RFC: `OK players=<count>`.
- **`CONNECT` twice** on the same connection → `ERR 400 BAD_REQUEST`; any other command
  before `CONNECT` → `ERR 202 NOT_AUTHENTICATED`.
- **`ERR 901 SEND_FAILED`** is never sent: when a send fails, the connection is broken
  and nothing can reach the client any more. The failure is logged (`send_failed`).
- **QUEST** returns `"status": "active"` instead of the RFC example's `"available"`: asking
  an NPC for a quest **accepts** it at once (see [Quest System](#quest-system)).

### Added commands

| Command | Reply | Why |
|---|---|---|
| `USE <item>` | `OK {"used":…, "hp":…, "max_hp":…}` | eat food to heal, open the chest; costs a combat turn |
| `WORLD` | `OK {"hostile":[…], "quest_givers":[…], "usable":[…]}` | static data so a client shows only the buttons that make sense |
| `POSITION <x> <y>` | `OK` | position inside the room (0–100000), for the GUI's real-time movement |
| `ALIVE` | `OK Alive` | keep-alive: lets a client detect a dead connection |

### Added events

| Event | Sent to | When |
|---|---|---|
| `EVT ROOM ITEM TAKE <player> <item>` / `DROP` | the room | a player takes / drops an item |
| `EVT ROOM ITEM SPAWN <item>` | the room | a spawn point refills |
| `EVT ROOM ITEM GONE <item>` | the room | a dropped item expires, or is handed over as a reward |
| `EVT ROOM COMBAT <json>` | the room | one combat round (same JSON as the ATTACK reply) |
| `EVT ROOM NPC SPAWN <npc>` | the room | a defeated enemy comes back |
| `EVT ROOM POSITION {"name":[x,y],…}` | players who sent `POSITION` | 24 times per second |
| `EVT GROUP MEMBERS a,b,c` | the group | after each change, leader first (a member cannot otherwise know who joined before them) |

### Added JSON fields

- `LOOK`: `names` maps each ID shown (items on the floor and carried, NPCs, exits) to its
  display name.
- `ATTACK`: `attacker`, `target`, `damage_taken`, and `room` when the player was defeated.
- `STATUS`: `attack`, `defense`, and `target` during a fight; `status` is `healthy`,
  `wounded` or `combat`.
- `QUEST`: `progress` (`0/1` or `1/1`).

### Added error codes

| Error | When |
|---|---|
| `ERR 202 NOT_AUTHENTICATED` | a command other than `CONNECT` / `QUIT` before `CONNECT` |
| `ERR 400 BAD_REQUEST` | wrong arguments, invalid username, invalid UTF-8 or control character |
| `ERR 400 UNKNOWN_COMMAND` | unknown command |
| `ERR 400 LINE_TOO_LONG` | line longer than 1024 bytes |
| `ERR 403 NOT_INVITED` | `GROUP JOIN` without an invitation |
| `ERR 404 PLAYER_NOT_FOUND` | `GROUP INVITE` / `JOIN` with a player who is not online |
| `ERR 408 ITEM_NOT_USABLE` | `USE` on an item that does nothing |
| `ERR 409 ALREADY_HAVE_ITEM` | `TAKE` an item the player already carries |
| `ERR 500 INTERNAL_ERROR` | a reply could not be encoded (should never happen) |
| `ERR 900 CONNECTION_FAILED` | the server already has 64 connections (RFC code) |

## Combat System

Combat is **turn-based and driven by the player**: one `ATTACK` is one round. Enemies
never strike on their own, so a player is never hit while reading or typing.

**Initiative order**: the player always strikes first. If the enemy reaches 0 HP it does
not hit back; otherwise it counter-attacks in the same round.

**Damage formula** (same for both sides):

```
damage = attacker.attack + random(0..4) − defender.defense      (at least 1)
```

The random part makes fights less predictable; the minimum of 1 means every fight ends.

| | HP | Attack | Defense | Drops |
|---|---|---|---|---|
| Player (start) | 100 | 10 | 2 | |
| The Queen Of Cows | 42 | 12 | 4 | |
| Wild Guardian | 60 | 12 | 1 | Mushroom |
| Supreme Critic | 110 | 18 | 2 | Chest |

Quest rewards raise the player's max HP and attack (see [Quest System](#quest-system)),
so the enemies are meant to be fought in that order.

**Combat state**

- The first `ATTACK` on an enemy starts the fight: `STATUS` then reports
  `"status": "combat"` and the target.
- **Fleeing** is `MOVE`: leaving the room ends the fight, no extra command is needed.
- **Healing** is `USE <item>` (Egg +25, Beef Wellington +100, up to max HP). During a fight
  it costs the turn: the enemy hits back, so healing cannot make a fight endless.
- We did not add `DEFEND`: with `MOVE` and `USE` a player can already escape or recover,
  and every new command would also have to be added to both clients.

**Several players, one enemy**: enemy HP is shared, so players can fight the same enemy
together. When it falls, the killer **and the members of their group in the same room**
get the quest credit and the drop.

**Defeat and respawn**

- A player at 0 HP is defeated: their HP is set to **half of their max HP**, the fight
  ends, and they are moved to the **Hub** (the respawn room, where no enemy is allowed).
  The ATTACK reply has `"status": "defeated"` and `"room": "hub"`; the two rooms get the
  usual `PRESENCE LEAVE` / `ENTER` events.
- A defeated enemy disappears from `LOOK` and comes back with full HP after **30
  seconds** (`EVT ROOM NPC SPAWN`).

**Broadcast and logs**: every round is sent to the other players of the room as
`EVT ROOM COMBAT <json>` and logged (`combat_started`, `combat_round`, `npc_defeated`,
`player_defeated`, `combat_left`).

## Quest System

**One command does everything: `QUEST <npc>`.**

1. If the player has an **active quest from this NPC**: when the objective is done the
   quest is **completed** and rewarded, otherwise the quest and its progress are shown.
2. Otherwise the NPC **gives its next quest** (in the order of `world.json`), which is
   accepted at once.
3. Otherwise: `ERR 406 NO_QUEST_AVAILABLE`.

We chose this over separate `ACCEPT` / `COMPLETE` commands: the RFC already defines
`QUEST`, every client supports it, and "talk to the NPC again to hand the quest in" is the
usual MUD flow.

**Quest types and completion validation**

| Type | Objective | Validated when |
|---|---|---|
| `fetch` | bring an item to the giver | the player carries the item when asking `QUEST`; the item is then taken (`delivered`) |
| `defeat` | defeat an enemy | the enemy is defeated **after** the quest was accepted, by the player or a member of their group in the same room |

Progress is `0/1` or `1/1` (`QUEST`, `QUESTS`). Quests are per player and, like the rest of
the state, are lost on disconnect (no persistence is required).

**Rewards** (on completion):

- **Max HP** is raised, and current HP by the same amount;
- **Attack** is raised;
- an optional **item** goes to the inventory. If the player already has one, nothing is
  given; if the item already exists as many times as there are players online, an
  instance lying on the ground is handed over instead of creating a new one (the room is
  told with `EVT ROOM ITEM GONE`).

**Progression**: Chef Gusto gives his quests one after the other, which makes a story:

| # | Quest | Giver | Objective | Reward |
|---|---|---|---|---|
| – | `defeat_cow_queen` | Butcher | defeat The Queen Of Cows | +15 HP, +3 attack, **Beef** |
| 1 | `fetch_beef` | Chef Gusto | bring Beef | +10 HP, +2 attack |
| 2 | `fetch_egg` | Chef Gusto | bring Egg | +15 HP, +3 attack |
| 3 | `defeat_guardian` | Chef Gusto | defeat the Wild Guardian | +20 HP, +4 attack |
| 4 | `fetch_mushroom` | Chef Gusto | bring Mushroom | +20 HP, +4 attack |
| 5 | `craft_wellington` | Chef Gusto | bring Flour | +40 HP, +8 attack, **Beef Wellington** |
| 6 | `defeat_critic` | Chef Gusto | defeat the Supreme Critic | +30 HP, +5 attack |

The Supreme Critic drops the **Chest**; using it gives the **Chef Hat**.

All quests are checked when the world is loaded: the giver exists, a fetched item can be
found in a room or is dropped by an enemy, a defeat target is an enemy, a reward item
exists.

## World Design

The world is in [`server/data/world.json`](server/data/world.json). The server checks it
at start and **refuses to start** if anything is wrong (unknown exit or reference,
duplicate ID or name, invalid direction, enemy without HP, enemy in the respawn room…).

### Story

The players work for the restaurant of the **Hub**, the heart of a small archipelago.
**Chef Gusto** wants to cook his signature dish, the legendary **Beef Wellington**, and
sends them to the neighbouring islands for the ingredients:

- **Flour** is already in the restaurant;
- prime **Beef** comes from **Jeetomal**, where **The Queen Of Cows** has seized the
  pastures. The **Butcher** lost his job because of her and asks the players to defeat
  her; his reward is the beef;
- fresh **Eggs** are gathered on **Fejka**, in the north-west;
- wild **Mushrooms** grow in the forest of **Kuggis**, guarded by the **Wild Guardian**.

With all the ingredients, Chef Gusto prepares the Beef Wellington. Eating it restores the
strength needed for the last fight: on the sanctuary of **Fugoumitak**, the **Supreme
Critic** guards an ancient chest and only accepts challengers "worthy of culinary
greatness". The chest holds the **Chef's Hat**, symbol of absolute culinary mastery.

The Apprentice Waiter of the Hub gives the first hints: talk to Chef Gusto, and never
challenge the Critic without a Beef Wellington.

### Layout

Eight islands (rooms), joined by bridges:

```
   Fejka ────── Smycka ──────┐
     │            │          │
 Vinterfint ── Jeetomal ── Kuggis
     │            │
 Komolexos ───── Hub ────── Fugoumitak
```

- **Loops**: Komolexos → Hub → Jeetomal → Vinterfint → Komolexos;
  Vinterfint → Jeetomal → Smycka → Fejka → Vinterfint; Jeetomal → Kuggis → Smycka → Jeetomal.
  Every island can be visited in one full circuit.
- **Optional branch**: **Fugoumitak**, a dead end east of the Hub, home of the final boss.
- **Hub** is the start room and the respawn room.

### NPC roles

| NPC | Island | Role |
|---|---|---|
| Apprentice Waiter | Hub | dialogue only |
| Chef Gusto | Hub | quest giver (main story) |
| Butcher | Jeetomal | quest giver |
| The Queen Of Cows | Jeetomal | enemy |
| Wild Guardian | Kuggis | enemy (drops Mushroom) |
| Supreme Critic | Fugoumitak | final boss (drops Chest) |

`TALK` returns an NPC's dialogue lines one after the other, in a loop.

### Items

| Item | Obtained | Use |
|---|---|---|
| Flour | lies in the Hub | quest `craft_wellington` |
| Beef | lies in Jeetomal; reward of the Butcher | quest `fetch_beef` |
| Egg | lies in Fejka | quest `fetch_egg`; heals 25 |
| Mushroom | dropped by the Wild Guardian | quest `fetch_mushroom` |
| Beef Wellington | reward of `craft_wellington` | heals 100, for the final fight |
| Chest | dropped by the Supreme Critic | `USE` opens it: heals 25 and gives the Chef Hat |
| Chef Hat | inside the Chest | trophy |

**Item rules** (unique instances, RFC 8):

- Each item is a **unique instance** with its own serial number (visible in the logs),
  in exactly one place: a room floor or one inventory. `TAKE` moves it from the room,
  `DROP` puts it back for the others: nothing is duplicated.
- A player carries **at most one item of each kind** (`ERR 409 ALREADY_HAVE_ITEM`).
- No new instance is created when an item already exists as many times as there are
  players online.
- An item placed in a room by `world.json` is a **spawn point**: 30 s after it is taken,
  a new one appears (`EVT ROOM ITEM SPAWN`), within the limit above.
- A **dropped** item disappears after 30 s (`EVT ROOM ITEM GONE`), so the floors do not
  fill up.
- When a player disconnects, what they carry disappears with them.

## Server Logging

### Format and destinations

One **JSON object per line**, written by `server/internal/logger`:

```json
{"time":"2026-11-18T10:18:25.070054021Z","level":"INFO","event":"item_taken","item":"beef","player":"alice","room":"jeetomal","serial":0}
```

- `time`: UTC, RFC 3339 with nanoseconds; then `level` and `event`; then the other fields
  sorted by name. Every value is JSON-escaped, so a message cannot break a line or forge a
  field.
- **`INFO` and `WARN` go to stdout, `ERROR` to stderr.**
- On a terminal, lines are coloured (errors red, received commands yellow, `OK` green,
  events dim). When the output is a file or a pipe, no colour is added and the file
  stays valid JSON.
- **Performance**: each line is built in a local buffer; only the final write is under a
  mutex, so goroutines never mix their lines and hardly wait for each other.

### Event types

| Category | Events |
|---|---|
| Server | `server_starting`, `world_loaded`, `server_listening`, `world_load_failed`, `listen_failed`, `accept_failed` |
| Connections (with IP) | `client_connected`, `player_authenticated`, `connect_rejected`, `connection_refused`, `connection_closed`, `player_disconnected`, `client_disconnected` |
| Commands | `command_received` (player, IP, command, arguments), `invalid_line`, `invalid_command`, `line_too_long`, `unauthenticated_command` |
| Replies and events sent | `message_sent` (every `OK`, `ERR` and `EVT` line, with player and IP), `send_failed` |
| Items | `item_created`, `item_taken`, `item_dropped`, `item_used`, `item_received`, `item_deleted`, `items_removed_on_disconnect`, `item_grant_skipped`, `item_grant_failed` |
| World and NPCs | `player_moved`, `npc_talk`, `npc_respawned`, `chat` |
| Combat | `combat_started`, `combat_round`, `combat_left`, `npc_defeated`, `player_defeated` |
| Quests | `quest_accepted`, `quest_progress`, `quest_completed` |
| Groups | `group_created`, `group_invite`, `group_joined`, `group_left`, `group_disbanded` |
| Abuse | `command_flood`, `rapid_connections` (`WARN`) |

### Abuse detection

- **Command flooding**: more than **60 commands in 1 second** on one connection →
  `command_flood`, once per window.
- **Rapid connections**: more than **5 connections from the same IP in 10 seconds**
  (sliding window) → `rapid_connections`.
- **Connection limit**: beyond **64** open connections, a new one gets
  `ERR 900 CONNECTION_FAILED` and is logged as `connection_refused`.

These patterns are logged, not blocked, so a legitimate fast client (the GUI sends
`POSITION` 24 times per second) is never cut off.

### Monitoring

```bash
make run-server                                    # everything on the terminal
make run-server-quiet                              # full log in server/server.log, screen without POSITION / ALIVE
grep '"level":"WARN"' server/server.log            # warnings only
grep -E 'command_flood|rapid_connections' server/server.log
grep '"player":"alice"' server/server.log          # everything about one player
grep '"item":"beef"' server/server.log             # the life of every beef instance
jq 'select(.event=="combat_round")' server/server.log   # if jq is installed
```

## Group Contributions

| Member | Responsibilities |
|---|---|
| **zqian** | Go server: protocol layer, game logic (items, combat, quests, groups, positions), world loading and validation, logging and abuse monitoring, protocol extensions; root Makefile |
| **lgoderne** | GUI client (raylib): scenes, HUD, chat and log panels, dialogs, minimap, save slots, CRT shader; island maps (Tiled) and sprites |
| **opernod** | CLI client (FTXUI): login view, game view, network listener thread, reply formatting; TUI Makefile |

The world story, the islands and the quests were designed together.

## Building and Running

### Requirements

| Part | Needs |
|---|---|
| Server | Go **1.18** or later |
| CLI client | `g++` (C++17), `make`, `cmake`; FTXUI is vendored in `tui/lib/FTXUI/` and built by `make install` |
| GUI client | `g++`, `make`, X11 and OpenGL headers; raylib is vendored in `gui/raylib/` |
| `make lint` | `gofmt` / `go vet` (with Go), `cpplint` |

### Root Makefile

| Target | Effect |
|---|---|
| `make install` | prepare the Go modules (`go mod tidy`), build FTXUI for the CLI client |
| `make run-server` | start the server (`go run ./cmd/server`) on port 4242 |
| `make run-server-quiet` | same, full log saved to `server/server.log`, screen without the `POSITION` / `ALIVE` traffic |
| `make run-client` | build and start the CLI client |
| `make run-client-gui` | build and start the GUI client |
| `make run-client-gui-quiet` | same, without the `POSITION` / `ALIVE` lines |
| `make lint` | `gofmt` + `go vet` on the server, `cpplint` on both clients |
| `make clean` | remove build outputs of the three parts and `server/server.log` |

### Server options

```bash
cd server && go run ./cmd/server -port 4242 -world data/world.json
```

### Clients

- **CLI**: the login screen asks for the server IP, the port and the player name.
- **GUI**: the server address is `SERVER_IP` in `gui/inc/game_world.hpp`
  (`127.0.0.1` by default); the name is typed in the *New game* menu.
  → **Controls and details in [`gui/README.md`](gui/README.md#build-and-run).**

To play from several computers, start the server on one of them and point the clients to
its IP address (port 4242 must be reachable on the local network).

## Testing

Start the server with `make run-server` and watch its log while testing. `nc` is enough
to test the protocol by hand; the clients can be mixed freely, since they all speak the
same protocol.

### Multiplayer

1. Open three clients (for example the GUI, the CLI and `nc localhost 4242`) and connect
   as `alice`, `bob`, `carol`. Each one receives `EVT STATS players=<n>`.
2. `CONNECT alice` from a fourth client → `ERR 201 NAME_IN_USE`.
3. `alice`: `MOVE north` → Hub players get `PRESENCE LEAVE alice`, Jeetomal players get
   `PRESENCE ENTER alice`.
4. Chat: `CHAT GLOBAL hi` (everyone), `CHAT ROOM hi` (same island only).
5. Groups: `alice` `GROUP CREATE`, `GROUP INVITE bob`; `bob` gets `EVT GROUP INVITE alice`
   and sends `GROUP JOIN alice`; `CHAT GROUP hi` reaches the two of them only;
   `GROUP JOIN alice` from `carol` without an invitation → `ERR 403 NOT_INVITED`.
6. Items: `alice` `TAKE Beef` in Jeetomal → `bob`, in the same room, gets
   `EVT ROOM ITEM TAKE alice beef` and `LOOK` no longer lists it; `DROP beef` puts it back
   (`EVT ROOM ITEM DROP alice beef`). If a second Beef lies there (dropped by `bob`, or
   respawned after 30 s), `TAKE beef` while carrying one → `ERR 409 ALREADY_HAVE_ITEM`.
7. Kill a client (`Ctrl+C`): the others get `PRESENCE LEAVE` and the new
   `EVT STATS players=<n>`, the server keeps running, and the items it carried are gone.

### Combat

1. In Jeetomal: `ATTACK queen` several times. Each reply shows `damage`, `damage_taken`
   and both HP; the other players of the room get `EVT ROOM COMBAT`.
2. `STATUS` during the fight → `"status":"combat"` and `"target":"queen"`.
3. `MOVE south` leaves the fight; `STATUS` → `wounded`.
4. `ATTACK waiter` in the Hub → `ERR 405 NPC_NOT_HOSTILE`; `ATTACK nobody` →
   `ERR 404 NPC_NOT_FOUND`.
5. Defeat: go to Fugoumitak at the start of the game and `ATTACK critic` until the reply
   says `"status":"defeated"` → the player is back in the Hub with half of their max HP.
6. After defeating the Queen, `LOOK` no longer lists her; 30 s later the room gets
   `EVT ROOM NPC SPAWN queen`.
7. Group credit: `alice` and `bob` in the same group and in Jeetomal, both with the
   Butcher's quest; `alice` defeats the Queen → `QUESTS` shows `1/1` for both.

### Quests

```
MOVE north           (Jeetomal: the Butcher and the Queen are here)
QUEST butcher        → defeat_cow_queen, active, 0/1
ATTACK queen         (until "victory")
QUEST butcher        → completed: +15 HP, +3 attack, Beef in the inventory
MOVE south
QUEST chef           → fetch_beef, active, 1/1 (Beef is already carried)
QUEST chef           → completed, Beef delivered; INVENTORY no longer lists it
QUEST chef           → fetch_egg, active, 0/1
QUESTS               → defeat_cow_queen and fetch_beef completed, fetch_egg active
QUEST waiter         → ERR 406 NO_QUEST_AVAILABLE
```

`STATUS` after each completion shows the new max HP and attack. If the Queen wins, the
player is back in the Hub: `MOVE north` and attack again (the quest stays active).

### Robustness

```bash
# line longer than 1024 bytes → ERR 400 LINE_TOO_LONG, connection still usable
(head -c 2000 /dev/zero | tr '\0' a; echo; echo WHO) | nc -q 1 localhost 4242

# invalid UTF-8 → ERR 400 BAD_REQUEST
printf 'CONNECT dave\nCHAT GLOBAL \xff\n' | nc -q 1 localhost 4242

# command flood → WARN command_flood in the log
(echo CONNECT flood; yes WHO | head -n 200) | nc -q 1 localhost 4242 > /dev/null

# rapid connections → WARN rapid_connections in the log
for i in 1 2 3 4 5 6; do echo QUIT | nc -q 1 localhost 4242; done
```

Also try: a command before `CONNECT` (`ERR 202`), an unknown command (`ERR 400
UNKNOWN_COMMAND`), a broken `world.json` (the server refuses to start and says why).

## Resources

- RFC 42TAP — the protocol given with the subject
- [RFC 2119](https://www.rfc-editor.org/rfc/rfc2119) (MUST / SHOULD) and
  [RFC 5234](https://www.rfc-editor.org/rfc/rfc5234) (ABNF), referenced by RFC 42TAP
- [RFC 3629](https://www.rfc-editor.org/rfc/rfc3629) — UTF-8
- [A Tour of Go](https://go.dev/tour/), [Effective Go](https://go.dev/doc/effective_go),
  and the package docs of [`net`](https://pkg.go.dev/net), [`bufio`](https://pkg.go.dev/bufio),
  [`sync`](https://pkg.go.dev/sync), [`encoding/json`](https://pkg.go.dev/encoding/json)
- [Go blog — Share Memory By Communicating](https://go.dev/blog/codelab-share)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) — sockets
- [MUD — Wikipedia](https://en.wikipedia.org/wiki/Multi-user_dungeon) — history of the genre
- [raylib](https://www.raylib.com/) and [Tiled](https://doc.mapeditor.org/) — GUI client,
  see [`gui/README.md`](gui/README.md#resources)
- [FTXUI](https://arthursonzogni.github.io/FTXUI/) — CLI client

### AI usage

AI was used as an assistant, never as a replacement for understanding: everything it
produced was read, questioned, tested by hand and reviewed by another member before being
merged, and each of us can explain their own part.

- **Server (zqian)**: explaining Go concepts (goroutines, mutexes, `atomic`, `bufio`);
  reviewing the code and the pull requests against the RFC and the subject; finding edge
  cases (line framing, control characters, item duplication, lock order); helping write
  some small changes once the design was decided; fixing environment problems (Go 1.18
  on the school machines, line endings with Git on WSL); the Makefile; preparing test
  scenarios; drafting this README.
- **GUI client (lgoderne)**: TODO.
- **CLI client (opernod)**: TODO.
