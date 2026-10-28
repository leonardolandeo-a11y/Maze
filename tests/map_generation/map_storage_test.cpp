#include "circuit_escape/map_generation/map_storage.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

GraphData graph_for(const Grid<Cell>& grid) {
    GraphData graph;
    graph.rows = grid.rows();
    graph.columns = grid.columns();
    const std::size_t count = graph.rows * graph.columns;
    graph.neighbors.resize(count);
    graph.blocked.resize(count);
    graph.entry_costs.resize(count, 1);
    for (std::size_t row = 0; row < graph.rows; ++row) {
        for (std::size_t column = 0; column < graph.columns; ++column) {
            const std::size_t vertex = row * graph.columns + column;
            const Cell& cell = grid.at({row, column});
            graph.blocked[vertex] = !isTraversable(cell);
            if (const auto* rough = std::get_if<RoughTerrain>(&cell)) {
                graph.entry_costs[vertex] = rough->energyCost;
            }
            if (graph.blocked[vertex]) continue;
            const auto connect = [&](std::size_t r, std::size_t c) {
                if (isTraversable(grid.at({r, c}))) {
                    graph.neighbors[vertex].push_back(r * graph.columns + c);
                }
            };
            if (row > 0) connect(row - 1, column);
            if (column > 0) connect(row, column - 1);
            if (column + 1 < graph.columns) connect(row, column + 1);
            if (row + 1 < graph.rows) connect(row + 1, column);
        }
    }
    return graph;
}

SavedMap verified_fixture(std::size_t rows, std::size_t columns) {
    SavedMap map(rows, columns);
    map.id = 7;
    map.seed = 87654;
    map.difficulty = SavedDifficulty::medium;
    map.minimum_energy_cost = 3;
    map.grid.at({1, 4}) = Exit{};
    map.grid.at({5, 5}) = RoughTerrain{4};
    map.grid.at({3, 3}) = Battery{6, false};
    map.grid.at({4, 4}) = Trap{5, 2};
    map.grid.at({3, 4}) = ResourceCell<int>{9, false};
    map.grid.at({rows - 2, columns - 3}) = Wall{};
    map.effects.batteries = {{3, 3}};
    map.effects.traps = {{4, 4}};
    map.effects.resources = {{{3, 4}, 9}};
    map.effects.trap_score_penalties = {{{4, 4}, 2}};
    map.effects.battery_recharge = 6;
    map.effects.trap_energy_penalty = 5;
    map.effects.wait_energy_cost = 1;
    map.input.graph = graph_for(map.grid);
    map.input.player_start = {1, 1};
    map.input.guard_start = {rows - 2, columns - 2};
    map.input.switch_a = {1, 2};
    map.input.switch_b = {1, 3};
    map.input.exit = {1, 4};
    map.input.initial_energy = 25;
    map.input.maximum_energy = 30;
    map.input.turn_limit = 40;
    map.input.has_batteries = true;
    map.winning_actions = {Action::right, Action::right, Action::right};
    return map;
}

template<typename Function>
void expect_failure(Function operation) {
    bool caught = false;
    try {
        operation();
    } catch (const std::exception&) {
        caught = true;
    }
    assert(caught);
}

void compare_maps(const SavedMap& source, const SavedMap& loaded) {
    assert(source.id == loaded.id);
    assert(source.seed == loaded.seed);
    assert(source.difficulty == loaded.difficulty);
    assert(source.minimum_energy_cost == loaded.minimum_energy_cost);
    assert(source.grid.rows() == loaded.grid.rows());
    assert(source.grid.columns() == loaded.grid.columns());
    assert(source.input.player_start == loaded.input.player_start);
    assert(source.input.guard_start == loaded.input.guard_start);
    assert(source.input.switch_a == loaded.input.switch_a);
    assert(source.input.switch_b == loaded.input.switch_b);
    assert(source.input.exit == loaded.input.exit);
    assert(source.input.initial_energy == loaded.input.initial_energy);
    assert(source.input.maximum_energy == loaded.input.maximum_energy);
    assert(source.input.turn_limit == loaded.input.turn_limit);
    assert(source.input.graph.neighbors == loaded.input.graph.neighbors);
    assert(source.input.graph.blocked == loaded.input.graph.blocked);
    assert(source.input.graph.entry_costs == loaded.input.graph.entry_costs);
    assert(source.effects.batteries == loaded.effects.batteries);
    assert(source.effects.traps == loaded.effects.traps);
    assert(source.effects.resources == loaded.effects.resources);
    assert(source.effects.trap_score_penalties == loaded.effects.trap_score_penalties);
    assert(source.effects.battery_recharge == loaded.effects.battery_recharge);
    assert(source.effects.trap_energy_penalty == loaded.effects.trap_energy_penalty);
    assert(source.effects.wait_energy_cost == loaded.effects.wait_energy_cost);
    assert(source.winning_actions == loaded.winning_actions);
    for (std::size_t row = 0; row < source.grid.rows(); ++row) {
        for (std::size_t column = 0; column < source.grid.columns(); ++column) {
            const Cell& first = source.grid.at({row, column});
            const Cell& second = loaded.grid.at({row, column});
            assert(first.index() == second.index());
            if (const auto* rough = std::get_if<RoughTerrain>(&first)) {
                assert(rough->energyCost == std::get<RoughTerrain>(second).energyCost);
            }
            if (const auto* battery = std::get_if<Battery>(&first)) {
                assert(battery->energy == std::get<Battery>(second).energy);
                assert(!std::get<Battery>(second).consumed);
            }
            if (const auto* trap = std::get_if<Trap>(&first)) {
                assert(trap->energyPenalty == std::get<Trap>(second).energyPenalty);
                assert(trap->scorePenalty == std::get<Trap>(second).scorePenalty);
            }
            if (const auto* resource = std::get_if<ResourceCell<int>>(&first)) {
                assert(resource->reward == std::get<ResourceCell<int>>(second).reward);
                assert(!std::get<ResourceCell<int>>(second).collected);
            }
        }
    }
    const GameVerifier verifier(loaded.input, loaded.effects);
    assert(verifier.replay(loaded.winning_actions).outcome == GameOutcome::victory);
}

void round_trip_dimensions_test(const std::filesystem::path& folder) {
    for (const auto& [rows, columns] : {
            std::pair<std::size_t, std::size_t>{10, 15}, {20, 30}, {40, 60}}) {
        const SavedMap map = verified_fixture(rows, columns);
        const auto path = folder / ("map_" + std::to_string(rows) + ".txt");
        save_map(map, path);
        assert(std::filesystem::file_size(path) > 100);
        compare_maps(map, load_map(path));
    }
}

void reject_invalid_input_test(const std::filesystem::path& folder) {
    const auto path = folder / "invalid.txt";
    SavedMap map = verified_fixture(10, 15);
    map.winning_actions = {Action::wait};
    expect_failure([&] { save_map(map, path); });
    map = verified_fixture(10, 15);
    map.minimum_energy_cost = 5;
    expect_failure([&] { save_map(map, path); });
    map = verified_fixture(10, 15);
    map.grid.at({1, 4}) = Empty{};
    expect_failure([&] { save_map(map, path); });
    map = verified_fixture(10, 15);
    map.grid.at({3, 3}) = Battery{8, false};
    expect_failure([&] { save_map(map, path); });
    map = verified_fixture(10, 15);
    map.grid.at({3, 4}) = ResourceCell<int>{9, true};
    expect_failure([&] { save_map(map, path); });
    map = verified_fixture(10, 15);
    save_map(map, path);
    std::ifstream input(path);
    const std::string text((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    auto corrupted = text;
    corrupted.replace(0, std::string("CIRCUIT_ESCAPE_MAP").size(), "INVALID_MAP_HEADER");
    {
        std::ofstream output(path);
        output << corrupted;
    }
    expect_failure([&] { (void)load_map(path); });
    corrupted = text;
    const std::size_t version = corrupted.find("CIRCUIT_ESCAPE_MAP 1");
    assert(version != std::string::npos);
    corrupted.replace(version, std::string("CIRCUIT_ESCAPE_MAP 1").size(), "CIRCUIT_ESCAPE_MAP 2");
    {
        std::ofstream output(path);
        output << corrupted;
    }
    expect_failure([&] { (void)load_map(path); });
    corrupted = text;
    const std::size_t cell_begin = corrupted.find("cells\n");
    assert(cell_begin != std::string::npos);
    corrupted.replace(cell_begin, std::string("cells\n").size(), "cells\nBROKEN ");
    {
        std::ofstream output(path);
        output << corrupted;
    }
    expect_failure([&] { (void)load_map(path); });
    corrupted = text;
    const std::size_t position = corrupted.find("minimum_energy_cost 3");
    assert(position != std::string::npos);
    corrupted.replace(position, std::string("minimum_energy_cost 3").size(), "minimum_energy_cost -1");
    {
        std::ofstream output(path);
        output << corrupted;
    }
    expect_failure([&] { (void)load_map(path); });
    {
        std::ofstream output(path);
        output << text << "extra\n";
    }
    expect_failure([&] { (void)load_map(path); });
    expect_failure([&] { (void)load_map(folder / "not_found.txt"); });
}

void selected_maps_test(const std::filesystem::path& folder) {
    MapBatchConfig config;
    config.candidate_count = 10;
    config.worker_count = 4;
    const MapBatchResult result = run_map_batch(config);
    assert(result.selection);
    save_selected_maps(result, folder / "selected");
    const auto check = [&](const std::string& file, std::size_t id,
                           SavedDifficulty difficulty) {
        const SavedMap map = load_map(folder / "selected" / file);
        assert(map.id == id);
        assert(map.difficulty == difficulty);
        assert(map.grid.rows() == config.rows);
        assert(map.grid.columns() == config.columns);
        const auto& entry = result.entries[id - 1];
        assert(map.seed == entry.seed);
        assert(map.minimum_energy_cost == entry.route_validation->recommended->total_energy_cost);
        assert(map.winning_actions == entry.winning_game->actions);
    };
    check("mapa_facil.txt", result.selection->easy_id, SavedDifficulty::easy);
    check("mapa_medio.txt", result.selection->medium_id, SavedDifficulty::medium);
    check("mapa_dificil.txt", result.selection->hard_id, SavedDifficulty::hard);
    // An invalid destination must not overwrite another map from the existing set.
    const auto blocked = folder / "blocked_output";
    std::filesystem::create_directories(blocked);
    {
        std::ofstream previous(blocked / "mapa_facil.txt");
        previous << "previous easy map\n";
    }
    std::filesystem::create_directory(blocked / "mapa_medio.txt");
    expect_failure([&] { save_selected_maps(result, blocked); });
    {
        std::ifstream preserved(blocked / "mapa_facil.txt");
        std::string line;
        std::getline(preserved, line);
        assert(line == "previous easy map");
    }
    assert(std::filesystem::is_directory(blocked / "mapa_medio.txt"));
    assert(!std::filesystem::exists(blocked / "mapa_dificil.txt"));
    // No temporary files should be left after a rejected save.
    for (const auto& item : std::filesystem::directory_iterator(blocked)) {
        assert(item.path().filename().string().find(".circuit_escape_stage_") == std::string::npos);
    }

    // Re-saving to an existing destination must update the whole verified set.
    save_selected_maps(result, folder / "selected");
    check("mapa_facil.txt", result.selection->easy_id, SavedDifficulty::easy);
    check("mapa_medio.txt", result.selection->medium_id, SavedDifficulty::medium);
    check("mapa_dificil.txt", result.selection->hard_id, SavedDifficulty::hard);

    MapBatchResult empty;
    expect_failure([&] { save_selected_maps(empty, folder / "no_selection"); });
    MapBatchEntry unverified;
    unverified.id = 1;
    expect_failure([&] { (void)make_saved_map(unverified, SavedDifficulty::easy); });
}

}

void run_map_storage_tests() {
    const auto folder = std::filesystem::temp_directory_path() / "maze_project2_storage_tests";
    std::filesystem::remove_all(folder);
    std::filesystem::create_directories(folder);
    round_trip_dimensions_test(folder);
    reject_invalid_input_test(folder);
    selected_maps_test(folder);
    std::filesystem::remove_all(folder);
}
