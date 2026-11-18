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
    /// Player username / handle.
    std::string pseudo;
    /// Server IPv4 address.
    std::string ip   = "127.0.0.1";
    /// Server TCP listening port.
    std::string port = "4242";
};

/**
 * @brief Structured chat or terminal message log entry.
 */
struct ChatMessage {
    /// Sender username (empty for system/server notices).
    std::string pseudo;
    /// Message body.
    std::string content;
    /// Target channel (e.g., "ROOM", "GLOBAL", "PARTY").
    std::string channel;
    /// True if sent by the local client.
    bool is_local = false;
};

/**
 * @brief Room state information received from LOOK responses.
 */
struct RoomInfo {
    /// Canonical room identifier (e.g. "tavern.entrance").
    std::string id;
    /// Display name of the room.
    std::string name;
    /// Descriptive narrative of the room.
    std::string description;
    /// Cardinal exits mapping direction to target room ID.
    std::map<std::string, std::string> exits;
    /// List of other players present in the room.
    std::vector<std::string> players;
    /// List of items on the floor.
    std::vector<std::string> items;
    /// List of non-player characters in the room.
    std::vector<std::string> npcs;
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
    /// Status (e.g., "in_combat", "defeated", "victory").
    std::string status;
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
    CMD_ATTACK                  ///< Combat command awaiting response.
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
    /// True if input represents a chat action.
    bool is_chat = false;
    /// Canonical channel ("ROOM", "GLOBAL", "PARTY").
    std::string channel;
    /// Message text without prefix.
    std::string message;
    /// Protocol-formatted line for socket transmission.
    std::string command_to_send;
};
