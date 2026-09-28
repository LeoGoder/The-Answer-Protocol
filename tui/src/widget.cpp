#include "widget.hpp"
#include "image.hpp"

#include <algorithm>
#include <memory>
#include <utility>

#include <ftxui/dom/node.hpp>
#include <ftxui/screen/box.hpp>

namespace {

class PositionedWidgetNode : public ftxui::Node {
 public:
  // Mode absolu en cellules de terminal
  PositionedWidgetNode(ftxui::Element child, int x, int y, int width, int height)
      : ftxui::Node({std::move(child)}),
        is_proportional_(false),
        x_(x),
        y_(y),
        width_(width),
        height_(height) {}

  // Mode proportionnel (ratios relatifs à l'écran : ex 0.35f = 35%)
  PositionedWidgetNode(ftxui::Element child, float rx, float ry, float rw, float rh)
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

      // Si ratio_x_ < 0 : marge par rapport au bord droit
      if (ratio_x_ < 0.0f) {
        int marge_droite = static_cast<int>(parent_w * (-ratio_x_));
        target_x = box.x_max + 1 - w - marge_droite;
      } else {
        target_x = box.x_min + static_cast<int>(parent_w * ratio_x_);
      }

      // Si ratio_y_ < 0 : marge par rapport au bord inférieur
      if (ratio_y_ < 0.0f) {
        int marge_bas = static_cast<int>(parent_h * (-ratio_y_));
        target_y = box.y_max + 1 - h - marge_bas;
      } else {
        target_y = box.y_min + static_cast<int>(parent_h * ratio_y_);
      }
    } else {
      w = std::max(1, std::min(width_, parent_w));
      h = std::max(1, std::min(height_, parent_h));

      target_x = (x_ >= 0) ? (box.x_min + x_) : (box.x_max + 1 + x_ - w + 1);
      target_y = (y_ >= 0) ? (box.y_min + y_) : (box.y_max + 1 + y_ - h + 1);
    }

    // Ajuster dans les limites strictes du conteneur parent
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

}  // namespace

ftxui::Element creerWidget(int x, int y, int largeur, int hauteur,
                           ftxui::Element contenu,
                           const std::string& titre) {
  // put_image avec 0, 0 et false (garder_ratio=false) permet à l'image
  // de s'étendre et d'adapter sa taille et sa résolution à 100% du widget
  if (!contenu) {
    contenu = put_image("temp_image.png", 0, 0, false);
  }

  ftxui::Element inner = titre.empty()
      ? ftxui::borderRounded(contenu | ftxui::flex)
      : ftxui::window(ftxui::text(titre), contenu | ftxui::flex);

  auto wgt = inner | ftxui::clear_under;
  return std::make_shared<PositionedWidgetNode>(wgt, x, y, largeur, hauteur);
}

ftxui::Element creerWidgetProportionnel(float ratio_x, float ratio_y,
                                       float ratio_largeur, float ratio_hauteur,
                                       ftxui::Element contenu,
                                       const std::string& titre) {
  if (!contenu) {
    contenu = put_image("temp_image.png", 0, 0, false);
  }

  ftxui::Element inner = titre.empty()
      ? ftxui::borderRounded(contenu | ftxui::flex)
      : ftxui::window(ftxui::text(titre), contenu | ftxui::flex);

  auto wgt = inner | ftxui::clear_under;
  return std::make_shared<PositionedWidgetNode>(wgt, ratio_x, ratio_y,
                                                ratio_largeur, ratio_hauteur);
}

ftxui::Element creerWidgetImage(const std::string& chemin_image,
                                int x, int y, int largeur, int hauteur,
                                const std::string& titre) {
  ftxui::Element img = put_image(chemin_image, 0, 0, false);
  return creerWidget(x, y, largeur, hauteur, img, titre);
}

ftxui::Element creerWidgetImageProportionnel(const std::string& chemin_image,
                                            float ratio_x, float ratio_y,
                                            float ratio_largeur, float ratio_hauteur,
                                            const std::string& titre) {
  ftxui::Element img = put_image(chemin_image, 0, 0, false);
  return creerWidgetProportionnel(ratio_x, ratio_y, ratio_largeur, ratio_hauteur,
                                  img, titre);
}
