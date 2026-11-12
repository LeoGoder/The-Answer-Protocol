/**
 * @file positioned_widget.hpp
 * @brief Floating and proportionally positioned overlay widgets for FTXUI.
 */

#pragma once

#include <string>

#include <ftxui/dom/elements.hpp>

/**
 * @brief Creates a floating overlay widget positioned at absolute terminal coordinates.
 *
 * Wraps content in a PositionedWidgetNode that places the child at specific (x, y)
 * offsets with explicit width and height. Supports negative coordinates to anchor
 * from the right or bottom edges of the screen.
 *
 * @param x Column offset (positive from left, negative from right).
 * @param y Row offset (positive from top, negative from bottom).
 * @param width Bounding width in character cells.
 * @param height Bounding height in character cells.
 * @param content Inner FTXUI element to host (defaults to placeholder image).
 * @param title Border title string.
 * @return FTXUI Element representing the positioned widget.
 */
ftxui::Element createWidget(int x, int y, int width, int height,
                            ftxui::Element content = nullptr,
                            const std::string& title = " Image ");

/**
 * @brief Creates a floating overlay widget positioned proportionally relative to parent dimensions.
 *
 * The widget recalculates its size and position on each screen resize based on float ratios
 * between 0.0 and 1.0. Supports negative ratios to anchor relative to opposite edges.
 *
 * @param ratio_x Horizontal offset ratio (0.0 = left edge).
 * @param ratio_y Vertical offset ratio (0.0 = top edge).
 * @param ratio_width Width ratio relative to parent container width.
 * @param ratio_height Height ratio relative to parent container height.
 * @param content Inner FTXUI element to host.
 * @param title Border title string.
 * @return FTXUI Element representing the proportional widget.
 */
ftxui::Element createProportionalWidget(float ratio_x, float ratio_y,
                                        float ratio_width,
                                        float ratio_height,
                                        ftxui::Element content = nullptr,
                                        const std::string& title = " Image ");

// Creates an absolute positioned floating widget displaying an image asset
ftxui::Element createWidgetImage(const std::string& image_path, int x, int y,
                                 int width, int height,
                                 const std::string& title = " Image ");

// Creates a proportional positioned floating widget displaying an image asset
ftxui::Element createProportionalWidgetImage(
    const std::string& image_path,
    float ratio_x, float ratio_y,
    float ratio_width,
    float ratio_height,
    const std::string& title = " Image ");

// Dynamically replaces the child content and border title of an existing widget
void changeWidget(ftxui::Element widget, ftxui::Element new_content,
                  const std::string& title = " Image ");

// Dynamically replaces the child content and border title of a proportional widget
void changeProportionalWidget(ftxui::Element widget,
                              ftxui::Element new_content,
                              const std::string& title = " Image ");
