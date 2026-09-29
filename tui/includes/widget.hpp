#pragma once

#include <string>

#include <ftxui/dom/elements.hpp>

ftxui::Element createWidget(int x, int y, int width, int height,
                            ftxui::Element content = nullptr,
                            const std::string& title = " Image ");

ftxui::Element createProportionalWidget(float ratio_x, float ratio_y,
                                        float ratio_width,
                                        float ratio_height,
                                        ftxui::Element content = nullptr,
                                        const std::string& title = " Image ");

ftxui::Element createWidgetImage(const std::string& image_path, int x, int y,
                                 int width, int height,
                                 const std::string& title = " Image ");

ftxui::Element createProportionalWidgetImage(const std::string& image_path,
                                             float ratio_x, float ratio_y,
                                             float ratio_width,
                                             float ratio_height,
                                             const std::string& title = " Image ");

void changeWidget(ftxui::Element widget, ftxui::Element new_content,
                  const std::string& title = " Image ");

void changeProportionalWidget(ftxui::Element widget,
                              ftxui::Element new_content,
                              const std::string& title = " Image ");
