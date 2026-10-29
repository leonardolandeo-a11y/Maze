#include "circuit_escape/project2_application.h"

#include "circuit_escape/project2_session.h"
#include "circuit_escape/StartupAnimation.h"
#include "circuit_escape/VictoryAnimation.h"
#include "circuit_escape/DeathAnimation.h"
#include "circuit_escape/menus.h"
#include "circuit_escape/console_ui.h"
#include "circuit_escape/project2_help_text.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <thread>
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

ftxui::Element help_content(const Project2Session& session, const HelpReport& report,
    int highlight_mode, const ConsoleUI& ui, Project2MapView view) {
    const std::vector<Position>* path = &report.complete_path;
    if (highlight_mode == 0) {
        static const std::vector<Position> empty;
        path = &empty;
    } else if (highlight_mode == 1) {
        path = &report.path_to_a;
    } else if (highlight_mode == 2) {
        path = &report.path_to_b;
    }
    return ui.render_project2(session, view, *path, format_project2_help_lines(report));
}

Observation animation_observation(const Project2Session& session) {
    return session.observation();
}

EndReason end_reason(GameOutcome outcome) {
    switch (outcome) {
        case GameOutcome::captured: return EndReason::captured;
        case GameOutcome::energy_depleted: return EndReason::noEnergy;
        case GameOutcome::turn_limit: return EndReason::turnLimit;
        case GameOutcome::victory: return EndReason::goalReached;
        case GameOutcome::in_progress: return EndReason::none;
    }
    return EndReason::none;
}

SavedDifficulty saved_difficulty(Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::easy: return SavedDifficulty::easy;
        case Difficulty::standard: return SavedDifficulty::medium;
        case Difficulty::hard: return SavedDifficulty::hard;
    }
    return SavedDifficulty::easy;
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
    bool menu = !difficulty.has_value();
    bool show_help = false;
    Project2MapView map_view = Project2MapView::overview;
    int highlight_mode = 3;
    std::string error;
    std::unique_ptr<Project2Session> session;
    std::optional<HelpReport> report;
    Menus menus(true);
    ConsoleUI ui(RenderMode::emoji);
    const StartupAnimation startup_animation;
    const VictoryAnimation victory_animation;
    const DeathAnimation death_animation;
    std::atomic<bool> startup_active{true};
    std::atomic<bool> victory_active{false};
    std::atomic<bool> death_active{false};
    std::atomic<bool> running{true};
    int startup_frame = 0;
    int victory_frame = 0;
    int death_frame = 0;

    auto load_difficulty = [&](SavedDifficulty value) {
        try {
            SavedMap map = load_map(map_file(maps_directory_, value));
            if (map.difficulty != value) {
                throw std::runtime_error("Map difficulty does not match the selected level");
            }
            session = std::make_unique<Project2Session>(std::move(map));
            ui = ConsoleUI(menus.selectedRenderMode());
            show_help = false;
            map_view = Project2MapView::overview;
            report.reset();
            error.clear();
            menu = false;
            victory_active = false;
            death_active = false;
            victory_frame = 0;
            death_frame = 0;
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
        if (startup_active) {
            return startup_animation.RenderStartupFrame(startup_frame);
        }
        if (menu || !session) {
            return vbox({
                menus.Render(),
                error.empty() ? text("") : text(error) | color(Color::RedLight)
            }) | bgcolor(Color::Black);
        }
        if (victory_active) {
            return victory_animation.RenderVictoryFrame(victory_frame, animation_observation(*session));
        }
        if (death_active) {
            return death_animation.RenderDeathFrame(death_frame,
                end_reason(session->state().outcome), animation_observation(*session));
        }
        Element content = show_help && report
            ? help_content(*session, *report, highlight_mode, ui, map_view)
            : ui.render_project2(*session, map_view);
        return vbox({filler(), hbox({filler(), std::move(content), filler()}), filler()})
            | bgcolor(Color::Black);
    });

    Component component = CatchEvent(renderer, [&](const Event& event) {
        if (startup_active) {
            if (event == Event::Character('q') || event == Event::Character('Q')) {
                screen.Exit();
            } else if (event == Event::Return) {
                startup_active = false;
            }
            return true;
        }
        if (event == Event::Character('q') || event == Event::Character('Q')) {
            screen.Exit();
            return true;
        }
        if (menu || !session) {
            const bool handled = menus.OnEvent(event);
            switch (menus.takeCommand()) {
                case MenuCommand::startGame:
                    load_difficulty(saved_difficulty(menus.selectedDifficulty()));
                    return true;
                case MenuCommand::quit:
                    screen.Exit();
                    return true;
                case MenuCommand::none:
                    break;
            }
            return handled;
        }
        if (victory_active || death_active) {
            const bool animation_complete = victory_active
                ? victory_frame >= VictoryAnimation::maxFrames
                : death_frame >= DeathAnimation::maxFrames;
            if (animation_complete &&
                (event == Event::Return || event == Event::Character('r') || event == Event::Character('R'))) {
                victory_active = false;
                death_active = false;
                menu = true;
                session.reset();
            }
            return true;
        }
        if (show_help) {
            if (event == Event::Character('z') || event == Event::Character('Z')) {
                map_view = map_view == Project2MapView::overview
                    ? Project2MapView::detail : Project2MapView::overview;
                return true;
            }
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
        if (session->state().outcome != GameOutcome::in_progress) return true;
        if (event == Event::Character('z') || event == Event::Character('Z')) {
            map_view = map_view == Project2MapView::overview
                ? Project2MapView::detail : Project2MapView::overview;
            return true;
        }
        if (event == Event::Character('h') || event == Event::Character('H')) {
            report = session->help_report();
            highlight_mode = 3;
            show_help = true;
            return true;
        }
        if (const auto action = action_from_event(event)) {
            const GameOutcome outcome = session->step(*action).outcome;
            if (outcome == GameOutcome::victory) {
                victory_frame = 0;
                victory_active = true;
            } else if (outcome != GameOutcome::in_progress) {
                death_frame = 0;
                death_active = true;
            }
            return true;
        }
        return false;
    });

    std::thread animation_thread([&] {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(60));
            if (!running) break;
            // Frame counters are owned by the FTXUI event thread. The timer
            // thread only reads atomic animation flags and posts callbacks.
            if (startup_active) {
                screen.Post([&] {
                    if (startup_active && ++startup_frame >= StartupAnimation::maxFrames)
                        startup_active = false;
                });
            } else if (victory_active) {
                screen.Post([&] {
                    if (victory_active && victory_frame < VictoryAnimation::maxFrames)
                        ++victory_frame;
                });
            } else if (death_active) {
                screen.Post([&] {
                    if (death_active && death_frame < DeathAnimation::maxFrames)
                        ++death_frame;
                });
            } else {
                continue;
            }
            screen.Post(Event::Custom);
        }
    });

    screen.Loop(component);
    running = false;
    animation_thread.join();
    return 0;
}
