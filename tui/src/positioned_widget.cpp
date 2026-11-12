// Absolute and proportional positioned UI widgets

#include "../includes/positioned_widget.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include <ftxui/dom/node.hpp>
#include <ftxui/screen/box.hpp>

#include "../includes/image_renderer.hpp"

namespace {

// FTXUI DOM node that positions its single child at explicit or proportional coordinates
class PositionedWidgetNode : public ftxui::Node {
 public:
    // Constructs node with absolute integer terminal coordinates
    PositionedWidgetNode(ftxui::Element child, int x, int y, int width,
                         int height)
        : ftxui::Node({std::move(child)}),
          is_proportional_(false),
          x_(x),
          y_(y),
          width_(width),
          height_(height) {}

    // Constructs node with proportional coordinates relative to parent box
    PositionedWidgetNode(ftxui::Element child, float rx, float ry, float rw,
                         float rh)
        : ftxui::Node({std::move(child)}),
          is_proportional_(true),
          ratio_x_(rx),
          ratio_y_(ry),
          ratio_w_(rw),
          ratio_h_(rh) {}

    // Computes layout requirement deferring to child
    void ComputeRequirement() override {
        if (!children_.empty()) {
            children_[0]->ComputeRequirement();
        }
        requirement_.min_x = 1;
        requirement_.min_y = 1;
        requirement_.flex_shrink_x = 1;
        requirement_.flex_shrink_y = 1;
        requirement_.flex_grow_x = 1;
        requirement_.flex_grow_y = 1;
    }

    /*
     * Computes child bounding box supporting both absolute offsets and relative ratios.
     * Negative values anchor coordinates to the right or bottom edges of the parent container.
     * Clamps final coordinates to guarantee content stays within terminal bounds.
     */
    void SetBox(ftxui::Box box) override {
        box_ = box;
        if (children_.empty()) return;

        int parent_w = box.x_max - box.x_min + 1;
        int parent_h = box.y_max - box.y_min + 1;
        if (parent_w <= 0 || parent_h <= 0) return;

        int w = 0, h = 0, target_x = 0, target_y = 0;

        if (is_proportional_) {
            w = std::max(4, static_cast<int>(parent_w * ratio_w_));
            h = std::max(3, static_cast<int>(parent_h * ratio_h_));

            // Negative ratio: anchor relative to right edge of parent box
            if (ratio_x_ < 0.0f) {
                int right_margin = static_cast<int>(parent_w * (-ratio_x_));
                target_x = box.x_max + 1 - w - right_margin;
            } else {
                target_x = box.x_min + static_cast<int>(parent_w * ratio_x_);
            }

            // Negative ratio: anchor relative to bottom edge of parent box
            if (ratio_y_ < 0.0f) {
                int bottom_margin = static_cast<int>(parent_h * (-ratio_y_));
                target_y = box.y_max + 1 - h - bottom_margin;
            } else {
                target_y = box.y_min + static_cast<int>(parent_h * ratio_y_);
            }
        } else {
            w = std::max(1, std::min(width_, parent_w));
            h = std::max(1, std::min(height_, parent_h));

            // Negative pixel coordinates anchor from the opposite container edge
            target_x =
                (x_ >= 0) ? (box.x_min + x_) : (box.x_max + 1 + x_ - w + 1);
            target_y =
                (y_ >= 0) ? (box.y_min + y_) : (box.y_max + 1 + y_ - h + 1);
        }

        // Clamp computed coordinates so widget remains strictly within terminal bounds
        target_x = std::max(box.x_min, std::min(target_x, box.x_max - w + 1));
        target_y = std::max(box.y_min, std::min(target_y, box.y_max - h + 1));

        ftxui::Box child_box;
        child_box.x_min = target_x;
        child_box.x_max = target_x + w - 1;
        child_box.y_min = target_y;
        child_box.y_max = target_y + h - 1;

        children_[0]->SetBox(child_box);
    }

    // Delegates rendering to positioned child node
    void Render(ftxui::Screen& screen) override {
        if (children_.empty()) return;
        children_[0]->Render(screen);
    }

    // Updates inner child element
    void SetChild(ftxui::Element child) {
        children_.clear();
        children_.push_back(std::move(child));
    }

 private:
    bool is_proportional_ = false;
    int x_ = 0;
    int y_ = 0;
    int width_ = 0;
    int height_ = 0;
    float ratio_x_ = 0.0f;
    float ratio_y_ = 0.0f;
    float ratio_w_ = 0.0f;
    float ratio_h_ = 0.0f;
};

// Wraps element in titled window or rounded border with clear_under decoration
ftxui::Element buildInner(ftxui::Element content,
                          const std::string& title) {
    ftxui::Element inner = title.empty()
        ? ftxui::borderRounded(content | ftxui::flex)
        : ftxui::window(ftxui::text(title), content | ftxui::flex);
    return inner | ftxui::clear_under;
}

}  // namespace

// Creates floating widget with absolute coordinates
ftxui::Element createWidget(int x, int y, int width, int height,
                            ftxui::Element content,
                            const std::string& title) {
    if (!content) {
        content = renderImage("temp_image.png", 0, 0, false);
    }
    auto wgt = buildInner(content, title);
    return std::make_shared<PositionedWidgetNode>(wgt, x, y, width, height);
}

// Creates floating widget with proportional coordinates
ftxui::Element createProportionalWidget(float ratio_x, float ratio_y,
                                         float ratio_width,
                                         float ratio_height,
                                         ftxui::Element content,
                                         const std::string& title) {
    if (!content) {
        content = renderImage("temp_image.png", 0, 0, false);
    }
    auto wgt = buildInner(content, title);
    return std::make_shared<PositionedWidgetNode>(
        wgt, ratio_x, ratio_y, ratio_width, ratio_height);
}

// Creates positioned floating widget displaying an image from disk
ftxui::Element createWidgetImage(const std::string& image_path, int x, int y,
                                 int width, int height,
                                 const std::string& title) {
    ftxui::Element img = renderImage(image_path, 0, 0, false);
    return createWidget(x, y, width, height, img, title);
}

// Creates proportional positioned floating widget displaying an image from disk
ftxui::Element createProportionalWidgetImage(const std::string& image_path,
                                             float ratio_x, float ratio_y,
                                             float ratio_width,
                                             float ratio_height,
                                             const std::string& title) {
    ftxui::Element img = renderImage(image_path, 0, 0, false);
    return createProportionalWidget(
        ratio_x, ratio_y, ratio_width, ratio_height, img, title);
}

// Replaces content and title of an existing positioned widget in-place
void changeWidget(ftxui::Element widget, ftxui::Element new_content,
                  const std::string& title) {
    auto node = std::dynamic_pointer_cast<PositionedWidgetNode>(widget);
    if (!node) return;
    if (!new_content) {
        new_content = renderImage("temp_image.png", 0, 0, false);
    }
    node->SetChild(buildInner(new_content, title));
}

// Updates existing proportional widget content in-place
void changeProportionalWidget(ftxui::Element widget,
                              ftxui::Element new_content,
                              const std::string& title) {
    changeWidget(widget, new_content, title);
}
