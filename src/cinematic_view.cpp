#include "circuit_escape/cinematic_view.h"
#include <ftxui/screen/screen.hpp>
#include <algorithm>

namespace CinematicView {
using namespace ftxui;
namespace {
constexpr int width = 90;
constexpr int height = 20;
}
Element Stage(Element content) {
    return std::move(content) | size(WIDTH, EQUAL, width) |
        size(HEIGHT, EQUAL, height) | center | bgcolor(Color::Black);
}

Element Fit(Element scene, std::optional<Dimensions> terminal) {
    const auto viewport = terminal.value_or(Terminal::Size());
    const int available_width = viewport.dimx > 0 ? viewport.dimx : 80;
    const int available_height = viewport.dimy > 0 ? viewport.dimy : 24;
    const double scale = std::min(available_width / double(width),
                                  available_height / double(height));
    const int fitted_width = std::max(1, int(width * scale));
    const int fitted_height = std::max(1, int(height * scale));

    auto original = Screen::Create(Dimension::Fixed(width), Dimension::Fixed(height));
    Render(original, scene);
    bool character_field = true;
    for (int y = 0; y < height && character_field; ++y)
        for (int x = 0; x < width; ++x) {
            const auto& glyph = original.PixelAt(x, y).character;
            if (glyph.size() != 1 || glyph == " ") { character_field = false; break; }
        }
    if (character_field) {
        // Extend only the dense character field; keep its central art proportional.
        Elements rows;
        for (int y = 0; y < available_height; ++y) {
            Elements cells;
            const int sy = (y - (available_height - fitted_height) / 2) * height / fitted_height;
            for (int x = 0; x < available_width; ++x) {
                const int sx = (x - (available_width - fitted_width) / 2) * width / fitted_width;
                const auto& pixel = original.PixelAt((sx % width + width) % width,
                                                     (sy % height + height) % height);
                auto cell = text(pixel.character) | color(pixel.foreground_color);
                cells.push_back(pixel.bold ? cell | bold : cell);
            }
            rows.push_back(hbox(std::move(cells)));
        }
        return vbox(std::move(rows)) | bgcolor(Color::Black);
    }
    if (scale == 1.0) return scene;
    if (scale > 1.0) {
        // Text keeps its native glyphs. Sprite frames contain only half/full blocks.
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x) {
                const auto& glyph = original.PixelAt(x, y).character;
                if (!glyph.empty() && glyph != " " && glyph != "█" &&
                    glyph != "▀" && glyph != "▄") return scene;
            }
        auto sample = [&](int x, int half_y) {
            const int source_y = half_y * height / fitted_height;
            const auto& pixel = original.PixelAt(x * width / fitted_width, source_y / 2);
            const bool foreground = pixel.character == "█" ||
                (pixel.character == "▀" && source_y % 2 == 0) ||
                (pixel.character == "▄" && source_y % 2 != 0);
            return foreground ? pixel.foreground_color : pixel.background_color;
        };
        Elements rows;
        for (int y = 0; y < fitted_height; ++y) {
            Elements columns;
            for (int x = 0; x < fitted_width; ++x) {
                const auto top = sample(x, y * 2);
                const auto bottom = sample(x, y * 2 + 1);
                columns.push_back(text(top == Color::Black && bottom == top ? " " : "▀") |
                                  color(top) | bgcolor(bottom));
            }
            rows.push_back(hbox(std::move(columns)));
        }
        return vbox(std::move(rows)) | center | bgcolor(Color::Black);
    }
    Elements rows;
    for (int y = 0; y < fitted_height; ++y) {
        Elements columns;
        for (int x = 0; x < fitted_width; ++x) {
            const auto& pixel = original.PixelAt(x * width / fitted_width,
                                                 y * height / fitted_height);
            auto cell = text(pixel.character.empty() ? " " : pixel.character) |
                color(pixel.foreground_color) | bgcolor(pixel.background_color);
            if (pixel.bold) cell = cell | bold;
            columns.push_back(std::move(cell));
        }
        rows.push_back(hbox(std::move(columns)));
    }
    return vbox(std::move(rows)) | center | bgcolor(Color::Black);
}
}
