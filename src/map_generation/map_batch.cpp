#include "circuit_escape/map_generation/map_batch.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <exception>
#include <limits>
#include <random>
#include <stdexcept>
#include <thread>
#include <utility>

namespace {

void validate_config(const MapBatchConfig& config) {
    if (config.rows < 3 || config.columns < 3 ||
        config.rows > std::numeric_limits<std::size_t>::max() / config.columns ||
        config.rows * config.columns > 1'000'000 ||
        ((config.rows - 1) / 2) * ((config.columns - 1) / 2) < 5) {
        throw std::invalid_argument("Map dimensions cannot accommodate five distinct room positions");
    }
    if (config.candidate_count == 0 || config.candidate_count > 10 ||
        config.worker_count == 0 || config.maximum_search_states == 0) {
        throw std::invalid_argument("Invalid map batch size, worker count or search budget");
    }
    if (config.initial_energy <= 0 || config.maximum_energy < config.initial_energy ||
        config.turn_limit == 0 || config.rough_terrain_percent > 100 ||
        config.rough_terrain_cost < 1 || config.battery_recharge < 0 ||
        config.trap_energy_penalty < 0) {
        throw std::invalid_argument("Invalid map batch rules");
    }
}

std::vector<Position> room_positions(std::size_t rows, std::size_t columns) {
    std::vector<Position> rooms;
    for (std::size_t row = 1; row + 1 < rows; row += 2) {
        for (std::size_t column = 1; column + 1 < columns; column += 2) {
            rooms.push_back({row, column});
        }
    }
    return rooms;
}

void shuffle_positions(std::vector<Position>& positions, std::mt19937& random) {
    for (std::size_t count = positions.size(); count > 1; --count) {
        const std::size_t index = static_cast<std::size_t>(random()) % count;
        std::swap(positions[count - 1], positions[index]);
    }
}

std::size_t distance(Position first, Position second) {
    const std::size_t row = first.row > second.row ? first.row - second.row : second.row - first.row;
    const std::size_t column = first.column > second.column ? first.column - second.column : second.column - first.column;
    return row + column;
}

GraphData graph_from_grid(const Grid<Cell>& grid) {
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
            const auto connect = [&](std::size_t adjacent_row, std::size_t adjacent_column) {
                if (isTraversable(grid.at({adjacent_row, adjacent_column}))) {
                    graph.neighbors[vertex].push_back(adjacent_row * graph.columns + adjacent_column);
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

template<typename Work>
void parallel_for(std::size_t task_count, std::size_t worker_count, Work work) {
    std::atomic<std::size_t> next{0};
    std::vector<std::jthread> workers;
    workers.reserve(std::min(task_count, worker_count));
    for (std::size_t worker = 0; worker < std::min(task_count, worker_count); ++worker) {
        workers.emplace_back([&]() {
            for (;;) {
                const std::size_t index = next.fetch_add(1, std::memory_order_relaxed);
                if (index >= task_count) break;
                work(index);
            }
        });
    }
}

}

PreparedMap build_map(const MapBatchConfig& config, std::size_t id, std::uint32_t seed) {
    validate_config(config);
    if (id == 0 || id > config.candidate_count) {
        throw std::invalid_argument("Map identifier is outside the batch");
    }

    MazeGenerationResult maze = generate_maze(
        config.rows, config.columns, seed, config.extra_openings
    );
    std::mt19937 random(seed ^ 0xA511E9B3U);
    std::vector<Position> rooms = room_positions(config.rows, config.columns);
    shuffle_positions(rooms, random);

    const Position player = rooms[0];
    const Position switch_a = rooms[1];
    const Position switch_b = rooms[2];
    const Position exit = rooms[3];
    std::size_t guard_index = 4;
    for (std::size_t i = 5; i < rooms.size(); ++i) {
        if (distance(rooms[i], player) > distance(rooms[guard_index], player)) {
            guard_index = i;
        }
    }
    const Position guard = rooms[guard_index];

    const auto is_objective = [&](Position cell) {
        return cell == player || cell == switch_a || cell == switch_b ||
               cell == exit || cell == guard;
    };

    std::vector<Position> available;
    for (std::size_t row = 0; row < config.rows; ++row) {
        for (std::size_t column = 0; column < config.columns; ++column) {
            const Position cell{row, column};
            if (isTraversable(maze.grid.at(cell)) && !is_objective(cell)) {
                available.push_back(cell);
            }
        }
    }
    if (config.battery_count > available.size() ||
        config.trap_count > available.size() - config.battery_count) {
        throw std::invalid_argument("Not enough free cells for batteries and traps");
    }

    for (Position cell : available) {
        if (random() % 100U < config.rough_terrain_percent) {
            maze.grid.at(cell) = RoughTerrain{config.rough_terrain_cost};
        }
    }
    shuffle_positions(available, random);
    GameEffects effects;
    effects.battery_recharge = config.battery_recharge;
    effects.trap_energy_penalty = config.trap_energy_penalty;
    for (std::size_t i = 0; i < config.battery_count; ++i) {
        effects.batteries.push_back(available[i]);
        maze.grid.at(available[i]) = Battery{config.battery_recharge, false};
    }
    for (std::size_t i = config.battery_count;
         i < config.battery_count + config.trap_count; ++i) {
        effects.traps.push_back(available[i]);
        maze.grid.at(available[i]) = Trap{config.trap_energy_penalty, 1};
    }
    maze.grid.at(exit) = Exit{};

    MapValidationInput input{
        graph_from_grid(maze.grid), player, guard, switch_a, switch_b, exit,
        config.initial_energy, config.maximum_energy, config.turn_limit,
        !effects.batteries.empty()
    };
    return {id, seed, std::move(maze), std::move(input), std::move(effects)};
}

void validate_map(MapBatchEntry& entry, std::size_t maximum_search_states) {
    if (!entry.map) {
        if (entry.rejection_reason.empty()) entry.rejection_reason = "Map construction failed";
        return;
    }
    entry.route_validation = validate_map_routes(entry.map->input);
    const MapRouteValidation& routes = *entry.route_validation;
    if (routes.status == MapRouteStatus::unreachable ||
        routes.status == MapRouteStatus::insufficient_energy) {
        entry.rejection_reason = routes.diagnostic;
        return;
    }
    const GameVerifier game(entry.map->input, entry.map->effects);
    entry.winning_game = game.find_winning_game(maximum_search_states);
    if (!entry.winning_game || entry.winning_game->outcome != GameOutcome::victory ||
        game.replay(entry.winning_game->actions).outcome != GameOutcome::victory) {
        entry.winning_game.reset();
        entry.rejection_reason = "No reproducible winning game found within the search budget";
    }
}

MapBatchResult run_map_batch(const MapBatchConfig& config) {
    validate_config(config);
    MapBatchResult result;
    result.entries.resize(config.candidate_count);
    for (std::size_t i = 0; i < config.candidate_count; ++i) {
        result.entries[i].id = i + 1;
        result.entries[i].seed = config.base_seed +
            static_cast<std::uint32_t>(i + 1) * 0x9E3779B9U;
    }

    const auto construction_start = std::chrono::steady_clock::now();
    parallel_for(config.candidate_count, config.worker_count, [&](std::size_t i) {
        MapBatchEntry& entry = result.entries[i];
        try {
            entry.map = build_map(config, entry.id, entry.seed);
        } catch (const std::exception& error) {
            entry.rejection_reason = error.what();
        }
    });
    const auto validation_start = std::chrono::steady_clock::now();
    result.construction_milliseconds = std::chrono::duration<double, std::milli>(
        validation_start - construction_start
    ).count();

    parallel_for(config.candidate_count, config.worker_count, [&](std::size_t i) {
        MapBatchEntry& entry = result.entries[i];
        try {
            validate_map(entry, config.maximum_search_states);
        } catch (const std::exception& error) {
            entry.winning_game.reset();
            entry.rejection_reason = error.what();
        }
    });
    const auto validation_end = std::chrono::steady_clock::now();
    result.validation_milliseconds = std::chrono::duration<double, std::milli>(
        validation_end - validation_start
    ).count();

    std::vector<MapCandidate> candidates;
    candidates.reserve(result.entries.size());
    for (const MapBatchEntry& entry : result.entries) {
        if (entry.winning_game && entry.route_validation && entry.route_validation->recommended) {
            candidates.push_back({entry.id,
                entry.route_validation->recommended->total_energy_cost, true});
        }
    }
    result.selection = select_maps(candidates);
    return result;
}
