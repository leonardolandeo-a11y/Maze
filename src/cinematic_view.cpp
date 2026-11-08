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
    const double scale = std::min({1.0, available_width / double(width),
                                  available_height / double(height)});
    const int fitted_width = std::max(1, int(width * scale));
    const int fitted_height = std::max(1, int(height * scale));
    if (scale == 1.0) return scene;

    auto original = Screen::Create(Dimension::Fixed(width), Dimension::Fixed(height));
    Render(original, scene);
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
