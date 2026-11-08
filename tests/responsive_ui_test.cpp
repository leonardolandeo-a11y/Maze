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

    for (auto viewport : viewports) {
        Menus menu(true);
        check(menu.Render(viewport), viewport, "main",
            {"START GAME", "DIFFICULTY", "RENDER MODE", "HELP", "QUIT", "Enter: confirm"});
        for (int page = 1; page <= 3; ++page) {
            Menus settings(true);
            down(settings, page);
            settings.OnEvent(ftxui::Event::Return);
            if (page == 1) check(settings.Render(viewport), viewport, "difficulty",
                {"EASY", "MEDIUM", "HARD", "BACK"});
            if (page == 2) check(settings.Render(viewport), viewport, "render",
                {"EMOJI", "ASCII", "BACK"});
            if (page == 3) check(settings.Render(viewport), viewport, "help",
                {"THE ESCAPE", "CONTROLS", "Wait one turn", "FULL MAP / DETAIL", "Enter: back"});
        }
        menu.OnEvent(ftxui::Event::Return);
        check(menu.Render(viewport), viewport, "sizes",
            {"10 x 15", "20 x 30", "40 x 60", "CUSTOM", "BACK"});
        down(menu, 3);
        menu.OnEvent(ftxui::Event::Return);
        check(menu.Render(viewport), viewport, "custom",
            {"Rows:", "Columns:", "CELLS", "GENERATE", "BACK", "Esc: back"});

        StartupAnimation startup;
        for (int frame : {0, 18, StartupAnimation::maxFrames - 1})
            check(startup.RenderStartupFrame(frame, viewport), viewport,
                "startup-" + std::to_string(frame), {"CIRCUITO DE ESCAPE", "Enter: continue"});
        MapGenerationAnimation loading;
        check(loading.RenderFrame(10, {20,30}, viewport), viewport, "loading",
            {"PREPARING THE ESCAPE", "GENERATING 20 x 30", "PLEASE WAIT", "Q: quit"});
        VictoryAnimation victory;
        check(victory.RenderVictoryFrame(VictoryAnimation::maxFrames - 1, observation, viewport),
              viewport, "victory", {"VICTORY", "RUN SUMMARY", "Energy", "Resources",
                                    "ENTER - MAIN MENU", "Q - EXIT"});
        DeathAnimation death;
        check(death.RenderDeathFrame(DeathAnimation::maxFrames - 1, EndReason::captured,
              observation, viewport), viewport, "death",
              {"GAME OVER", "CAPTURED BY THE GUARD", "RUN SUMMARY", "ENTER - MAIN MENU"});

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
                    "Route: CURRENT", "Remaining", "H/Esc back", "Z zoom"});
        }
        assert(session.state().turn == 0);
        assert(session.state().energy == map.input.initial_energy);
    }
}
