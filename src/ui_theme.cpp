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
    const std::string icon = title == "START GAME" ? "✚" :
        title == "DIFFICULTY" ? "▥" : title == "RENDER MODE" ? "▣" :
        title == "HELP" ? "?" : title == "QUIT" ? "↪" : "◇";
    auto heading = hbox({text(selected ? " > " : "   ") | color(accent),
        text(std::move(title)) | bold | color(selected ? accent : ink), filler(),
        text(selected ? " ENTER " : "       ") | color(muted)});
    if (dimensions.height < 33)
        return window(hbox({text(" " + icon) | bold | color(accent), heading}),
            text(" " + subtitle) | color(muted), ROUNDED)
            | color(selected ? accent : edge)
            | bgcolor(selected ? selected_surface : surface);
    return hbox({text(" " + icon + " ") | bold | color(accent),
        vbox({heading, text("   " + subtitle) | color(muted)}) | flex})
        | borderStyled(ROUNDED, selected ? accent : edge)
        | bgcolor(selected ? selected_surface : surface);
}

Element shell(std::string section, Element body, std::string controls,
              const Metrics& dimensions, bool brand) {
    const bool main_menu = section == "MAIN MENU";
    Elements lines{
        hbox({text(" MAZE / ") | bold | color(accent),
              text(std::move(section)) | color(ink), filler(),
              text(main_menu ? "" : "CIRCUITO DE ESCAPE ") | color(muted)}),
        separator() | color(edge)};
    if (brand) {
        Elements branding{logo() | hcenter};
        if (!main_menu) branding.push_back(text("CIRCUITO DE ESCAPE") | bold | hcenter);
        branding.push_back(text("Two switches. One way out.") | color(muted) | hcenter);
        lines.push_back(vbox(std::move(branding)));
    }
    lines.push_back(vbox({filler(), std::move(body), filler()}) | flex);
    lines.push_back(separator() | color(edge));
    lines.push_back(text(std::move(controls)) | color(muted) | hcenter);
    return vbox(std::move(lines)) | size(WIDTH, EQUAL, dimensions.width + 2)
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
        heading->requirement().min_y + 10 > dimensions.height)
        heading = victory ? vbox({text("ESCAPE"), text("COMPLETED")}) | hcenter : text(title);
    heading = heading | bold | color(tone);
    heading->ComputeRequirement();
    const int heading_height = heading->requirement().min_y;
    const bool boxed_keys = dimensions.height >= heading_height + 12;
    const bool spaced_rows = dimensions.height >= heading_height + 18;
    int row_index = 0;
    auto stat = [&](std::string icon, std::string label, std::string value) {
        return hbox({text("  "), text(icon + " ") | bold | color(tone),
            text(std::move(label)) | bold | color(muted) | size(WIDTH, EQUAL, 12),
            filler(), text(std::move(value)) | bold | color(Color::RGB(255, 255, 255)), text("  ")})
            | bgcolor(row_index++ % 2 ? selected_surface : background);
    };
    Elements stats_lines{
        stat("▥", "Turnos:", std::to_string(observation.turn)),
        stat("ϟ", "Energía:", std::to_string(observation.energy) + " / " +
            std::to_string(observation.maximumEnergy)),
        stat("★", "Puntaje:", std::to_string(observation.score)),
        stat("◇", "Recursos:", std::to_string(observation.collectedResources))};
    if (spaced_rows)
        for (int i = 3; i > 0; --i)
            stats_lines.insert(stats_lines.begin() + i, text(""));
    const int stats_width = std::max(1, std::min(53, content_width - 4));
    auto stats = window(text(" FINAL STATS ") | bold | color(tone) | hcenter |
                            size(WIDTH, EQUAL, stats_width),
                        vbox(std::move(stats_lines)) | size(WIDTH, EQUAL, stats_width), ROUNDED)
        | color(tone) | bgcolor(background) | hcenter;
    auto key = [&](std::string label) {
        Element hint = text("[ " + label + " ]") | bold | color(accent);
        return boxed_keys ? hint | borderStyled(ROUNDED, accent) : hint;
    };
    Elements hints;
    if (done) {
        hints.push_back(key("ENTER"));
        hints.push_back(text(" Main Menu   |   ") | color(ink) | vcenter);
    }
    hints.push_back(key("Q"));
    hints.push_back(text(" Exit") | color(ink) | vcenter);
    const int free_rows = std::max(0, dimensions.height - 2 - heading_height - 1 -
        (spaced_rows ? 9 : 6) - (boxed_keys ? 3 : 1));
    const int stats_gap = std::min(2, (free_rows + 2) / 3);
    const int controls_gap = std::min(1, free_rows - stats_gap);
    const int top_margin = std::min(3, (free_rows - stats_gap - controls_gap) / 2);
    auto body = vbox({text(" " + title) | bold | color(ink),
        separator() | color(edge),
        text("") | size(HEIGHT, EQUAL, top_margin),
        heading | hcenter,
        text(std::move(description)) | bold | color(ink) | hcenter,
        text("") | size(HEIGHT, EQUAL, stats_gap), stats,
        text("") | size(HEIGHT, EQUAL, controls_gap),
        hbox(std::move(hints)) | hcenter, filler()});
    return body | size(WIDTH, EQUAL, content_width)
        | size(HEIGHT, EQUAL, dimensions.height)
        | borderStyled(ROUNDED, edge) | bgcolor(background) | center | bgcolor(background);
}
}
