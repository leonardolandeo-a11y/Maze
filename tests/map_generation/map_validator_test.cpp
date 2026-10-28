#include "circuit_escape/map_generation/map_validator.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <cstdint>
#include <utility>
#include <stdexcept>
#include <vector>

namespace {

GraphData make_graph(std::size_t rows, std::size_t columns) {
    GraphData graph;
    graph.rows = rows;
    graph.columns = columns;
    graph.neighbors.resize(rows * columns);
    graph.blocked.resize(rows * columns, false);
    graph.entry_costs.resize(rows * columns, 1);

    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t column = 0; column < columns; ++column) {
            const std::size_t index = row * columns + column;
            if (row > 0) graph.neighbors[index].push_back(index - columns);
            if (column > 0) graph.neighbors[index].push_back(index - 1);
            if (column + 1 < columns) graph.neighbors[index].push_back(index + 1);
            if (row + 1 < rows) graph.neighbors[index].push_back(index + columns);
        }
    }
    return graph;
}

MapValidationInput base_input() {
    return {
        make_graph(3, 4),
        {0, 0},
        {2, 3},
        {0, 1},
        {0, 2},
        {0, 3},
        10,
        10,
        50,
        false
    };
}

template<typename Callback>
void expect_invalid(Callback callback) {
    bool caught = false;
    try {
        callback();
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);
}

void basic_order_and_path_test() {
    const auto input = base_input();
    const auto result = validate_map_routes(input);
    assert(result.status == MapRouteStatus::ready_for_simulation);
    assert(result.a_then_b && result.b_then_a && result.recommended);
    assert(result.a_then_b->total_energy_cost == 3);
    assert(result.b_then_a->total_energy_cost == 5);
    assert(result.recommended->order == SwitchOrder::a_then_b);
    assert(result.recommended->complete_path.size() == 4);
    assert((result.recommended->complete_path.front() == Position{0, 0}));
    assert((result.recommended->complete_path.back() == Position{0, 3}));
    assert(result.recommended->segments[0].cost == 1);
    assert(result.recommended->segments[1].cost == 1);
    assert(result.recommended->segments[2].cost == 1);
}

void choose_b_first_test() {
    auto input = base_input();
    input.switch_a = {0, 3};
    input.switch_b = {0, 1};
    input.exit = {0, 2};
    const auto result = validate_map_routes(input);
    assert(result.recommended);
    assert(result.recommended->order == SwitchOrder::b_then_a);
    assert(result.b_then_a->total_energy_cost == 4);
    assert(result.a_then_b->total_energy_cost == 6);
}

void deterministic_tie_test() {
    auto input = base_input();
    input.graph = make_graph(3, 5);
    input.player_start = {1, 2};
    input.guard_start = {2, 2};
    input.switch_a = {1, 1};
    input.switch_b = {1, 3};
    input.exit = {0, 2};
    const auto result = validate_map_routes(input);
    assert(result.a_then_b && result.b_then_a);
    assert(result.a_then_b->total_energy_cost == result.b_then_a->total_energy_cost);
    assert(result.recommended->order == SwitchOrder::a_then_b);
}

void energy_threshold_test() {
    auto input = base_input();
    input.initial_energy = 3;
    assert(validate_map_routes(input).status == MapRouteStatus::insufficient_energy);
    input.has_batteries = true;
    assert(validate_map_routes(input).status == MapRouteStatus::requires_battery_verification);
    input.initial_energy = 4;
    assert(validate_map_routes(input).status == MapRouteStatus::ready_for_simulation);
}

void unreachable_exit_test() {
    auto input = base_input();
    input.exit = {2, 2};
    for (std::size_t column = 0; column < 4; ++column) {
        input.graph.blocked[4 + column] = true;
    }
    const auto result = validate_map_routes(input);
    assert(result.status == MapRouteStatus::unreachable);
    assert(!result.a_then_b);
    assert(!result.b_then_a);
    assert(!result.recommended);
    assert(!result.diagnostic.empty());
}

void directed_one_order_test() {
    auto input = base_input();
    input.graph = make_graph(1, 5);
    input.graph.neighbors = {{1}, {2}, {3}, {}, {}};
    input.player_start = {0, 0};
    input.switch_a = {0, 1};
    input.switch_b = {0, 2};
    input.exit = {0, 3};
    input.guard_start = {0, 4};
    const auto result = validate_map_routes(input);
    assert(result.status == MapRouteStatus::ready_for_simulation);
    assert(result.a_then_b);
    assert(!result.b_then_a);
    assert(result.recommended->total_energy_cost == 3);
}

void high_entry_cost_test() {
    auto input = base_input();
    input.graph.entry_costs[1] = 7;
    const auto result = validate_map_routes(input);
    assert(result.recommended);
    assert(result.recommended->total_energy_cost >= 3);
    std::int64_t recalculated = 0;
    for (std::size_t index = 1; index < result.recommended->complete_path.size(); ++index) {
        const Position destination = result.recommended->complete_path[index];
        recalculated += input.graph.entry_costs[destination.row * input.graph.columns + destination.column];
    }
    assert(recalculated == result.recommended->total_energy_cost);
}

void invalid_settings_test() {
    auto input = base_input();
    input.initial_energy = 0;
    expect_invalid([&] { (void)validate_map_routes(input); });
    input = base_input();
    input.initial_energy = 11;
    expect_invalid([&] { (void)validate_map_routes(input); });
    input = base_input();
    input.turn_limit = 0;
    expect_invalid([&] { (void)validate_map_routes(input); });
    input = base_input();
    input.switch_b = input.switch_a;
    expect_invalid([&] { (void)validate_map_routes(input); });
    input = base_input();
    input.guard_start = {4, 0};
    expect_invalid([&] { (void)validate_map_routes(input); });
    input = base_input();
    input.graph.blocked[1] = true;
    expect_invalid([&] { (void)validate_map_routes(input); });
    input = base_input();
    input.graph.entry_costs[1] = -1;
    expect_invalid([&] { (void)validate_map_routes(input); });
    input = base_input();
    input.graph.entry_costs.pop_back();
    expect_invalid([&] { (void)validate_map_routes(input); });
}

void total_cost_overflow_test() {
    auto input = base_input();
    std::fill(input.graph.entry_costs.begin(), input.graph.entry_costs.end(),
              1'000'000'000);
    bool caught = false;
    try {
        (void)validate_map_routes(input);
    } catch (const std::overflow_error&) {
        caught = true;
    }
    assert(caught);
}

void dynamic_dimensions_test() {
    for (const auto& [rows, columns] : {
            std::pair<std::size_t, std::size_t>{10, 15},
            {20, 30},
            {40, 60}}) {
        auto input = base_input();
        input.graph = make_graph(rows, columns);
        input.guard_start = {rows - 1, columns - 1};
        const auto result = validate_map_routes(input);
        assert(result.recommended);
        assert(result.recommended->total_energy_cost == 3);
    }
}

}

void run_map_validator_tests() {
    basic_order_and_path_test();
    choose_b_first_test();
    deterministic_tie_test();
    energy_threshold_test();
    unreachable_exit_test();
    directed_one_order_test();
    high_entry_cost_test();
    invalid_settings_test();
    total_cost_overflow_test();
    dynamic_dimensions_test();
}
