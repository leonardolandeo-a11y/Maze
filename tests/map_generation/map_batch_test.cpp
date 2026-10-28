#include "circuit_escape/map_generation/map_batch.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

void expect_invalid(const MapBatchConfig& config) {
    bool caught = false;
    try {
        (void)run_map_batch(config);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);
}

void invalid_configuration_test() {
    MapBatchConfig config;
    config.worker_count = 0;
    expect_invalid(config);
    config = {};
    config.candidate_count = 11;
    expect_invalid(config);
    config = {};
    config.candidate_count = 0;
    expect_invalid(config);
    config = {};
    config.rows = 3;
    config.columns = 3;
    expect_invalid(config);
    config = {};
    config.rough_terrain_percent = 101;
    expect_invalid(config);
    config = {};
    config.initial_energy = 0;
    expect_invalid(config);
    config = {};
    config.maximum_search_states = 0;
    expect_invalid(config);
}

void generated_candidate_integrity_test() {
    MapBatchConfig config;
    config.candidate_count = 3;
    config.worker_count = 2;
    config.battery_count = 2;
    config.trap_count = 2;
    config.maximum_search_states = 500;
    const auto batch = run_map_batch(config);
    assert(batch.entries.size() == 3);
    assert(batch.construction_milliseconds >= 0);
    assert(batch.validation_milliseconds >= 0);
    for (std::size_t i = 0; i < batch.entries.size(); ++i) {
        const auto& entry = batch.entries[i];
        assert(entry.id == i + 1);
        assert(entry.map);
        assert(entry.route_validation);
        assert(entry.map->maze.grid.rows() == config.rows);
        assert(entry.map->maze.grid.columns() == config.columns);
        assert(entry.map->input.graph.rows == config.rows);
        assert(entry.map->input.graph.columns == config.columns);
        assert(entry.map->effects.batteries.size() == 2);
        assert(entry.map->effects.traps.size() == 2);
        assert(entry.map->input.has_batteries);
        assert(entry.map->seed == entry.seed);
        if (entry.winning_game) {
            const GameVerifier game(entry.map->input, entry.map->effects);
            assert(game.replay(entry.winning_game->actions).outcome == GameOutcome::victory);
            assert(entry.rejection_reason.empty());
        } else {
            assert(!entry.rejection_reason.empty());
        }
    }
}

void deterministic_workers_test() {
    MapBatchConfig config;
    config.rows = 10;
    config.columns = 15;
    config.candidate_count = 4;
    config.worker_count = 1;
    config.maximum_search_states = 1500;
    const auto single = run_map_batch(config);
    config.worker_count = 4;
    const auto parallel = run_map_batch(config);
    assert(single.entries.size() == parallel.entries.size());
    assert(single.selection.has_value() == parallel.selection.has_value());
    if (single.selection && parallel.selection) {
        assert(single.selection->easy_id == parallel.selection->easy_id);
        assert(single.selection->medium_id == parallel.selection->medium_id);
        assert(single.selection->hard_id == parallel.selection->hard_id);
    }
    for (std::size_t i = 0; i < single.entries.size(); ++i) {
        const auto& a = single.entries[i];
        const auto& b = parallel.entries[i];
        assert(a.id == b.id);
        assert(a.seed == b.seed);
        assert(a.map.has_value() == b.map.has_value());
        assert(a.route_validation.has_value() == b.route_validation.has_value());
        assert(a.winning_game.has_value() == b.winning_game.has_value());
        assert(a.rejection_reason == b.rejection_reason);
        if (!a.map) continue;
        const auto& first = *a.map;
        const auto& second = *b.map;
        assert(first.input.player_start == second.input.player_start);
        assert(first.input.guard_start == second.input.guard_start);
        assert(first.input.switch_a == second.input.switch_a);
        assert(first.input.switch_b == second.input.switch_b);
        assert(first.input.exit == second.input.exit);
        assert(first.input.graph.neighbors == second.input.graph.neighbors);
        assert(first.input.graph.blocked == second.input.graph.blocked);
        assert(first.input.graph.entry_costs == second.input.graph.entry_costs);
        assert(first.maze.tree_openings.size() == second.maze.tree_openings.size());
        assert(first.maze.extra_openings.size() == second.maze.extra_openings.size());
        if (a.route_validation) {
            assert(a.route_validation->status == b.route_validation->status);
            assert(a.route_validation->recommended.has_value() ==
                   b.route_validation->recommended.has_value());
            if (a.route_validation->recommended) {
                assert(a.route_validation->recommended->total_energy_cost ==
                       b.route_validation->recommended->total_energy_cost);
                assert(a.route_validation->recommended->complete_path ==
                       b.route_validation->recommended->complete_path);
            }
        }
        if (a.winning_game) {
            assert(a.winning_game->actions == b.winning_game->actions);
            assert(a.winning_game->outcome == GameOutcome::victory);
        }
    }
}

void difficulty_selection_test() {
    MapBatchConfig config;
    config.candidate_count = 10;
    const auto result = run_map_batch(config);
    assert(result.selection);
    const auto& selection = *result.selection;
    assert(selection.easy_id != selection.medium_id);
    assert(selection.medium_id != selection.hard_id);
    int easy_cost = -1;
    int medium_cost = -1;
    int hard_cost = -1;
    for (const auto& entry : result.entries) {
        if (!entry.winning_game) continue;
        assert(entry.route_validation && entry.route_validation->recommended);
        const int cost = entry.route_validation->recommended->total_energy_cost;
        if (entry.id == selection.easy_id) easy_cost = cost;
        if (entry.id == selection.medium_id) medium_cost = cost;
        if (entry.id == selection.hard_id) hard_cost = cost;
        if (easy_cost >= 0) assert(cost >= easy_cost);
    }
    assert(easy_cost >= 0 && medium_cost > easy_cost && hard_cost > medium_cost);
    for (const auto& entry : result.entries) {
        if (!entry.winning_game) continue;
        const int cost = entry.route_validation->recommended->total_energy_cost;
        assert(cost >= easy_cost && cost <= hard_cost);
    }
}

void dimensions_and_rejections_test() {
    MapBatchConfig config;
    config.candidate_count = 1;
    config.worker_count = 3;
    config.maximum_search_states = 100;
    for (const auto& [rows, columns] : {
            std::pair<std::size_t, std::size_t>{10, 15},
            {20, 30}, {40, 60}}) {
        config.rows = rows;
        config.columns = columns;
        auto batch = run_map_batch(config);
        assert(batch.entries.size() == 1);
        assert(batch.entries[0].map);
        assert(batch.entries[0].route_validation);
    }
    config.rows = 10;
    config.columns = 15;
    config.battery_count = 1000;
    auto rejected = run_map_batch(config);
    assert(rejected.entries.size() == 1);
    assert(!rejected.entries[0].map);
    assert(!rejected.entries[0].rejection_reason.empty());
    assert(!rejected.selection);
}

}

void run_map_batch_tests() {
    invalid_configuration_test();
    generated_candidate_integrity_test();
    deterministic_workers_test();
    difficulty_selection_test();
    dimensions_and_rejections_test();
}
