#include "image.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <ftxui/dom/node.hpp>
#include <ftxui/dom/requirement.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>

// stb_image : bibliothèque « header-only » qui décode les images (lib/stb_image.h)
#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

using namespace ftxui;

namespace {

struct Rgb {
    std::uint8_t r, g, b;
};

struct Bitmap {
    int w = 0;
    int h = 0;
    std::vector<Rgb> pixels;

    // Dernière version redimensionnée (évite de la recalculer à chaque rendu)
    int cache_w = 0;
    int cache_h = 0;
    std::vector<Rgb> cache;
};

struct Entree {
    std::shared_ptr<Bitmap> bitmap; // nullptr si le chargement a échoué
    std::string erreur;
};

// Cache des images déjà chargées, indexé par chemin de fichier
std::mutex images_mutex;
std::unordered_map<std::string, Entree> images_chargees;

Entree charger(const std::string& chemin) {
    Entree e;
    int w = 0, h = 0, canaux = 0;
    unsigned char* data = stbi_load(chemin.c_str(), &w, &h, &canaux, 4); // force RGBA
    if (!data) {
        const char* raison = stbi_failure_reason();
        e.erreur = raison ? raison : "erreur inconnue";
        return e;
    }

    auto bmp = std::make_shared<Bitmap>();
    bmp->w = w;
    bmp->h = h;
    bmp->pixels.resize(static_cast<std::size_t>(w) * static_cast<std::size_t>(h));
    for (std::size_t i = 0; i < bmp->pixels.size(); ++i) {
        const unsigned char* p = data + 4 * i;
        const int a = p[3]; // transparence : on fond l'image sur du noir
        bmp->pixels[i] = {static_cast<std::uint8_t>(p[0] * a / 255),
                          static_cast<std::uint8_t>(p[1] * a / 255),
                          static_cast<std::uint8_t>(p[2] * a / 255)};
    }
    stbi_image_free(data);
    e.bitmap = bmp;
    return e;
}

// Taille (en pixels) de l'image une fois ajustée dans max_w x max_h, sans agrandir.
// Une valeur <= 0 signifie « pas de limite ».
void ajuster(const Bitmap& b, int max_w, int max_h, int& tw, int& th) {
    double s = 1.0;
    if (max_w > 0) s = std::min(s, static_cast<double>(max_w) / b.w);
    if (max_h > 0) s = std::min(s, static_cast<double>(max_h) / b.h);
    tw = std::max(1, static_cast<int>(b.w * s + 0.5));
    th = std::max(1, static_cast<int>(b.h * s + 0.5));
}

// Redimensionne par moyenne des pixels (meilleure qualité que « le plus proche »)
void redimensionner(Bitmap& b, int tw, int th) {
    if (b.cache_w == tw && b.cache_h == th) return;

    b.cache.assign(static_cast<std::size_t>(tw) * static_cast<std::size_t>(th), Rgb{0, 0, 0});
    for (int y = 0; y < th; ++y) {
        const int y0 = static_cast<int>(static_cast<std::int64_t>(y) * b.h / th);
        const int y1 = std::max(y0 + 1, static_cast<int>(static_cast<std::int64_t>(y + 1) * b.h / th));
        for (int x = 0; x < tw; ++x) {
            const int x0 = static_cast<int>(static_cast<std::int64_t>(x) * b.w / tw);
            const int x1 = std::max(x0 + 1, static_cast<int>(static_cast<std::int64_t>(x + 1) * b.w / tw));

            std::uint64_t r = 0, g = 0, bl = 0;
            for (int sy = y0; sy < y1; ++sy) {
                for (int sx = x0; sx < x1; ++sx) {
                    const Rgb& p = b.pixels[static_cast<std::size_t>(sy) * b.w + sx];
                    r += p.r;
                    g += p.g;
                    bl += p.b;
                }
            }
            const std::uint64_t n = static_cast<std::uint64_t>(y1 - y0) * (x1 - x0);
            b.cache[static_cast<std::size_t>(y) * tw + x] = {static_cast<std::uint8_t>(r / n),
                                                             static_cast<std::uint8_t>(g / n),
                                                             static_cast<std::uint8_t>(bl / n)};
        }
    }
    b.cache_w = tw;
    b.cache_h = th;
}

int limite(int disponible, int maximum) {
    return maximum > 0 ? std::min(disponible, maximum) : disponible;
}

// Élément FTXUI personnalisé : dessine l'image directement dans l'écran
class NoeudImage : public Node {
public:
    NoeudImage(std::shared_ptr<Bitmap> bmp, int largeur_max, int hauteur_max, bool garder_ratio = false)
        : bmp_(std::move(bmp)),
          largeur_max_(largeur_max),
          hauteur_max_(hauteur_max),
          garder_ratio_(garder_ratio) {}

    void ComputeRequirement() override {
        if (largeur_max_ <= 0 && hauteur_max_ <= 0) {
            requirement_.min_x = 1;
            requirement_.min_y = 1;
        } else {
            int tw, th;
            ajuster(*bmp_, largeur_max_, hauteur_max_ * 2, tw, th); // 1 cellule = 2 pixels de haut
            requirement_.min_x = tw;
            requirement_.min_y = (th + 1) / 2;
        }
        requirement_.flex_shrink_x = 1; // peut rétrécir si la fenêtre est petite
        requirement_.flex_shrink_y = 1;
        requirement_.flex_grow_x = 1;
        requirement_.flex_grow_y = 1;
    }

    void Render(Screen& screen) override {
        const int cw = box_.x_max - box_.x_min + 1;
        const int ch = box_.y_max - box_.y_min + 1;
        if (cw <= 0 || ch <= 0) return;

        int tw = cw;
        int th = ch * 2;
        if (garder_ratio_) {
            ajuster(*bmp_, limite(cw, largeur_max_), limite(ch * 2, hauteur_max_ * 2), tw, th);
        } else {
            if (largeur_max_ > 0) tw = std::min(tw, largeur_max_);
            if (hauteur_max_ > 0) th = std::min(th, hauteur_max_ * 2);
        }
        redimensionner(*bmp_, tw, th);

        const int lignes = (th + 1) / 2;
        const int offset_x = std::max(0, (cw - tw) / 2);
        const int offset_y = std::max(0, (ch - lignes) / 2);
        for (int y = 0; y < lignes; ++y) {
            for (int x = 0; x < tw; ++x) {
                const Rgb& haut = bmp_->cache[static_cast<std::size_t>(2 * y) * tw + x];
                auto& pix = screen.PixelAt(box_.x_min + offset_x + x, box_.y_min + offset_y + y);
                pix.character = "▀"; // moitié haute = couleur de texte, moitié basse = fond
                pix.foreground_color = Color::RGB(haut.r, haut.g, haut.b);
                if (2 * y + 1 < th) {
                    const Rgb& bas = bmp_->cache[static_cast<std::size_t>(2 * y + 1) * tw + x];
                    pix.background_color = Color::RGB(bas.r, bas.g, bas.b);
                } else {
                    pix.background_color = Color::Default; // hauteur impaire : dernière demi-ligne vide
                }
            }
        }
    }

private:
    std::shared_ptr<Bitmap> bmp_;
    int largeur_max_;
    int hauteur_max_;
    bool garder_ratio_;
};

} // namespace

Element put_image(const std::string& chemin, int largeur_max, int hauteur_max, bool garder_ratio) {
    Entree entree;
    {
        std::lock_guard<std::mutex> lock(images_mutex);
        auto it = images_chargees.find(chemin);
        if (it == images_chargees.end()) {
            it = images_chargees.emplace(chemin, charger(chemin)).first;
        }
        entree = it->second;
    }

    if (!entree.bitmap) {
        return text("[image illisible] " + chemin + " : " + entree.erreur) | color(Color::Red);
    }
    return std::make_shared<NoeudImage>(entree.bitmap, largeur_max, hauteur_max, garder_ratio);
}
