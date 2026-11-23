#include "circuit_escape/menus.h"
#include "circuit_escape/StartupAnimation.h"
#include "circuit_escape/MapGenerationAnimation.h"
#include "circuit_escape/VictoryAnimation.h"
#include "circuit_escape/DeathAnimation.h"
#include "circuit_escape/project2_session.h"
#include "circuit_escape/project2_help_text.h"

#include <ftxui/screen/screen.hpp>
#include <array>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace {
void check(ftxui::Element element, ftxui::Dimensions viewport, const std::string& name,
           std::initializer_list<const char*> visible) {
    element->ComputeRequirement();
    assert(element->requirement().min_x <= viewport.dimx);
    assert(element->requirement().min_y <= viewport.dimy);
    auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(viewport.dimx),
                                       ftxui::Dimension::Fixed(viewport.dimy));
    ftxui::Render(screen, std::move(element));
    std::string text;
    for (int y = 0; y < viewport.dimy; ++y) {
        for (int x = 0; x < viewport.dimx; ++x) {
            const auto& cell = screen.PixelAt(x, y);
            text += cell.character.empty() ? " " : cell.character;
        }
        text += '\n';
    }
    if (name == "main" || name == "legacy-main" || name == "death" || name == "victory") {
        assert(screen.PixelAt(0, 0).character == "╭");
        assert(screen.PixelAt(viewport.dimx - 1, viewport.dimy - 1).character == "╯");
    }
    if (name == "death" || name == "victory") {
        assert(text.find("FINAL STATS") != std::string::npos);
        assert(text.find("Main Menu") != std::string::npos);
        for (const auto* icon : {"▥", "ϟ", "★", "◇"})
            assert(text.find(icon) != std::string::npos);
    }
    if (name == "main" || name == "legacy-main") {
        int borders = 0;
        for (int y = 0; y < viewport.dimy; ++y)
            for (int x = 0; x < viewport.dimx; ++x)
                borders += screen.PixelAt(x, y).character == "╭";
        assert(borders >= (name == "main" ? 6 : 7));
        assert(text.find("CIRCUITO DE ESCAPE") == std::string::npos);
        for (const auto* detail : {"✚", "▥", "▣", "?", "↪", "Choose your challenge",
                                   "Choose map symbols", "Learn the circuit", "Leave the circuit"})
            assert(text.find(detail) != std::string::npos);
    }
    if (name == "render-visual" || name == "render-ascii" || name == "preview-render")
        assert(text.find('@') == std::string::npos);
    if (ftxui::Terminal::ColorSupport() == ftxui::Terminal::Color::TrueColor) {
        auto has_color = [&](ftxui::Color tone) {
            for (int y = 0; y < viewport.dimy; ++y)
                for (int x = 0; x < viewport.dimx; ++x) {
                    const auto& cell = screen.PixelAt(x, y);
                    if (cell.foreground_color == tone || cell.background_color == tone) return true;
                }
            return false;
        };
        const std::array fills{ftxui::Color::RGB(84,235,121), ftxui::Color::RGB(255,216,77),
                                ftxui::Color::RGB(255,89,100)};
        if (name == "preview-difficulty") {
            assert(has_color(ftxui::Color::RGB(156,163,175)));
            for (auto tone : fills) assert(!has_color(tone));
        }
        if (name.starts_with("difficulty-face-"))
            for (int level = 0; level < 3; ++level)
                assert(has_color(fills[level]) == (name.back() - '0' == level));
        if (name == "preview-render") {
            assert(has_color(ftxui::Color::RGB(184,192,204)));
            assert(!has_color(ftxui::Color::RGB(255,205,165)));
            assert(!has_color(ftxui::Color::BlueLight));
        }
        if (name == "render-visual") {
            assert(has_color(ftxui::Color::RGB(255,205,165)));
            assert(has_color(ftxui::Color::BlueLight));
        }
        if (name == "render-ascii")
            for (auto tone : {ftxui::Color::RGB(198,205,213), ftxui::Color::RGB(228,232,237),
                              ftxui::Color::RGB(139,150,163), ftxui::Color::RGB(181,190,200)})
                assert(has_color(tone));
    }
    if (name == "main" || name.starts_with("prepared-")) {
        for (const auto* obsolete : {"SELECT MAP SIZE", "CUSTOM MAP SIZE",
                                    "GENERATING", "NOT PREPARED"})
            assert(text.find(obsolete) == std::string::npos);
    }
    // Optional review artifacts are actual FTXUI frames, not separate mockups.
    if (const char* directory = std::getenv("MAZE_UI_PREVIEW_DIR")) {
        const auto path = std::filesystem::path(directory) /
            (name + "-" + std::to_string(viewport.dimx) + "x" + std::to_string(viewport.dimy));
        std::filesystem::create_directories(directory);
        std::ofstream(path.string() + ".txt") << text;
        std::ofstream(path.string() + ".ansi") << screen.ToString();
    }
    for (const auto* expected : visible) {
        if (text.find(expected) == std::string::npos) {
            std::cerr << name << " " << viewport.dimx << "x" << viewport.dimy
                      << ": missing " << expected << '\n' << text;
            assert(false && "Required content clipped or missing");
        }
    }
}
void down(Menus& menu, int count) {
    while (count--) menu.OnEvent(ftxui::Event::ArrowDown);
}
}

void run_responsive_ui_tests() {
    const std::array viewports{ftxui::Dimensions{80,24}, ftxui::Dimensions{100,30},
        ftxui::Dimensions{120,35}, ftxui::Dimensions{160,45}};
    const auto map = load_map(std::filesystem::path(MAZE_DEFAULT_MAPS_DIRECTORY) / "mapa_facil.txt");
    Project2Session session(map);
    const auto observation = session.observation();
    ConsoleUI marker_ui(RenderMode::ascii);
    check(marker_ui.render_project2_board(session, {}, Project2MapView::overview, 60, 4),
        {60, 4}, "sampled-markers", {"@", "V", "A", "B", "S"});

    for (auto viewport : {ftxui::Dimensions{60,24}, ftxui::Dimensions{70,24}, ftxui::Dimensions{80,20}}) {
        Menus menu(true);
        down(menu, 3);
        menu.OnEvent(ftxui::Event::Return);
        check(menu.Render(viewport), viewport, "small-help",
            {"THE ESCAPE", "CONTROLS", "Z · Alternar", "FULL MAP", "DETAIL", "Enter: back"});
        ConsoleUI ui(RenderMode::ascii);
        for (int energy : {100, 51, 1}) {
            auto report = session.help_report();
            report.current_energy = energy;
            const auto lines = format_project2_help_lines(report);
            for (auto view : {Project2MapView::overview, Project2MapView::detail})
                check(ui.render_project2(session, view, {}, lines, viewport), viewport,
                    "small-panel-" + std::to_string(energy),
                    {"DIJKSTRA ENERGY HELP", "OBJECTIVES", "ROUTE", "ENERGY", "CONTROLS",
                     "Remaining", "H/Esc back", "Z · Alternar", "FULL MAP", "DETAIL"});
        }
    }

    for (auto viewport : {ftxui::Dimensions{60,20}, {80,20}}) {
        Menus menu(true);
        down(menu, 4);
        check(menu.Render(viewport), viewport, "small-main",
            {"QUIT", "Two switches. One way out.", "Enter: confirm", "Q: quit"});
        check(VictoryAnimation{}.RenderVictoryFrame(VictoryAnimation::maxFrames - 1,
            observation, viewport), viewport, "small-victory",
            {"ESCAPE", "COMPLETED", "¡Has escapado del circuito!", "Energía:", "[ ENTER ]"});
        check(DeathAnimation{}.RenderDeathFrame(DeathAnimation::maxFrames - 1,
            EndReason::captured, observation, viewport), viewport, "small-death",
            {"GAME OVER", "CAPTURED BY THE GUARD", "Puntaje:", "[ ENTER ]"});
    }

    for (auto viewport : viewports) {
        Menus menu(true);
        check(menu.Render(viewport), viewport, "main",
            {"START GAME", "DIFFICULTY", "RENDER MODE", "HELP", "QUIT", "Enter: confirm",
             "Two switches. One way out.", "╚═╝"});
        Menus legacy;
        check(legacy.Render(viewport), viewport, "legacy-main",
            {"START GAME", "SCENARIO", "DIFFICULTY", "RENDER MODE", "HELP", "QUIT", "Enter: confirm"});
        for (int page = 1; page <= 3; ++page) {
            Menus settings(true);
            down(settings, page);
            if (page == 1) check(settings.Render(viewport), viewport, "preview-difficulty",
                {"EASY", "MEDIUM", "HARD", "▀"});
            if (page == 2) check(settings.Render(viewport), viewport, "preview-render",
                {"EMOJI", "ASCII", "▀"});
            if (page == 3) check(settings.Render(viewport), viewport, "preview-help", {"█████"});
            settings.OnEvent(ftxui::Event::Return);
            if (page == 1) {
                for (int level = 0; level < 3; ++level) {
                    check(settings.Render(viewport), viewport, "difficulty-face-" + std::to_string(level),
                        {"▀", "BACK"});
                    assert(settings.selectedDifficulty() == Difficulty::easy);
                    settings.OnEvent(ftxui::Event::ArrowDown);
                }
            }
            if (page == 2) {
                check(settings.Render(viewport), viewport, "render-visual", {"EMOJI", "ASCII", "BACK", "▀"});
                settings.OnEvent(ftxui::Event::ArrowDown);
                check(settings.Render(viewport), viewport, "render-ascii", {"ASCII", "BACK", "▀"});
                assert(settings.selectedRenderMode() == RenderMode::emoji);
            }
            if (page == 3) check(settings.Render(viewport), viewport, "help",
                {"THE ESCAPE", "CONTROLS", "Wait one turn", "FULL MAP", "DETAIL", "Enter: back"});
        }
        // The responsive menu starts an existing map at every viewport.
        // No size form or generation command can intervene.
        for (int level = 0; level < 3; ++level) {
            Menus prepared(true);
            down(prepared, 1);
            prepared.OnEvent(ftxui::Event::Return);
            down(prepared, level);
            prepared.OnEvent(ftxui::Event::Return);
            assert(prepared.takeCommand() == MenuCommand::none);
            assert(prepared.selectedDifficulty() == (level == 0 ? Difficulty::easy :
                level == 1 ? Difficulty::standard : Difficulty::hard));
            check(prepared.Render(viewport), viewport, "prepared-" + std::to_string(level),
                {"MAIN MENU", "START GAME", "DIFFICULTY", "RENDER MODE", "HELP", "QUIT"});
            prepared.OnEvent(ftxui::Event::Return);
            assert(prepared.takeCommand() == MenuCommand::startGame);
            assert(prepared.takeCommand() == MenuCommand::none);
        }

        StartupAnimation startup;
        for (int frame : {0, 18, StartupAnimation::maxFrames - 1})
            check(startup.RenderStartupFrame(frame, viewport), viewport,
                "startup-" + std::to_string(frame), {"CIRCUITO DE ESCAPE", "Enter: continue"});
        // Reusable animation remains tested, but is disconnected from the game.
        MapGenerationAnimation loading;
        check(loading.RenderFrame(10, {20,30}, viewport), viewport, "loading",
            {"PREPARING THE ESCAPE", "GENERATING 20 x 30", "PLEASE WAIT", "Q: quit"});
        VictoryAnimation victory;
        check(victory.RenderVictoryFrame(VictoryAnimation::maxFrames - 1, observation, viewport),
              viewport, "victory", {"ESCAPE COMPLETED", "¡Has escapado del circuito!", "Energía:", "Recursos:",
                                    "[ ENTER ]", "[ Q ]"});
        DeathAnimation death;
        check(death.RenderDeathFrame(DeathAnimation::maxFrames - 1, EndReason::captured,
              observation, viewport), viewport, "death",
              {"GAME OVER", "CAPTURED BY THE GUARD", "Puntaje:", "[ ENTER ]"});

        ConsoleUI ui(RenderMode::ascii);
        const auto help = session.help_report();
        const auto help_lines = format_project2_help_lines(help);
        for (const auto view : {Project2MapView::overview, Project2MapView::detail}) {
            const std::string name = view == Project2MapView::overview ? "overview" : "detail";
            check(ui.render_project2(session, view, {}, {}, viewport), viewport, name,
                {"STATUS", "Turn 0/", "Energy", "Score 0", "Resources 0/", "MAZE",
                 "EVENTS / CONTROLS", "Help: H", "Quit: Q"});
            check(ui.render_project2(session, view, help.complete_path, help_lines, viewport),
                viewport, name + "-help", {"STATUS", "MAZE", "DIJKSTRA ENERGY HELP",
                    "Route: CURRENT", "Remaining", "H/Esc back", "Alternar entre mapa", "FULL MAP", "DETAIL"});
        }
        assert(session.state().turn == 0);
        assert(session.state().energy == map.input.initial_energy);
    }
}
