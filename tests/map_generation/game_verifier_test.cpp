#include "circuit_escape/map_generation/game_verifier.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

GraphData graph_of(std::size_t rows, std::size_t columns) {
    GraphData result;
    result.rows = rows;
    result.columns = columns;
    result.neighbors.resize(rows * columns);
    result.blocked.resize(rows * columns, false);
    result.entry_costs.resize(rows * columns, 1);
    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t column = 0; column < columns; ++column) {
            const std::size_t vertex = row * columns + column;
            if (row > 0) result.neighbors[vertex].push_back(vertex - columns);
            if (column > 0) result.neighbors[vertex].push_back(vertex - 1);
            if (column + 1 < columns) result.neighbors[vertex].push_back(vertex + 1);
            if (row + 1 < rows) result.neighbors[vertex].push_back(vertex + columns);
        }
    }
    return result;
}

MapValidationInput isolated_guard_map() {
    MapValidationInput result{
        graph_of(3, 5), {0, 0}, {2, 4}, {0, 1}, {0, 2}, {0, 3}, 10, 10, 20, false
    };
    for (std::size_t column = 0; column < 5; ++column) {
        result.graph.blocked[5 + column] = true;
    }
    return result;
}

void full_win_and_reproducibility_test() {
    GameVerifier game(isolated_guard_map());
    const std::vector<Action> actions{Action::right, Action::right, Action::right};
    const GameVerification result = game.replay(actions);
    assert(result.outcome == GameOutcome::victory);
    assert(result.history.size() == 4);
    assert(result.history.back().energy == 7);
    assert(result.history.back().turn == 3);
    assert(result.history[1].switch_a_active);
    assert(result.history[2].switch_b_active);
    const auto witness = game.find_winning_game();
    assert(witness);
    assert(witness->outcome == GameOutcome::victory);
    assert(witness->actions == actions);
    assert(game.replay(witness->actions).outcome == GameOutcome::victory);
}

void exit_requires_both_switches_test() {
    const auto input = isolated_guard_map();
    GameVerifier game(input);
    const auto before_switches = game.replay({Action::right, Action::right, Action::right});
    assert(before_switches.outcome == GameOutcome::victory);
    auto altered = input;
    altered.switch_a = {0, 4};
    altered.switch_b = {0, 2};
    GameVerifier alternate(altered);
    const auto early_exit = alternate.replay({Action::right, Action::right, Action::right});
    assert(early_exit.outcome == GameOutcome::in_progress);
    assert(!early_exit.history.back().switch_a_active);
}

void guard_even_turn_capture_test() {
    auto input = isolated_guard_map();
    input.graph = graph_of(1, 5);
    input.guard_start = {0, 4};
    GameVerifier game(input);
    GameState state = game.initial_state();
    state = game.step(state, Action::right);
    assert((state.guard == Position{0, 4}));
    state = game.step(state, Action::right);
    assert((state.guard == Position{0, 3}));
    state = game.step(state, Action::right);
    assert(state.outcome == GameOutcome::captured);
}

void capture_priority_test() {
    auto input = isolated_guard_map();
    input.graph = graph_of(2, 4);
    input.guard_start = {0, 1};
    input.switch_a = {0, 2};
    input.switch_b = {0, 3};
    input.exit = {1, 3};
    input.initial_energy = 1;
    input.maximum_energy = 1;
    GameVerifier game(input);
    const auto result = game.replay({Action::right});
    assert(result.outcome == GameOutcome::captured);
}

void battery_and_trap_test() {
    auto input = isolated_guard_map();
    input.switch_b = {0, 3};
    input.exit = {0, 4};
    input.initial_energy = 4;
    input.maximum_energy = 5;
    input.has_batteries = true;
    GameVerifier game(input, {{Position{0, 2}}, {Position{0, 3}}, 3, 2, 1});
    const auto result = game.replay({Action::right, Action::right, Action::right, Action::right});
    assert(result.outcome == GameOutcome::victory);
    assert(result.history[2].energy == 5);
    assert(result.history[3].energy == 2);
    assert(result.history.back().energy == 1);
    const auto witness = game.find_winning_game();
    assert(witness && witness->outcome == GameOutcome::victory);
    GameVerifier without_battery(input, {{}, {Position{0, 3}}, 3, 2, 1});
    assert(without_battery.replay({Action::right, Action::right, Action::right, Action::right}).outcome == GameOutcome::energy_depleted);
}

void limit_and_wait_test() {
    auto input = isolated_guard_map();
    input.turn_limit = 3;
    GameVerifier game(input);
    assert(game.replay({Action::wait, Action::right, Action::right}).outcome == GameOutcome::turn_limit);
    assert(game.replay({Action::right, Action::right, Action::right}).outcome == GameOutcome::victory);
    auto low_energy = input;
    low_energy.initial_energy = 1;
    GameVerifier second(low_energy);
    assert(second.replay({Action::wait}).outcome == GameOutcome::energy_depleted);
    auto invalid = input;
    invalid.turn_limit = 2;
    GameVerifier third(invalid);
    assert(third.replay({Action::up, Action::up}).outcome == GameOutcome::turn_limit);
}

void blocked_and_validation_test() {
    auto input = isolated_guard_map();
    GameVerifier game(input);
    auto result = game.replay({Action::down, Action::right});
    assert(result.history[1].energy == 9);
    assert((result.history[1].player == Position{0, 0}));
    auto impossible = input;
    impossible.graph.blocked[1] = true;
    bool rejected = false;
    try { (void)GameVerifier(impossible); }
    catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
    auto effects = GameEffects{};
    effects.batteries = {{1, 0}};
    rejected = false;
    try { (void)GameVerifier(input, effects); }
    catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
}

void different_sizes_test() {
    for (auto [rows, columns] : {
        std::pair<std::size_t, std::size_t>{10, 15}, {20, 30}, {40, 60}
    }) {
        auto input = isolated_guard_map();
        input.graph = graph_of(rows, columns);
        input.guard_start = {rows - 1, columns - 1};
        const GameVerifier game(input);
        const auto result = game.replay({Action::right, Action::right, Action::right});
        assert(result.outcome == GameOutcome::victory);
    }
}


void forced_battery_detour_test() {
    auto input = isolated_guard_map();
    input.graph = graph_of(3, 7);
    input.guard_start = {2, 6};
    input.switch_b = {0, 4};
    input.exit = {0, 6};
    input.initial_energy = 5;
    input.maximum_energy = 10;
    input.has_batteries = true;
    for (std::size_t column = 0; column < 6; ++column) {
        input.graph.blocked[14 + column] = true;
    }
    input.graph.blocked[13] = true;
    const GameVerifier game(input, {{Position{1, 2}}, {}, 6, 0, 1});
    const auto direct = game.replay({Action::right, Action::right, Action::right,
        Action::right, Action::right, Action::right});
    assert(direct.outcome == GameOutcome::energy_depleted);
    const auto witness = game.find_winning_game(50'000);
    assert(witness);
    assert(witness->outcome == GameOutcome::victory);
    assert(witness->actions.size() > 6);
    assert(game.replay(witness->actions).outcome == GameOutcome::victory);
}

void guard_tie_break_test() {
    auto input = isolated_guard_map();
    input.graph = graph_of(3, 3);
    input.player_start = {0, 0};
    input.guard_start = {1, 1};
    input.switch_a = {0, 2};
    input.switch_b = {2, 0};
    input.exit = {2, 2};
    for (auto& neighbors : input.graph.neighbors) {
        std::reverse(neighbors.begin(), neighbors.end());
    }
    GameVerifier game(input);
    auto result = game.replay({Action::wait, Action::wait});
    assert(result.outcome == GameOutcome::in_progress);
    assert((result.history.back().guard == Position{0, 1}));
}

void bounded_search_test() {
    const GameVerifier game(isolated_guard_map());
    assert(!game.find_winning_game(0));
    assert(game.find_winning_game(10));
    auto impossible = isolated_guard_map();
    impossible.initial_energy = 1;
    const GameVerifier no_win(impossible);
    assert(!no_win.find_winning_game(1'000));
}

}

void run_game_verifier_tests() {
    full_win_and_reproducibility_test();
    exit_requires_both_switches_test();
    guard_even_turn_capture_test();
    capture_priority_test();
    battery_and_trap_test();
    limit_and_wait_test();
    blocked_and_validation_test();
    different_sizes_test();
    bounded_search_test();
    forced_battery_detour_test();
    guard_tie_break_test();
}
