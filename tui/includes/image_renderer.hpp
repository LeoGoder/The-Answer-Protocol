#pragma once

#include <string>

#include <ftxui/dom/elements.hpp>

ftxui::Element renderImage(const std::string& path,
                           int max_width = 0,
                           int max_height = 0,
                           bool keep_ratio = false);

inline ftxui::Element put_image(const std::string& path,
                                int max_width = 0,
                                int max_height = 0,
                                bool keep_ratio = false) {
    return renderImage(path, max_width, max_height, keep_ratio);
}
