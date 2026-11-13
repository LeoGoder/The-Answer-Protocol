/**
 * @file ui_helpers.hpp
 * @brief Layout compression utilities and formatted chat line rendering.
 */

#pragma once

#include <string>

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

// Wraps an element into a ShrinkableNode to allow terminal window shrinking without layout overflow
ftxui::Element shrinkable(ftxui::Element child);

// String trimming and prefix manipulation utilities
std::string trim(const std::string& str);
std::string toUpper(std::string str);
bool startsWith(const std::string& str, const std::string& prefix);
bool stripPrefix(const std::string& str, const std::string& prefix, std::string& out);

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
