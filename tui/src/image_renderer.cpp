// Terminal image renderer using stb_image

#include "../includes/image_renderer.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <ftxui/dom/node.hpp>
#include <ftxui/dom/requirement.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

using ftxui::Color;
using ftxui::color;
using ftxui::Element;
using ftxui::Node;
using ftxui::Screen;
using ftxui::text;

namespace {

// 24-bit RGB color representation
struct Rgb {
    std::uint8_t r, g, b;
};

// In-memory decoded bitmap and downsampled pixel cache
struct Bitmap {
    int w = 0;
    int h = 0;
    std::vector<Rgb> pixels;

    int cache_w = 0;
    int cache_h = 0;
    std::vector<Rgb> cache;
};

// Cache entry for loaded image file
struct ImageEntry {
    std::shared_ptr<Bitmap> bitmap;
    std::string error;
};

std::mutex images_mutex;
std::unordered_map<std::string, ImageEntry> loaded_images;

/*
 * Decodes image from disk using stb_image and premultiplies RGB by alpha
 * to prevent dark edge artifacts when blending transparent PNGs against terminal cells.
 */
ImageEntry loadImage(const std::string& path) {
    ImageEntry entry;
    int w = 0, h = 0, channels = 0;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
    if (!data) {
        const char* reason = stbi_failure_reason();
        entry.error = reason ? reason : "unknown error";
        return entry;
    }

    auto bmp = std::make_shared<Bitmap>();
    bmp->w = w;
    bmp->h = h;
    bmp->pixels.resize(
        static_cast<std::size_t>(w) * static_cast<std::size_t>(h));
    for (std::size_t i = 0; i < bmp->pixels.size(); ++i) {
        const unsigned char* p = data + 4 * i;
        const int a = p[3];
        // Premultiply RGB channels by alpha value to avoid dark fringe halos against terminal backgrounds
        bmp->pixels[i] = {static_cast<std::uint8_t>(p[0] * a / 255),
                          static_cast<std::uint8_t>(p[1] * a / 255),
                          static_cast<std::uint8_t>(p[2] * a / 255)};
    }
    stbi_image_free(data);
    entry.bitmap = bmp;
    return entry;
}

// Calculates target dimensions preserving aspect ratio within bounding limits
void adjustSize(const Bitmap& b, int max_w, int max_h, int& tw, int& th) {
    double s = 1.0;
    if (max_w > 0) s = std::min(s, static_cast<double>(max_w) / b.w);
    if (max_h > 0) s = std::min(s, static_cast<double>(max_h) / b.h);
    tw = std::max(1, static_cast<int>(b.w * s + 0.5));
    th = std::max(1, static_cast<int>(b.h * s + 0.5));
}

/*
 * Rescales image using an area-averaging box filter (integrating pixel coverage
 * over fractional intervals) and caches the scaled RGB buffer for fast redrawing.
 */
void rescale(Bitmap& b, int tw, int th) {
    if (b.cache_w == tw && b.cache_h == th) return;

    b.cache.assign(
        static_cast<std::size_t>(tw) * static_cast<std::size_t>(th),
        Rgb{0, 0, 0});
    for (int y = 0; y < th; ++y) {
        // Map output pixel interval to source bitmap continuous coordinates
        const int y0 = static_cast<int>(
            static_cast<std::int64_t>(y) * b.h / th);
        const int y1 = std::max(y0 + 1, static_cast<int>(
            static_cast<std::int64_t>(y + 1) * b.h / th));
        for (int x = 0; x < tw; ++x) {
            const int x0 = static_cast<int>(
                static_cast<std::int64_t>(x) * b.w / tw);
            const int x1 = std::max(x0 + 1, static_cast<int>(
                static_cast<std::int64_t>(x + 1) * b.w / tw));

            // Integrate RGB values over covered source pixel sub-matrix
            std::uint64_t r = 0, g = 0, bl = 0;
            for (int sy = y0; sy < y1; ++sy) {
                for (int sx = x0; sx < x1; ++sx) {
                    const Rgb& p = b.pixels[static_cast<std::size_t>(
                        sy) * b.w + sx];
                    r += p.r;
                    g += p.g;
                    bl += p.b;
                }
            }
            const std::uint64_t n = static_cast<std::uint64_t>(
                y1 - y0) * (x1 - x0);
            // Store normalized average color into cache
            b.cache[static_cast<std::size_t>(y) * tw + x] = {
                static_cast<std::uint8_t>(r / n),
                static_cast<std::uint8_t>(g / n),
                static_cast<std::uint8_t>(bl / n)};
        }
    }
    b.cache_w = tw;
    b.cache_h = th;
}

// Clamps available dimension against optional maximum boundary
int clampDimension(int available, int maximum) {
    return maximum > 0 ? std::min(available, maximum) : available;
}

// Custom FTXUI DOM Node for drawing half-block terminal pixels
class ImageNode : public Node {
 public:
    ImageNode(std::shared_ptr<Bitmap> bmp, int max_w,
              int max_h, bool keep_ratio = false)
        : bmp_(std::move(bmp)),
          max_w_(max_w),
          max_h_(max_h),
          keep_ratio_(keep_ratio) {}

    // Computes required character cell dimensions for terminal layout engine
    void ComputeRequirement() override {
        if (max_w_ <= 0 && max_h_ <= 0) {
            requirement_.min_x = 1;
            requirement_.min_y = 1;
        } else {
            int tw, th;
            adjustSize(*bmp_, max_w_, max_h_ * 2, tw, th);
            requirement_.min_x = tw;
            requirement_.min_y = (th + 1) / 2;
        }
        requirement_.flex_shrink_x = 1;
        requirement_.flex_shrink_y = 1;
        requirement_.flex_grow_x = 1;
        requirement_.flex_grow_y = 1;
    }

    /*
     * Renders two vertical pixels per terminal cell using Unicode upper half-block ('▀').
     * Sets top pixel color as foreground and bottom pixel color as background to achieve
     * double vertical resolution with 24-bit TrueColor escapes.
     */
    void Render(Screen& screen) override {
        const int cw = box_.x_max - box_.x_min + 1;
        const int ch = box_.y_max - box_.y_min + 1;
        if (cw <= 0 || ch <= 0) return;

        int tw = cw;
        int th = ch * 2;
        if (keep_ratio_) {
            adjustSize(*bmp_, clampDimension(cw, max_w_),
                       clampDimension(ch * 2, max_h_ * 2), tw, th);
        } else {
            if (max_w_ > 0) tw = std::min(tw, max_w_);
            if (max_h_ > 0) th = std::min(th, max_h_ * 2);
        }
        rescale(*bmp_, tw, th);

        const int lines = (th + 1) / 2;
        // Center image within available terminal box margins
        const int offset_x = std::max(0, (cw - tw) / 2);
        const int offset_y = std::max(0, (ch - lines) / 2);
        for (int y = 0; y < lines; ++y) {
            for (int x = 0; x < tw; ++x) {
                // Top pixel of the cell pair
                const Rgb& top = bmp_->cache[static_cast<std::size_t>(
                    2 * y) * tw + x];
                auto& pix = screen.PixelAt(box_.x_min + offset_x + x,
                    box_.y_min + offset_y + y);
                // Half-block character: top half takes foreground color
                pix.character = "▀";
                pix.foreground_color = Color::RGB(top.r, top.g, top.b);
                if (2 * y + 1 < th) {
                    // Bottom pixel of the cell pair provides background color
                    const Rgb& bottom = bmp_->cache[static_cast<std::size_t>(
                        2 * y + 1) * tw + x];
                    pix.background_color = Color::RGB(
                        bottom.r, bottom.g, bottom.b);
                } else {
                    pix.background_color = Color::Default;
                }
            }
        }
    }

 private:
    std::shared_ptr<Bitmap> bmp_;
    int max_w_;
    int max_h_;
    bool keep_ratio_;
};

}  // namespace

// Retrieves cached bitmap or loads new image from disk and returns an ImageNode
Element renderImage(const std::string& path, int max_width,
                    int max_height, bool keep_ratio) {
    ImageEntry entry;
    {
        std::lock_guard<std::mutex> lock(images_mutex);
        auto it = loaded_images.find(path);
        if (it == loaded_images.end()) {
            it = loaded_images.emplace(path, loadImage(path)).first;
        }
        entry = it->second;
    }

    if (!entry.bitmap) {
        return text("[image error] " + path + " : " + entry.error) |
               color(Color::Red);
    }
    return std::make_shared<ImageNode>(entry.bitmap, max_width,
                                       max_height, keep_ratio);
}
