#include "../includes/widget.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include <ftxui/dom/node.hpp>
#include <ftxui/screen/box.hpp>

#include "../includes/image.hpp"

namespace {

class PositionedWidgetNode : public ftxui::Node {
 public:
  PositionedWidgetNode(ftxui::Element child, int x, int y, int width,
                       int height)
      : ftxui::Node({std::move(child)}),
        is_proportional_(false),
        x_(x),
        y_(y),
        width_(width),
        height_(height) {}

  PositionedWidgetNode(ftxui::Element child, float rx, float ry, float rw,
                       float rh)
      : ftxui::Node({std::move(child)}),
        is_proportional_(true),
        ratio_x_(rx),
        ratio_y_(ry),
        ratio_w_(rw),
        ratio_h_(rh) {}

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

      if (ratio_x_ < 0.0f) {
        int right_margin = static_cast<int>(parent_w * (-ratio_x_));
        target_x = box.x_max + 1 - w - right_margin;
      } else {
        target_x = box.x_min + static_cast<int>(parent_w * ratio_x_);
      }

      if (ratio_y_ < 0.0f) {
        int bottom_margin = static_cast<int>(parent_h * (-ratio_y_));
        target_y = box.y_max + 1 - h - bottom_margin;
      } else {
        target_y = box.y_min + static_cast<int>(parent_h * ratio_y_);
      }
    } else {
      w = std::max(1, std::min(width_, parent_w));
      h = std::max(1, std::min(height_, parent_h));

      target_x =
          (x_ >= 0) ? (box.x_min + x_) : (box.x_max + 1 + x_ - w + 1);
      target_y =
          (y_ >= 0) ? (box.y_min + y_) : (box.y_max + 1 + y_ - h + 1);
    }

    target_x = std::max(box.x_min, std::min(target_x, box.x_max - w + 1));
    target_y = std::max(box.y_min, std::min(target_y, box.y_max - h + 1));

    ftxui::Box child_box;
    child_box.x_min = target_x;
    child_box.x_max = target_x + w - 1;
    child_box.y_min = target_y;
    child_box.y_max = target_y + h - 1;

    children_[0]->SetBox(child_box);
  }

  void Render(ftxui::Screen& screen) override {
    if (children_.empty()) return;
    children_[0]->Render(screen);
  }

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

ftxui::Element buildInner(ftxui::Element content,
                          const std::string& title) {
  ftxui::Element inner = title.empty()
      ? ftxui::borderRounded(content | ftxui::flex)
      : ftxui::window(ftxui::text(title), content | ftxui::flex);
  return inner | ftxui::clear_under;
}

}  // namespace

ftxui::Element createWidget(int x, int y, int width, int height,
                            ftxui::Element content,
                            const std::string& title) {
  if (!content) {
    content = put_image("temp_image.png", 0, 0, false);
  }
  auto wgt = buildInner(content, title);
  return std::make_shared<PositionedWidgetNode>(wgt, x, y, width, height);
}

ftxui::Element createProportionalWidget(float ratio_x, float ratio_y,
                                        float ratio_width,
                                        float ratio_height,
                                        ftxui::Element content,
                                        const std::string& title) {
  if (!content) {
    content = put_image("temp_image.png", 0, 0, false);
  }
  auto wgt = buildInner(content, title);
  return std::make_shared<PositionedWidgetNode>(wgt, ratio_x, ratio_y,
                                                ratio_width, ratio_height);
}

ftxui::Element createWidgetImage(const std::string& image_path, int x, int y,
                                 int width, int height,
                                 const std::string& title) {
  ftxui::Element img = put_image(image_path, 0, 0, false);
  return createWidget(x, y, width, height, img, title);
}

ftxui::Element createProportionalWidgetImage(const std::string& image_path,
                                             float ratio_x, float ratio_y,
                                             float ratio_width,
                                             float ratio_height,
                                             const std::string& title) {
  ftxui::Element img = put_image(image_path, 0, 0, false);
  return createProportionalWidget(ratio_x, ratio_y, ratio_width, ratio_height,
                                  img, title);
}

void changeWidget(ftxui::Element widget, ftxui::Element new_content,
                  const std::string& title) {
  auto node = std::dynamic_pointer_cast<PositionedWidgetNode>(widget);
  if (!node) return;
  if (!new_content) {
    new_content = put_image("temp_image.png", 0, 0, false);
  }
  node->SetChild(buildInner(new_content, title));
}

void changeProportionalWidget(ftxui::Element widget,
                              ftxui::Element new_content,
                              const std::string& title) {
  changeWidget(widget, new_content, title);
}
