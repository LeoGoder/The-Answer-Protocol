#pragma once

#include <string>

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

ftxui::Element shrinkable(ftxui::Element child);

ftxui::Element wrapLine(const std::string& pseudo,
                        const std::string& content,
                        ftxui::Color pseudo_color,
                        const std::string& channel = "");
