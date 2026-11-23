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
    return heading | borderStyled(ROUNDED, selected ? accent : edge)
        | bgcolor(selected ? selected_surface : surface);
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
        | size(HEIGHT, EQUAL, dimensions.height)
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
    const bool victory = title == "ESCAPE COMPLETED";
    tone = victory ? Color::RGB(65, 245, 105) : Color::RGB(255, 65, 100);
    static constexpr std::array<std::string_view, 6> game_over{
        " ██████╗  █████╗ ███╗   ███╗███████╗     ██████╗ ██╗   ██╗███████╗██████╗",
        "██╔════╝ ██╔══██╗████╗ ████║██╔════╝    ██╔═══██╗██║   ██║██╔════╝██╔══██╗",
        "██║  ███╗███████║██╔████╔██║█████╗      ██║   ██║██║   ██║█████╗  ██████╔╝",
        "██║   ██║██╔══██║██║╚██╔╝██║██╔══╝      ██║   ██║╚██╗ ██╔╝██╔══╝  ██╔══██╗",
        "╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗    ╚██████╔╝ ╚████╔╝ ███████╗██║  ██║",
        " ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝     ╚═════╝   ╚═══╝  ╚══════╝╚═╝  ╚═╝"};
    static constexpr std::array<std::string_view, 6> escape{
        "███████╗███████╗ ██████╗ █████╗ ██████╗ ███████╗",
        "██╔════╝██╔════╝██╔════╝██╔══██╗██╔══██╗██╔════╝",
        "█████╗  ███████╗██║     ███████║██████╔╝█████╗",
        "██╔══╝  ╚════██║██║     ██╔══██║██╔═══╝ ██╔══╝",
        "███████╗███████║╚██████╗██║  ██║██║     ███████╗",
        "╚══════╝╚══════╝ ╚═════╝╚═╝  ╚═╝╚═╝     ╚══════╝"};
    static constexpr std::array<std::string_view, 6> completed{
        " ██████╗ ██████╗ ███╗   ███╗██████╗ ██╗     ███████╗████████╗███████╗██████╗",
        "██╔════╝██╔═══██╗████╗ ████║██╔══██╗██║     ██╔════╝╚══██╔══╝██╔════╝██╔══██╗",
        "██║     ██║   ██║██╔████╔██║██████╔╝██║     █████╗     ██║   █████╗  ██║  ██║",
        "██║     ██║   ██║██║╚██╔╝██║██╔═══╝ ██║     ██╔══╝     ██║   ██╔══╝  ██║  ██║",
        "╚██████╗╚██████╔╝██║ ╚═╝ ██║██║     ███████╗███████╗   ██║   ███████╗██████╔╝",
        " ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚══════╝╚══════╝   ╚═╝   ╚══════╝╚═════╝"};
    const bool spacious = dimensions.height >= 28;
    const int content_width = dimensions.width + 2;
    Elements title_lines;
    if (victory) {
        Elements word;
        for (const auto& line : escape) word.push_back(text(std::string(line)));
        title_lines.push_back(vbox(std::move(word)) | hcenter);
        if (spacious) title_lines.push_back(separatorEmpty());
    }
    Elements word;
    for (const auto& line : victory ? completed : game_over)
        word.push_back(text(std::string(line)));
    title_lines.push_back(vbox(std::move(word)) | hcenter);
    Element heading = vbox(std::move(title_lines));
    heading->ComputeRequirement();
    if (heading->requirement().min_x > content_width ||
        heading->requirement().min_y + 8 > dimensions.height - 2)
        heading = victory ? vbox({text("ESCAPE"), text("COMPLETED")}) | hcenter : text(title);
    heading = heading | bold | color(tone);
    auto stat = [&](std::string label, std::string value) {
        return hbox({text(std::move(label)) | color(muted) |
                         size(WIDTH, EQUAL, 12), filler(),
                     text(std::move(value)) | bold | color(ink)});
    };
    Elements stats_lines{
        stat("Turnos:", std::to_string(observation.turn)),
        stat("Energía:", std::to_string(observation.energy) + " / " +
            std::to_string(observation.maximumEnergy)),
        stat("Puntaje:", std::to_string(observation.score)),
        stat("Recursos:", std::to_string(observation.collectedResources))};
    if (spacious)
        for (int i = 3; i > 0; --i)
            stats_lines.insert(stats_lines.begin() + i, separatorEmpty());
    const int stats_width = std::min(60, dimensions.width - 8);
    auto stats = vbox(std::move(stats_lines))
        | size(WIDTH, EQUAL, stats_width) | borderStyled(ROUNDED, tone) | hcenter;
    Elements lines{heading | hcenter,
        text(std::move(description)) | color(ink) | hcenter};
    if (spacious) lines.push_back(separatorEmpty());
    lines.push_back(std::move(stats));
    if (spacious) lines.push_back(separatorEmpty());
    lines.push_back(text(done ? "ENTER - MAIN MENU   |   Q - EXIT" : "Q - EXIT") |
                    color(muted) | hcenter);
    auto body = vbox(std::move(lines));
    body->ComputeRequirement();
    return window(text(" " + title + " ") | bold | color(tone),
                  vbox({filler(), body, filler()}), ROUNDED)
        | size(WIDTH, EQUAL, std::min(content_width + 2, body->requirement().min_x + 10))
        | size(HEIGHT, EQUAL, std::min(dimensions.height, body->requirement().min_y + 6))
        | color(edge) | bgcolor(background) | center | bgcolor(background);
}
}
