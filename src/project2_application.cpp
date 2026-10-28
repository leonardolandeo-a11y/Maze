#include "circuit_escape/project2_application.h"

#include "circuit_escape/project2_session.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <algorithm>
#include <array>
#include <memory>
#include <stdexcept>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace {

std::filesystem::path map_file(const std::filesystem::path& directory, SavedDifficulty difficulty) {
    switch (difficulty) {
        case SavedDifficulty::easy: return directory / "mapa_facil.txt";
        case SavedDifficulty::medium: return directory / "mapa_medio.txt";
        case SavedDifficulty::hard: return directory / "mapa_dificil.txt";
    }
    return directory / "mapa_facil.txt";
}

std::string difficulty_label(SavedDifficulty difficulty) {
    switch (difficulty) {
        case SavedDifficulty::easy: return "EASY";
        case SavedDifficulty::medium: return "MEDIUM";
        case SavedDifficulty::hard: return "HARD";
    }
    return "EASY";
}

std::string outcome_label(GameOutcome outcome) {
    switch (outcome) {
        case GameOutcome::victory: return "VICTORY";
        case GameOutcome::captured: return "CAPTURED BY THE GUARD";
        case GameOutcome::energy_depleted: return "OUT OF ENERGY";
        case GameOutcome::turn_limit: return "TURN LIMIT REACHED";
        case GameOutcome::in_progress: return "IN PROGRESS";
    }
    return "IN PROGRESS";
}

std::string target_label(HelpTarget target) {
    switch (target) {
        case HelpTarget::switch_a: return "A";
        case HelpTarget::switch_b: return "B";
        case HelpTarget::exit: return "EXIT";
    }
    return "EXIT";
}

std::string status_line(const char* label, const ObjectiveInfo& info) {
    switch (info.status) {
        case ObjectiveStatus::activated: return std::string(label) + ": activated";
        case ObjectiveStatus::locked: return std::string(label) + ": locked";
        case ObjectiveStatus::enabled: break;
        case ObjectiveStatus::pending: break;
    }
    if (!info.cost_from_current) return std::string(label) + ": unreachable";
    return std::string(label) + ": " + std::to_string(*info.cost_from_current) + " energy";
}

bool on_path(Position position, std::span<const Position> path) {
    return std::find(path.begin(), path.end(), position) != path.end();
}

ftxui::Element draw_board(const Project2Session& session, std::span<const Position> highlighted) {
    using namespace ftxui;
    const SavedMap& map = session.map();
    const GameState& state = session.state();
    const std::size_t view_rows = std::min<std::size_t>(map.grid.rows(), 17);
    const std::size_t view_columns = std::min<std::size_t>(map.grid.columns(), 30);
    const std::size_t first_row = std::min(
        state.player.row > view_rows / 2 ? state.player.row - view_rows / 2 : 0,
        map.grid.rows() - view_rows
    );
    const std::size_t first_column = std::min(
        state.player.column > view_columns / 2 ? state.player.column - view_columns / 2 : 0,
        map.grid.columns() - view_columns
    );
    Elements rows;
    for (std::size_t row = first_row; row < first_row + view_rows; ++row) {
        Elements columns;
        for (std::size_t column = first_column; column < first_column + view_columns; ++column) {
            const Position position{row, column};
            const ::Cell& cell = map.grid.at(position);
            char symbol = '.';
            Color foreground = Color::GrayLight;
            if (std::holds_alternative<Wall>(cell)) {
                symbol = '#'; foreground = Color::GrayDark;
            } else if (std::holds_alternative<RoughTerrain>(cell)) {
                symbol = '~'; foreground = Color::Yellow;
            } else if (std::holds_alternative<Exit>(cell)) {
                symbol = 'S'; foreground = Color::Green;
            } else if (std::holds_alternative<Battery>(cell) && !session.battery_consumed(position)) {
                symbol = '+'; foreground = Color::GreenLight;
            } else if (std::holds_alternative<Trap>(cell)) {
                symbol = '^'; foreground = Color::Red;
            }
            if (position == map.input.switch_a) {
                symbol = state.switch_a_active ? 'a' : 'A';
                foreground = Color::YellowLight;
            } else if (position == map.input.switch_b) {
                symbol = state.switch_b_active ? 'b' : 'B';
                foreground = Color::YellowLight;
            }
            if (position == state.guard) {
                symbol = 'V'; foreground = Color::RedLight;
            }
            if (position == state.player) {
                symbol = '@'; foreground = Color::CyanLight;
            }
            Element visual = text(std::string(1, symbol) + ' ') | color(foreground);
            if (on_path(position, highlighted)) visual = visual | bgcolor(Color::Blue);
            columns.push_back(std::move(visual));
        }
        rows.push_back(hbox(std::move(columns)));
    }
    return vbox({
        text("View rows " + std::to_string(first_row) + "-" +
            std::to_string(first_row + view_rows - 1) + ", columns " +
            std::to_string(first_column) + "-" +
            std::to_string(first_column + view_columns - 1)),
        vbox(std::move(rows))
    }) | border;
}

ftxui::Element help_content(const Project2Session& session, const HelpReport& report, int highlight_mode) {
    using namespace ftxui;
    const std::vector<Position>* path = &report.complete_path;
    if (highlight_mode == 0) {
        static const std::vector<Position> empty;
        path = &empty;
    } else if (highlight_mode == 1) {
        path = &report.path_to_a;
    } else if (highlight_mode == 2) {
        path = &report.path_to_b;
    }
    std::string recommendation = "No complete route available";
    if (report.total_cost) {
        recommendation = "Route: CURRENT";
        for (HelpTarget target : report.recommended_order) {
            recommendation += " -> " + target_label(target);
        }
        recommendation += " | Cost " + std::to_string(*report.total_cost);
        recommendation += " | Remaining " + std::to_string(report.current_energy - *report.total_cost);
        recommendation += report.current_energy > *report.total_cost
            ? " | ENOUGH WITHOUT EXTRA COSTS" : " | NOT ENOUGH WITHOUT RECHARGES";
    }
    return vbox({
        text("DIJKSTRA ENERGY HELP") | bold | center | color(Color::CyanLight),
        text(status_line("A", report.switch_a)),
        text(status_line("B", report.switch_b)),
        text(status_line("Exit", report.exit_goal)),
        text(recommendation),
        text("Estimate excludes guard movement, battery detours and waiting"),
        text("Highlight: 1=A  2=B  3=full route  0=off"),
        text("H or Esc: return to game (no turn consumed)"),
        draw_board(session, *path)
    }) | border;
}

ftxui::Element game_content(const Project2Session& session) {
    using namespace ftxui;
    const GameState& state = session.state();
    const SavedMap& map = session.map();
    const std::string header = difficulty_label(map.difficulty) +
        " | " + std::to_string(map.grid.rows()) + "x" + std::to_string(map.grid.columns()) +
        " | Turn " + std::to_string(state.turn) + "/" + std::to_string(map.input.turn_limit) +
        " | Energy " + std::to_string(state.energy) + "/" + std::to_string(map.input.maximum_energy);
    const std::string switches = std::string("Switch A: ") + (state.switch_a_active ? "ON" : "OFF") +
        " | Switch B: " + (state.switch_b_active ? "ON" : "OFF") +
        " | Guard moves on " + (state.turn % 2 == 0 ? "NEXT turn" : "the turn after next") +
        " | Guard (" + std::to_string(state.guard.row) + "," + std::to_string(state.guard.column) + ")";
    return vbox({
        text("CIRCUIT ESCAPE - PROJECT 2") | bold | color(Color::CyanLight),
        text(header),
        text(switches),
        text("@ player | V guard | A/B switches | S exit | + battery | ^ trap | ~ rough"),
        draw_board(session, {}),
        text(state.outcome == GameOutcome::in_progress
            ? "WASD/arrows: move | E: wait | H: energy help | Q: quit"
            : outcome_label(state.outcome) + " | R: choose difficulty | Q: quit")
    });
}

ftxui::Element menu_content(int selected, const std::string& error) {
    using namespace ftxui;
    Elements options;
    const std::array<std::string, 3> labels = {"EASY", "MEDIUM", "HARD"};
    for (int index = 0; index < 3; ++index) {
        options.push_back(text((selected == index ? "> " : "  ") + labels[static_cast<std::size_t>(index)]) |
            color(selected == index ? Color::CyanLight : Color::White));
    }
    return vbox({
        text("CIRCUIT ESCAPE - PROJECT 2") | bold | center | color(Color::CyanLight),
        text("Select one of the generated maps"),
        vbox(std::move(options)),
        text("Up/Down or W/S: select | Enter: play | Q: quit"),
        text(error) | color(Color::RedLight)
    }) | border | center;
}

std::optional<Action> action_from_event(const ftxui::Event& event) {
    using namespace ftxui;
    if (event == Event::ArrowUp || event == Event::Character('w') || event == Event::Character('W')) return Action::up;
    if (event == Event::ArrowDown || event == Event::Character('s') || event == Event::Character('S')) return Action::down;
    if (event == Event::ArrowLeft || event == Event::Character('a') || event == Event::Character('A')) return Action::left;
    if (event == Event::ArrowRight || event == Event::Character('d') || event == Event::Character('D')) return Action::right;
    if (event == Event::Character('e') || event == Event::Character('E')) return Action::wait;
    return std::nullopt;
}

}

Project2Application::Project2Application(std::filesystem::path maps_directory)
    : maps_directory_(std::move(maps_directory)) {}

int Project2Application::run(std::optional<SavedDifficulty> difficulty) {
    using namespace ftxui;
    int selected = difficulty ? static_cast<int>(*difficulty) : 0;
    bool menu = !difficulty.has_value();
    bool show_help = false;
    int highlight_mode = 3;
    std::string error;
    std::unique_ptr<Project2Session> session;
    std::optional<HelpReport> report;

    auto load_difficulty = [&](SavedDifficulty value) {
        try {
            SavedMap map = load_map(map_file(maps_directory_, value));
            if (map.difficulty != value) {
                throw std::runtime_error("Map difficulty does not match the selected level");
            }
            session = std::make_unique<Project2Session>(std::move(map));
            show_help = false;
            report.reset();
            error.clear();
            menu = false;
        } catch (const std::exception& exception) {
            error = exception.what();
            session.reset();
            menu = true;
        }
    };

    if (difficulty) {
        load_difficulty(*difficulty);
        if (!session) throw std::runtime_error(error);
    }
    ScreenInteractive screen = ScreenInteractive::Fullscreen();

    Component renderer = Renderer([&] {
        Element content = menu || !session ? menu_content(selected, error)
            : show_help && report ? help_content(*session, *report, highlight_mode)
            : game_content(*session);
        return vbox({filler(), hbox({filler(), std::move(content), filler()}), filler()});
    });

    Component component = CatchEvent(renderer, [&](const Event& event) {
        if (event == Event::Character('q') || event == Event::Character('Q')) {
            screen.Exit();
            return true;
        }
        if (menu || !session) {
            if (event == Event::ArrowDown || event == Event::Character('s') || event == Event::Character('S')) {
                selected = (selected + 1) % 3;
                return true;
            }
            if (event == Event::ArrowUp || event == Event::Character('w') || event == Event::Character('W')) {
                selected = (selected + 2) % 3;
                return true;
            }
            if (event == Event::Return) {
                load_difficulty(static_cast<SavedDifficulty>(selected));
                return true;
            }
            return false;
        }
        if (show_help) {
            if (event == Event::Character('h') || event == Event::Character('H') || event == Event::Escape) {
                show_help = false;
                return true;
            }
            if (event == Event::Character('0')) highlight_mode = 0;
            if (event == Event::Character('1')) highlight_mode = 1;
            if (event == Event::Character('2')) highlight_mode = 2;
            if (event == Event::Character('3')) highlight_mode = 3;
            return true;
        }
        if (event == Event::Character('r') || event == Event::Character('R')) {
            if (session->state().outcome != GameOutcome::in_progress) {
                menu = true;
                session.reset();
            }
            return true;
        }
        if (session->state().outcome != GameOutcome::in_progress) return true;
        if (event == Event::Character('h') || event == Event::Character('H')) {
            report = session->help_report();
            highlight_mode = 3;
            show_help = true;
            return true;
        }
        if (const auto action = action_from_event(event)) {
            (void)session->step(*action);
            return true;
        }
        return false;
    });
    screen.Loop(component);
    return 0;
}
