/**
 * @file ui_helpers.hpp
 * @brief Layout compression utilities and formatted chat line rendering.
 */

#pragma once

#include <string>

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

// Wraps an element into a ShrinkableNode to allow window shrinking horizontally
ftxui::Element shrinkable(ftxui::Element child);

/**
 * @brief Wraps an element into a ShrinkableYNode to clamp min_y to 1 and enable flex_shrink_y.
 *
 * Prevents scrollable containers (like chat logs or message lists) from expanding
 * their layout requirement as items are added, ensuring sibling panels (like the map)
 * are not crushed by message spam while allowing responsive terminal window resizing.
 *
 * @param child Child element (typically a yframe element).
 * @return Wrapped FTXUI Element with min_y = 1 and flex_shrink_y / flex_grow_y enabled.
 */
ftxui::Element shrinkableY(ftxui::Element child);

// String trimming and prefix manipulation utilities
std::string trim(const std::string& str);
std::string toUpper(std::string str);
bool startsWith(const std::string& str, const std::string& prefix);
bool stripPrefix(const std::string& str, const std::string& prefix,
                 std::string& out);

/**
 * @brief Constructs an auto-wrapping paragraph element with optional sender pseudo and channel badge.
 *
 * Formats a message line with syntax highlighting:
 * - Empty pseudo: Plain message body (server notices).
 * - Pseudo with channel: "[Pseudo] (Channel): Content"
 * - Pseudo alone: "[Pseudo]: Content"
 * Automatically wraps long text lines to fit the available container width.
 *
 * @param pseudo Sender username or identifier.
 * @param content Message payload text.
 * @param pseudo_color Color styling applied to the sender badge.
 * @param channel Optional chat channel name (e.g., "ROOM", "GLOBAL").
 * @return FTXUI Element ready for rendering in a vbox list.
 */
ftxui::Element wrapLine(const std::string& pseudo,
                        const std::string& content,
                        ftxui::Color pseudo_color,
                        const std::string& channel = "");
