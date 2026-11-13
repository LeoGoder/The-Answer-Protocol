/**
 * @file image_renderer.hpp
 * @brief Terminal image rendering utilities using Unicode half-blocks and 24-bit TrueColor.
 */

#pragma once

#include <string>

#include <ftxui/dom/elements.hpp>

/**
 * @brief Renders an image file to an FTXUI Element using half-block Unicode characters.
 *
 * Loads and caches the image from disk via stb_image, rescales it with an area-averaging
 * box filter to fit the bounding limits, and renders two vertical pixels per character
 * cell using the Unicode upper half-block ('▀') with foreground and background ANSI TrueColor.
 *
 * @param path File system path to the image asset (PNG, JPG, etc.).
 * @param max_width Maximum allowable character cell width (0 for unconstrained).
 * @param max_height Maximum allowable character cell height (0 for unconstrained).
 * @param keep_ratio If true, scales dimensions uniformly preserving aspect ratio.
 * @return FTXUI Element representing the rendered image DOM node or error text.
 */
ftxui::Element renderImage(const std::string& path,
                           int max_width = 0,
                           int max_height = 0,
                           bool keep_ratio = false);

// Legacy alias forwarding directly to renderImage
inline ftxui::Element put_image(const std::string& path,
                                int max_width = 0,
                                int max_height = 0,
                                bool keep_ratio = false) {
    return renderImage(path, max_width, max_height, keep_ratio);
}
