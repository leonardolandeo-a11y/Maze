#include "circuit_escape/ui_theme.h"
#include <ftxui/dom/canvas.hpp>
#include <algorithm>
#include <array>
#include <string_view>

namespace MazeUI {
using namespace ftxui;

Metrics metrics(std::optional<Dimensions> terminal) {
    const auto actual = terminal.value_or(Terminal::Size());
    const int width = actual.dimx > 0 ? actual.dimx : 80;
    const int height = actual.dimy > 0 ? actual.dimy : 24;
    return {std::max(1, width - 4), std::max(1, height - 2),
        width >= 120 && height >= 35 ? Layout::split :
        width >= 90 && height >= 28 ? Layout::stacked : Layout::compact,
        height < 28};
}

Element logo() {
    // A single stable wordmark. Canvas uses 2x4 pixels per terminal cell;
    // aligned 2x2 blocks avoid braille/font-dependent spacing.
    static constexpr std::array<std::string_view, 5> pixels{
        "10001 01110 11111 11111",
        "11011 10001 00010 10000",
        "10101 11111 00100 11110",
        "10001 10001 01000 10000",
        "10001 10001 11111 11111"};
    return canvas(46, 12, [](Canvas& image) {
        for (int y = 0; y < 5; ++y)
            for (int x = 0; x < 23; ++x)
                if (pixels[y][x] == '1')
                    for (int dx = 0; dx < 2; ++dx)
                        image.DrawBlock(x * 2 + dx, y * 2, true, accent);
    });
}

Element panel(std::string title, Element content) {
    return window(text(" " + title + " ") | bold | color(accent),
                  std::move(content) | color(ink), ROUNDED)
        | color(edge) | bgcolor(surface);
}

Element chip(std::string label, std::string value, Color tone) {
    return hbox({text(std::move(label) + " ") | color(muted),
                 text(std::move(value)) | bold | color(tone)});
}

Element choice(std::string title, std::string subtitle, bool selected,
               const Metrics& dimensions) {
    auto heading = hbox({text(selected ? " > " : "   ") | color(accent),
        text(std::move(title)) | bold | color(selected ? accent : ink), filler(),
        text(selected ? "ENTER " : "      ") | color(muted)});
    if (dimensions.dense)
        return heading | bgcolor(selected ? selected_surface : surface);
    Elements lines{heading};
    if (dimensions.height >= 33 && !subtitle.empty())
        lines.push_back(text("   " + subtitle) | color(muted));
    return vbox(std::move(lines)) | borderStyled(ROUNDED, selected ? accent : edge)
        | bgcolor(selected ? selected_surface : surface);
}

Element shell(std::string section, Element body, std::string controls,
              const Metrics& dimensions, bool brand) {
    Elements lines{
        hbox({text(" MAZE / ") | bold | color(accent),
              text(std::move(section)) | color(ink), filler(),
              text("CIRCUITO DE ESCAPE ") | color(muted)}),
        separator() | color(edge)};
    if (brand) lines.push_back(logo() | hcenter);
    lines.push_back(vbox({filler(), std::move(body), filler()}) | flex);
    lines.push_back(separator() | color(edge));
    lines.push_back(text(std::move(controls)) | color(muted) | hcenter);
    return vbox(std::move(lines)) | size(WIDTH, EQUAL, dimensions.width - 2)
        | size(HEIGHT, EQUAL, dimensions.height - 2)
        | borderStyled(ROUNDED, edge) | color(ink) | bgcolor(background)
        | center | bgcolor(background);
}

Element circuit(int frame, Color tone) {
    // Activity only: never represents progress of the generation worker.
    return canvas(64, 12, [frame, tone](Canvas& image) {
        const int head = (std::max(0, frame) / 2) % 30;
        image.DrawBlockLine(2, 6, 60, 6, edge);
        for (int i = 0; i < 4; ++i)
            image.DrawBlock(2 + ((head + i) % 30) * 2, 6, true, tone);
        image.DrawBlockLine(2, 2, 2, 6, tone);
        image.DrawBlockLine(60, 6, 60, 10, tone);
    });
}

Element result(int frame, int frames, std::string title, std::string description,
               const Observation& observation, Color tone,
               std::optional<Dimensions> terminal) {
    const auto dimensions = metrics(terminal);
    const bool done = frame >= frames - 1;
    auto stats = panel("RUN SUMMARY", vbox({
        chip("Turn", std::to_string(observation.turn)),
        chip("Energy", std::to_string(observation.energy) + "/" +
            std::to_string(observation.maximumEnergy)),
        chip("Score", std::to_string(observation.score)),
        chip("Resources", std::to_string(observation.collectedResources))}));
    auto heading = vbox({logo() | hcenter, separatorEmpty(),
        text(title) | bold | color(tone) | hcenter,
        text(std::move(description)) | color(ink) | hcenter,
        circuit(done ? frames - 1 : frame, tone) | hcenter});
    Element body = dimensions.layout == Layout::split
        ? hbox({heading | flex, text("  "), stats | flex})
        : vbox({heading, stats});
    return shell(title, std::move(body), done
        ? "ENTER - MAIN MENU   |   Q - EXIT" : "Q - EXIT", dimensions);
}
}
