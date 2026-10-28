#include "circuit_escape/map_generation/map_storage.h"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace {

constexpr std::size_t maximum_cells = 1'000'000;

std::string difficulty_name(SavedDifficulty difficulty) {
    switch (difficulty) {
        case SavedDifficulty::easy: return "easy";
        case SavedDifficulty::medium: return "medium";
        case SavedDifficulty::hard: return "hard";
    }
    throw std::invalid_argument("Unknown map difficulty");
}

SavedDifficulty parse_difficulty(const std::string& token) {
    if (token == "easy") return SavedDifficulty::easy;
    if (token == "medium") return SavedDifficulty::medium;
    if (token == "hard") return SavedDifficulty::hard;
    throw std::runtime_error("Invalid map difficulty");
}

char action_symbol(Action action) {
    switch (action) {
        case Action::up: return 'U';
        case Action::down: return 'D';
        case Action::left: return 'L';
        case Action::right: return 'R';
        case Action::wait: return 'W';
    }
    throw std::invalid_argument("Invalid game action");
}

Action parse_action(const std::string& token) {
    if (token == "U") return Action::up;
    if (token == "D") return Action::down;
    if (token == "L") return Action::left;
    if (token == "R") return Action::right;
    if (token == "W") return Action::wait;
    throw std::runtime_error("Invalid saved game action");
}

std::string read_token(std::istream& input) {
    std::string token;
    if (!(input >> token)) throw std::runtime_error("Map file ended unexpectedly");
    return token;
}

void expect(std::istream& input, std::string_view expected) {
    if (read_token(input) != expected) {
        throw std::runtime_error("Invalid map file field: " + std::string(expected));
    }
}

template<typename Number>
Number parse_number(std::string_view token) {
    Number value{};
    const auto [end, error] = std::from_chars(token.data(), token.data() + token.size(), value);
    if (error != std::errc{} || end != token.data() + token.size()) {
        throw std::runtime_error("Invalid map file number");
    }
    return value;
}

template<typename Number>
Number read_number(std::istream& input) {
    return parse_number<Number>(read_token(input));
}

Position read_position(std::istream& input, std::string_view name) {
    expect(input, name);
    return {read_number<std::size_t>(input), read_number<std::size_t>(input)};
}

void write_position(std::ostream& output, std::string_view name, Position position) {
    output << name << ' ' << position.row << ' ' << position.column << '\n';
}

std::vector<std::string_view> parts(std::string_view token) {
    std::vector<std::string_view> result;
    for (;;) {
        const std::size_t delimiter = token.find(':');
        result.push_back(token.substr(0, delimiter));
        if (delimiter == std::string_view::npos) return result;
        token.remove_prefix(delimiter + 1);
    }
}

Cell parse_cell(const std::string& token) {
    const auto fields = parts(token);
    if (fields.size() == 1) {
        if (fields[0] == ".") return Empty{};
        if (fields[0] == "#") return Wall{};
        if (fields[0] == "E") return Exit{};
    } else if (fields.size() == 2) {
        if (fields[0] == "R") {
            const int cost = parse_number<int>(fields[1]);
            if (cost < 1) throw std::runtime_error("Invalid rough terrain cost");
            return RoughTerrain{cost};
        }
        if (fields[0] == "P") {
            const int reward = parse_number<int>(fields[1]);
            if (reward < 0) throw std::runtime_error("Invalid resource reward");
            return ResourceCell<int>{reward, false};
        }
        if (fields[0] == "B") {
            const int energy = parse_number<int>(fields[1]);
            if (energy < 0) throw std::runtime_error("Invalid battery energy");
            return Battery{energy, false};
        }
    } else if (fields.size() == 3 && fields[0] == "T") {
        const int energy = parse_number<int>(fields[1]);
        const int score = parse_number<int>(fields[2]);
        if (energy < 0 || score < 0) throw std::runtime_error("Invalid trap penalty");
        return Trap{energy, score};
    }
    throw std::runtime_error("Unknown map cell: " + token);
}

std::string cell_token(const Cell& cell) {
    if (std::holds_alternative<Empty>(cell)) return ".";
    if (std::holds_alternative<Wall>(cell)) return "#";
    if (std::holds_alternative<Exit>(cell)) return "E";
    if (const auto* value = std::get_if<RoughTerrain>(&cell)) {
        return "R:" + std::to_string(value->energyCost);
    }
    if (const auto* value = std::get_if<ResourceCell<int>>(&cell)) {
        if (value->collected) throw std::invalid_argument("Cannot save a consumed resource");
        return "P:" + std::to_string(value->reward);
    }
    if (const auto* value = std::get_if<Battery>(&cell)) {
        if (value->consumed) throw std::invalid_argument("Cannot save a consumed battery");
        return "B:" + std::to_string(value->energy);
    }
    const auto& value = std::get<Trap>(cell);
    return "T:" + std::to_string(value.energyPenalty) + ":" + std::to_string(value.scorePenalty);
}

GraphData graph_from_cells(const Grid<Cell>& grid) {
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

void verify_map(const SavedMap& map) {
    const std::size_t rows = map.grid.rows();
    const std::size_t columns = map.grid.columns();
    if (rows < 3 || columns < 3 || rows > maximum_cells / columns || map.id == 0 ||
        map.minimum_energy_cost < 0 || map.winning_actions.empty()) {
        throw std::invalid_argument("Invalid saved map metadata");
    }
    (void)difficulty_name(map.difficulty);
    const GraphData rebuilt = graph_from_cells(map.grid);
    const GraphData& graph = map.input.graph;
    if (graph.rows != rows || graph.columns != columns ||
        graph.neighbors != rebuilt.neighbors || graph.blocked != rebuilt.blocked ||
        graph.entry_costs != rebuilt.entry_costs) {
        throw std::invalid_argument("Map grid and graph do not match");
    }
    if (!map.grid.contains(map.input.exit) ||
        !std::holds_alternative<Exit>(map.grid.at(map.input.exit))) {
        throw std::invalid_argument("Exit cell is missing");
    }
    std::vector<Position> batteries;
    std::vector<Position> traps;
    std::vector<std::pair<Position, int>> resources;
    std::vector<std::pair<Position, int>> trap_scores;
    std::size_t exits = 0;
    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t column = 0; column < columns; ++column) {
            const Position position{row, column};
            const Cell& cell = map.grid.at(position);
            (void)cell_token(cell);
            if (std::holds_alternative<Exit>(cell)) ++exits;
            if (const auto* resource = std::get_if<ResourceCell<int>>(&cell)) {
                resources.emplace_back(position, resource->reward);
            }
            if (const auto* battery = std::get_if<Battery>(&cell)) {
                if (battery->energy != map.effects.battery_recharge) {
                    throw std::invalid_argument("Battery recharge values are inconsistent");
                }
                batteries.push_back(position);
            }
            if (const auto* trap = std::get_if<Trap>(&cell)) {
                if (trap->energyPenalty != map.effects.trap_energy_penalty) {
                    throw std::invalid_argument("Trap energy penalties are inconsistent");
                }
                traps.push_back(position);
                trap_scores.emplace_back(position, trap->scorePenalty);
            }
        }
    }
    auto sorted = [](std::vector<Position> positions) {
        std::sort(positions.begin(), positions.end(), [](Position a, Position b) {
            return a.row < b.row || (a.row == b.row && a.column < b.column);
        });
        return positions;
    };
    const auto sorted_effects = [](std::vector<std::pair<Position, int>> values) {
        std::sort(values.begin(), values.end(), [](const auto& a, const auto& b) {
            return a.first.row < b.first.row ||
                (a.first.row == b.first.row && a.first.column < b.first.column);
        });
        return values;
    };
    if (exits != 1 || sorted(batteries) != sorted(map.effects.batteries) ||
        sorted(traps) != sorted(map.effects.traps) ||
        sorted_effects(resources) != sorted_effects(map.effects.resources) ||
        sorted_effects(trap_scores) != sorted_effects(map.effects.trap_score_penalties) ||
        map.input.has_batteries != !batteries.empty()) {
        throw std::invalid_argument("Map effects and cells do not match");
    }
    const auto routes = validate_map_routes(map.input);
    if (!routes.recommended ||
        routes.recommended->total_energy_cost != map.minimum_energy_cost) {
        throw std::invalid_argument("Saved map minimum energy cost does not match Dijkstra");
    }
    const GameVerifier verifier(map.input, map.effects);
    const auto replay = verifier.replay(map.winning_actions);
    if (replay.outcome != GameOutcome::victory ||
        replay.actions.size() != map.winning_actions.size()) {
        throw std::invalid_argument("Saved actions do not reproduce a complete victory");
    }
}

const MapBatchEntry& selected_entry(const MapBatchResult& batch, std::size_t id) {
    const auto found = std::find_if(batch.entries.begin(), batch.entries.end(),
        [id](const MapBatchEntry& entry) { return entry.id == id; });
    if (found == batch.entries.end()) {
        throw std::invalid_argument("Selected map identifier is missing");
    }
    return *found;
}

}

SavedMap::SavedMap(std::size_t rows, std::size_t columns) : grid(rows, columns) {}

SavedMap make_saved_map(const MapBatchEntry& entry, SavedDifficulty difficulty) {
    if (!entry.map || !entry.winning_game ||
        entry.winning_game->outcome != GameOutcome::victory ||
        !entry.route_validation || !entry.route_validation->recommended ||
        entry.id != entry.map->id || entry.seed != entry.map->seed) {
        throw std::invalid_argument("Only verified map candidates can be saved");
    }
    SavedMap result(entry.map->maze.grid.rows(), entry.map->maze.grid.columns());
    result.id = entry.id;
    result.seed = entry.seed;
    result.difficulty = difficulty;
    result.minimum_energy_cost = entry.route_validation->recommended->total_energy_cost;
    result.grid = entry.map->maze.grid;
    result.input = entry.map->input;
    result.effects = entry.map->effects;
    result.winning_actions = entry.winning_game->actions;
    verify_map(result);
    return result;
}

void save_map(const SavedMap& map, const std::filesystem::path& path) {
    verify_map(map);
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) throw std::runtime_error("Cannot create map file");
    output << "CIRCUIT_ESCAPE_MAP 1\n";
    output << "id " << map.id << '\n';
    output << "seed " << map.seed << '\n';
    output << "difficulty " << difficulty_name(map.difficulty) << '\n';
    output << "minimum_energy_cost " << map.minimum_energy_cost << '\n';
    output << "dimensions " << map.grid.rows() << ' ' << map.grid.columns() << '\n';
    output << "energy " << map.input.initial_energy << ' ' << map.input.maximum_energy << '\n';
    output << "turn_limit " << map.input.turn_limit << '\n';
    write_position(output, "player", map.input.player_start);
    write_position(output, "guard", map.input.guard_start);
    write_position(output, "switch_a", map.input.switch_a);
    write_position(output, "switch_b", map.input.switch_b);
    write_position(output, "exit", map.input.exit);
    output << "effects " << map.effects.battery_recharge << ' '
           << map.effects.trap_energy_penalty << ' ' << map.effects.wait_energy_cost << '\n';
    output << "actions " << map.winning_actions.size();
    for (Action action : map.winning_actions) output << ' ' << action_symbol(action);
    output << "\ncells\n";
    for (std::size_t row = 0; row < map.grid.rows(); ++row) {
        for (std::size_t column = 0; column < map.grid.columns(); ++column) {
            if (column != 0) output << ' ';
            output << cell_token(map.grid.at({row, column}));
        }
        output << '\n';
    }
    output << "end\n";
    if (!output) throw std::runtime_error("Failed to write map file");
}

SavedMap load_map(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("Cannot open map file");
    expect(input, "CIRCUIT_ESCAPE_MAP");
    if (read_number<unsigned int>(input) != 1U) {
        throw std::runtime_error("Unsupported map format version");
    }
    expect(input, "id");
    const std::size_t id = read_number<std::size_t>(input);
    expect(input, "seed");
    const std::uint32_t seed = read_number<std::uint32_t>(input);
    expect(input, "difficulty");
    const SavedDifficulty difficulty = parse_difficulty(read_token(input));
    expect(input, "minimum_energy_cost");
    const int minimum_cost = read_number<int>(input);
    expect(input, "dimensions");
    const std::size_t rows = read_number<std::size_t>(input);
    const std::size_t columns = read_number<std::size_t>(input);
    if (rows < 3 || columns < 3 || rows > maximum_cells / columns) {
        throw std::runtime_error("Invalid saved map dimensions");
    }
    SavedMap map(rows, columns);
    map.id = id;
    map.seed = seed;
    map.difficulty = difficulty;
    map.minimum_energy_cost = minimum_cost;
    expect(input, "energy");
    map.input.initial_energy = read_number<int>(input);
    map.input.maximum_energy = read_number<int>(input);
    expect(input, "turn_limit");
    map.input.turn_limit = read_number<std::size_t>(input);
    map.input.player_start = read_position(input, "player");
    map.input.guard_start = read_position(input, "guard");
    map.input.switch_a = read_position(input, "switch_a");
    map.input.switch_b = read_position(input, "switch_b");
    map.input.exit = read_position(input, "exit");
    expect(input, "effects");
    map.effects.battery_recharge = read_number<int>(input);
    map.effects.trap_energy_penalty = read_number<int>(input);
    map.effects.wait_energy_cost = read_number<int>(input);
    expect(input, "actions");
    const std::size_t actions = read_number<std::size_t>(input);
    if (actions == 0 || actions > map.input.turn_limit || actions > maximum_cells) {
        throw std::runtime_error("Invalid winning game action count");
    }
    map.winning_actions.reserve(actions);
    for (std::size_t i = 0; i < actions; ++i) {
        map.winning_actions.push_back(parse_action(read_token(input)));
    }
    expect(input, "cells");
    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t column = 0; column < columns; ++column) {
            const Position position{row, column};
            map.grid.at(position) = parse_cell(read_token(input));
            if (const auto* resource = std::get_if<ResourceCell<int>>(&map.grid.at(position))) {
                map.effects.resources.emplace_back(position, resource->reward);
            }
            if (const auto* battery = std::get_if<Battery>(&map.grid.at(position))) {
                map.effects.batteries.push_back(position);
                if (battery->energy != map.effects.battery_recharge) {
                    throw std::runtime_error("Inconsistent battery value");
                }
            }
            if (const auto* trap = std::get_if<Trap>(&map.grid.at(position))) {
                map.effects.traps.push_back(position);
                map.effects.trap_score_penalties.emplace_back(position, trap->scorePenalty);
                if (trap->energyPenalty != map.effects.trap_energy_penalty) {
                    throw std::runtime_error("Inconsistent trap value");
                }
            }
        }
    }
    expect(input, "end");
    std::string trailing;
    if (input >> trailing) throw std::runtime_error("Unexpected trailing map file content");
    map.input.has_batteries = !map.effects.batteries.empty();
    map.input.graph = graph_from_cells(map.grid);
    verify_map(map);
    return map;
}

void save_selected_maps(const MapBatchResult& batch, const std::filesystem::path& directory) {
    if (!batch.selection) {
        throw std::invalid_argument("Three verified difficulty levels are required");
    }
    const SavedMap easy = make_saved_map(
        selected_entry(batch, batch.selection->easy_id), SavedDifficulty::easy);
    const SavedMap medium = make_saved_map(
        selected_entry(batch, batch.selection->medium_id), SavedDifficulty::medium);
    const SavedMap hard = make_saved_map(
        selected_entry(batch, batch.selection->hard_id), SavedDifficulty::hard);
    std::filesystem::create_directories(directory);
    save_map(easy, directory / "mapa_facil.txt");
    save_map(medium, directory / "mapa_medio.txt");
    save_map(hard, directory / "mapa_dificil.txt");
}
