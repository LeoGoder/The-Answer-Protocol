/**
 * @file models.hpp
 * @brief Common data structures, models, and enumerations for the TAP TUI client.
 */

#pragma once

#include <map>
#include <string>
#include <vector>

/**
 * @brief Connection parameters for TCP socket establishment.
 */
struct ConnectionConfig {
    std::string pseudo;         ///< Player username / handle.
    std::string ip   = "127.0.0.1"; ///< Server IPv4 address.
    std::string port = "4242";      ///< Server TCP listening port.
};

/**
 * @brief Structured chat or terminal message log entry.
 */
struct ChatMessage {
    std::string pseudo;         ///< Sender username (empty for system/server notices).
    std::string content;        ///< Message body.
    std::string channel;        ///< Target channel (e.g., "ROOM", "GLOBAL", "PARTY").
    bool is_local = false;      ///< True if sent by the local client.
};

/**
 * @brief Room state information received from LOOK responses.
 */
struct RoomInfo {
    std::string id;             ///< Canonical room identifier (e.g. "tavern.entrance").
    std::string name;           ///< Display name of the room.
    std::string description;    ///< Descriptive narrative of the room.
    std::map<std::string, std::string> exits; ///< Cardinal exits mapping direction to target room ID.
    std::vector<std::string> players;         ///< List of other players present in the room.
    std::vector<std::string> items;           ///< List of items on the floor.
    std::vector<std::string> npcs;            ///< List of non-player characters in the room.
};

/**
 * @brief Quest tracking details.
 */
struct QuestInfo {
    std::string quest_id;       ///< Unique identifier of the quest.
    std::string description;    ///< Full quest narrative.
    std::string reward;         ///< Reward description.
    std::string status;         ///< Status (e.g., "in_progress", "completed").
    std::string progress;       ///< Formatted progress status.
    std::string objective;      ///< Extracted primary objective.
    int required_items = 0;     ///< Quantity of items needed to complete.
    std::string target_item;    ///< Item name required by objective.
    std::string target_npc;     ///< Target NPC recipient for turn-in.
};

/**
 * @brief Result data returned after using an item.
 */
struct UseInfo {
    std::string used;           ///< Name of the item consumed.
    int hp = 0;                 ///< Current hit points after use.
    int max_hp = 0;             ///< Maximum hit points after use.
    int attack = 0;             ///< Current attack stat after use.
    std::string received;       ///< Additional item gained from use.
};

/**
 * @brief Combat action details returned from ATTACK or EVT ROOM COMBAT.
 */
struct CombatInfo {
    std::string attacker;       ///< Entity initiating the attack.
    std::string target;         ///< Entity taking the attack.
    int attacker_hp = 0;        ///< Attacker current hit points.
    int target_hp = 0;          ///< Target remaining hit points.
    int damage = 0;             ///< Damage dealt by attacker.
    int damage_taken = 0;       ///< Retaliation damage suffered.
    std::string status;         ///< Status (e.g., "in_combat", "defeated", "victory").
    std::string room;           ///< Respawn room or location.
};

/**
 * @brief Command enumeration tracking server response expectations.
 */
enum PendingCommand {
    CMD_NONE,                   ///< No awaiting synchronous response.
    CMD_LOOK,                   ///< Expecting room JSON.
    CMD_QUEST,                  ///< Expecting single quest JSON.
    CMD_QUESTS,                 ///< Expecting quest list JSON.
    CMD_INVENTORY,              ///< Expecting explicit inventory query JSON.
    CMD_INVENTORY_AUTO,         ///< Expecting automatic inventory refresh JSON.
    CMD_TAKE,                   ///< Take command awaiting confirmation.
    CMD_DROP,                   ///< Drop command awaiting confirmation.
    CMD_MOVE,                   ///< Movement command awaiting response.
    CMD_OTHER_JSON,             ///< Generic command expecting arbitrary JSON.
    CMD_USE,                    ///< Item usage awaiting response.
    CMD_ATTACK,                 ///< Combat command awaiting response.
    CMD_QUIT                    ///< Quit command awaiting OK bye response.
};

/**
 * @brief In-flight chat message record awaiting server OK acknowledgment.
 */
struct PendingChat {
    std::string pseudo;         ///< Author username.
    std::string content;        ///< Message body.
    std::string channel;        ///< Target channel.
    std::string raw_command;    ///< Original user command.
};

/**
 * @brief Normalized result of parsing user chat input.
 */
struct ParsedChat {
    bool is_chat = false;       ///< True if input represents a chat action.
    std::string channel;        ///< Canonical channel ("ROOM", "GLOBAL", "PARTY").
    std::string message;        ///< Message text without prefix.
    std::string command_to_send;///< Protocol-formatted line for socket transmission.
};
