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
    static const std::array<std::string, 6> mazeTitle = {
        "███╗   ███╗ █████╗ ███████╗███████╗",
        "████╗ ████║██╔══██╗╚══███╔╝██╔════╝",
        "██╔████╔██║███████║  ███╔╝ █████╗  ",
        "██║╚██╔╝██║██╔══██║ ███╔╝  ██╔══╝  ",
        "██║ ╚═╝ ██║██║  ██║███████╗███████╗",
        "╚═╝     ╚═╝╚═╝  ╚═╝╚══════╝╚══════╝"
    };

    Elements lines;
    for (const auto& line : mazeTitle)
        lines.push_back(text(line) | color(accent) | bold);
    return vbox(std::move(lines));
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
    return heading | bgcolor(selected ? selected_surface : surface);
}

Element shell(std::string section, Element body, std::string controls,
              const Metrics& dimensions, bool brand) {
    Elements lines{
        hbox({text(" MAZE / ") | bold | color(accent),
              text(std::move(section)) | color(ink), filler(),
              text("CIRCUITO DE ESCAPE ") | color(muted)}),
        separator() | color(edge)};
    if (brand) lines.push_back(vbox({logo() | hcenter,
        text("CIRCUITO DE ESCAPE") | bold | hcenter,
        text("Two switches. One way out.") | color(muted) | hcenter}));
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
    // Five-pixel glyphs use half-height blocks so terminal cell proportions stay intact.
    static constexpr std::string_view alphabet = "ACDEGLMOPRSTV";
    static constexpr std::array<std::array<int, 5>, 13> glyphs{{
        {{14,17,31,17,17}}, {{15,16,16,16,15}}, {{30,17,17,17,30}},
        {{31,16,30,16,31}}, {{15,16,23,17,15}}, {{16,16,16,16,31}},
        {{17,27,21,17,17}}, {{14,17,17,17,14}}, {{30,17,30,16,16}},
        {{30,17,30,18,17}}, {{15,16,14,1,30}}, {{31,4,4,4,4}},
        {{17,17,17,10,4}}
    }};
    const int scale = dimensions.width >= int(title.size()) * 6 ? 2 : 1;
    auto heading = canvas((int(title.size()) * 6 - 1) * scale, 10 * scale,
        [title, tone, scale](Canvas& image) {
            for (int i = 0; i < int(title.size()); ++i) {
                const auto glyph = alphabet.find(title[i]);
                if (glyph == std::string_view::npos) continue;
                for (int y = 0; y < 5; ++y)
                    for (int x = 0; x < 5; ++x)
                        if (glyphs[glyph][y] & (1 << (4 - x)))
                            for (int dx = 0; dx < scale; ++dx)
                                for (int dy = 0; dy < scale; ++dy)
                                    image.DrawBlock((i * 6 + x) * scale + dx,
                                        (y * scale + dy) * 2, true, tone);
            }
        });
    auto stat = [&](std::string label, std::string value) {
        return hbox({text(std::move(label)) | color(muted) |
                         size(WIDTH, EQUAL, 12),
                     text(std::move(value)) | bold | color(ink)});
    };
    auto stats = vbox({
        stat("Turnos:", std::to_string(observation.turn)),
        stat("Energía:", std::to_string(observation.energy) + " / " +
            std::to_string(observation.maximumEnergy)),
        stat("Puntaje:", std::to_string(observation.score)),
        stat("Recursos:", std::to_string(observation.collectedResources))})
        | size(WIDTH, EQUAL, 30) | borderStyled(ROUNDED, tone) | hcenter;
    Elements lines;
    if (title == "ESCAPE COMPLETED")
        lines.push_back(vbox({text("╰─┤█├─╯"), text("  ▀█▀  ")}) |
                        color(warning) | hcenter);
    lines.push_back(heading | hcenter);
    lines.push_back(text(std::move(description)) | color(ink) | hcenter);
    lines.push_back(separatorEmpty());
    lines.push_back(std::move(stats));
    lines.push_back(separatorEmpty());
    lines.push_back(text(done ? "ENTER - MAIN MENU   |   Q - EXIT" : "Q - EXIT") |
                    color(muted) | hcenter);
    return window(text(" " + title + " ") | bold | color(tone),
                  vbox({filler(), vbox(std::move(lines)), filler()}), ROUNDED)
        | size(WIDTH, EQUAL, dimensions.width - 2)
        | size(HEIGHT, EQUAL, dimensions.height - 2)
        | color(edge) | bgcolor(background) | center | bgcolor(background);
}
}
