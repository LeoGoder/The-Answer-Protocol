// UI layout helpers and text wrapping

#include "../includes/ui_helpers.hpp"

#include <memory>
#include <string>
#include <utility>

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/color.hpp>

namespace {

/*
 * Custom FTXUI DOM node that overrides default layout constraints.
 * Forces min_x = 1 and enables flex_shrink to prevent container overflow
 * when resizing the terminal window.
 */
class ShrinkableNode : public ftxui::Node {
 public:
    explicit ShrinkableNode(ftxui::Element child)
        : ftxui::Node({std::move(child)}) {}

    // Computes layout requirements with min_x = 1 and flex shrink enabled
    void ComputeRequirement() override {
        ftxui::Node::ComputeRequirement();
        requirement_.min_x = 1;
        requirement_.flex_shrink_x = 1;
        requirement_.flex_grow_x = 1;
    }

    // Propagates layout box to child
    void SetBox(ftxui::Box box) override {
        ftxui::Node::SetBox(box);
        if (!children_.empty()) {
            children_[0]->SetBox(box);
        }
    }
};

}  // namespace

// Wraps an element into a ShrinkableNode to permit dynamic window compression
ftxui::Element shrinkable(ftxui::Element child) {
    return std::make_shared<ShrinkableNode>(std::move(child));
}

// Constructs an auto-wrapping message element with optional channel and colored pseudo
ftxui::Element wrapLine(const std::string& pseudo,
                        const std::string& content,
                        ftxui::Color pseudo_color,
                        const std::string& channel) {
    if (pseudo.empty()) {
        return shrinkable(ftxui::paragraph(content));
    }
    std::string prefix;
    if (!channel.empty()) {
        prefix = pseudo + " (" + channel + "): ";
    } else {
        prefix = pseudo + ": ";
    }
    return shrinkable(ftxui::hbox({
        ftxui::text(prefix) | ftxui::color(pseudo_color),
        shrinkable(ftxui::paragraph(content)) | ftxui::flex,
    }));
}
