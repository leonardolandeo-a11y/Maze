#include "circuit_escape/map_generation/game_verifier.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <queue>
#include <stdexcept>
#include <tuple>
#include <unordered_set>
#include <utility>

namespace {

constexpr std::size_t unreachable = std::numeric_limits<std::size_t>::max();

std::size_t safe_sum(std::size_t first, std::size_t second) {
    if (first == unreachable || second == unreachable ||
        first > unreachable - second) {
        return unreachable;
    }
    return first + second;
}

struct StateKey {
    std::size_t player{};
    std::size_t guard{};
    std::size_t turn{};
    int energy{};
    unsigned int switches{};
    std::vector<std::uint64_t> batteries;

    bool operator==(const StateKey&) const = default;
};

struct StateKeyHash {
    std::size_t operator()(const StateKey& key) const noexcept {
        std::size_t hash = key.player;
        const auto combine = [&hash](std::size_t value) {
            hash ^= value + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
        };
        combine(key.guard);
        combine(key.turn);
        combine(static_cast<std::size_t>(key.energy));
        combine(key.switches);
        for (const std::uint64_t value : key.batteries) {
            combine(static_cast<std::size_t>(value));
        }
        return hash;
    }
};

struct SearchNode {
    GameState state;
    std::size_t parent{};
    Action action{Action::wait};
};

struct PendingNode {
    std::size_t estimate{};
    std::size_t steps{};
    int negative_energy{};
    std::size_t index{};

    bool operator>(const PendingNode& other) const {
        return std::tie(estimate, steps, negative_energy, index) >
               std::tie(other.estimate, other.steps, other.negative_energy, other.index);
    }
};

}

GameVerifier::GameVerifier(MapValidationInput input, GameEffects effects)
    : input_(std::move(input)), effects_(std::move(effects)) {
    (void)validate_map_routes(input_);
    if (effects_.battery_recharge < 0 || effects_.trap_energy_penalty < 0 ||
        effects_.wait_energy_cost < 0) {
        throw std::invalid_argument("Game effect costs must be nonnegative");
    }

    const std::size_t vertex_count = input_.graph.rows * input_.graph.columns;
    battery_indices_.assign(vertex_count, -1);
    trap_cells_.assign(vertex_count, false);
    trap_score_values_.assign(vertex_count, 1);
    resource_indices_.assign(vertex_count, -1);
    reversed_neighbors_.resize(vertex_count);
    ordered_neighbors_ = input_.graph.neighbors;

    for (std::size_t vertex = 0; vertex < vertex_count; ++vertex) {
        for (std::size_t adjacent : input_.graph.neighbors[vertex]) {
            const Position from = position(vertex);
            const Position to = position(adjacent);
            const bool horizontal = from.row == to.row &&
                (from.column + 1 == to.column || to.column + 1 == from.column);
            const bool vertical = from.column == to.column &&
                (from.row + 1 == to.row || to.row + 1 == from.row);
            if (!(horizontal || vertical)) {
                throw std::invalid_argument("Graph edges must connect orthogonal neighbors");
            }
            if (!input_.graph.blocked[vertex] && !input_.graph.blocked[adjacent]) {
                reversed_neighbors_[adjacent].push_back(vertex);
            }
        }
    }

    for (std::size_t vertex = 0; vertex < vertex_count; ++vertex) {
        const Position from = position(vertex);
        std::sort(ordered_neighbors_[vertex].begin(), ordered_neighbors_[vertex].end(),
            [this, from](std::size_t first, std::size_t second) {
                const auto rank = [this, from](std::size_t candidate) {
                    const Position target = position(candidate);
                    if (target.row < from.row) return 0;
                    if (target.column < from.column) return 1;
                    if (target.column > from.column) return 2;
                    return 3;
                };
                return rank(first) < rank(second);
            });
    }

    for (std::size_t battery = 0; battery < effects_.batteries.size(); ++battery) {
        const Position cell = effects_.batteries[battery];
        if (cell.row >= input_.graph.rows || cell.column >= input_.graph.columns ||
            input_.graph.blocked[index(cell)] ||
            battery_indices_[index(cell)] != -1) {
            throw std::invalid_argument("Invalid battery placement");
        }
        if (battery > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            throw std::length_error("Too many batteries");
        }
        battery_indices_[index(cell)] = static_cast<int>(battery);
    }

    for (Position cell : effects_.traps) {
        if (cell.row >= input_.graph.rows || cell.column >= input_.graph.columns ||
            input_.graph.blocked[index(cell)] || trap_cells_[index(cell)]) {
            throw std::invalid_argument("Invalid trap placement");
        }
        trap_cells_[index(cell)] = true;
    }
    for (const auto& [cell, points] : effects_.trap_score_penalties) {
        if (cell.row >= input_.graph.rows || cell.column >= input_.graph.columns ||
            !trap_cells_[index(cell)] || points < 0) {
            throw std::invalid_argument("Invalid trap score penalty");
        }
        trap_score_values_[index(cell)] = points;
    }
    for (std::size_t resource = 0; resource < effects_.resources.size(); ++resource) {
        const auto& [cell, points] = effects_.resources[resource];
        if (cell.row >= input_.graph.rows || cell.column >= input_.graph.columns ||
            input_.graph.blocked[index(cell)] || resource_indices_[index(cell)] != -1 ||
            battery_indices_[index(cell)] >= 0 || trap_cells_[index(cell)] ||
            points < 0 || resource > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            throw std::invalid_argument("Invalid resource placement or points");
        }
        resource_indices_[index(cell)] = static_cast<int>(resource);
    }
}

std::size_t GameVerifier::index(Position cell) const {
    return cell.row * input_.graph.columns + cell.column;
}

Position GameVerifier::position(std::size_t vertex) const {
    return {vertex / input_.graph.columns, vertex % input_.graph.columns};
}

GameState GameVerifier::initial_state() const {
    GameState result;
    result.player = input_.player_start;
    result.guard = input_.guard_start;
    result.energy = input_.initial_energy;
    result.consumed_batteries.resize((effects_.batteries.size() + 63) / 64);
    result.collected_resources.resize((effects_.resources.size() + 63) / 64);
    return result;
}

Position GameVerifier::guard_next_step(Position guard, Position player) const {
    return next_guard_position(guard, player);
}

Position GameVerifier::next_guard_position(Position guard, Position player) const {
    const std::size_t source = index(guard);
    const std::size_t destination = index(player);
    if (source == destination) {
        return guard;
    }
    const std::size_t vertex_count = input_.graph.rows * input_.graph.columns;
    std::vector<std::size_t> first_step(vertex_count, unreachable);
    std::queue<std::size_t> pending;
    first_step[source] = source;
    pending.push(source);

    while (!pending.empty()) {
        const std::size_t current = pending.front();
        pending.pop();
        for (std::size_t adjacent : ordered_neighbors_[current]) {
            if (input_.graph.blocked[adjacent] || first_step[adjacent] != unreachable) {
                continue;
            }
            first_step[adjacent] = current == source ? adjacent : first_step[current];
            if (adjacent == destination) {
                return position(first_step[adjacent]);
            }
            pending.push(adjacent);
        }
    }
    return guard;
}

GameState GameVerifier::step(const GameState& state, Action action) const {
    if (state.outcome != GameOutcome::in_progress) {
        throw std::logic_error("Cannot advance a finished game");
    }
    GameState result = state;
    ++result.turn;
    bool moved = false;

    if (action != Action::wait) {
        const std::optional<Position> destination = neighbor(state.player, action);
        if (destination && destination->row < input_.graph.rows &&
            destination->column < input_.graph.columns &&
            !input_.graph.blocked[index(*destination)]) {
            const auto& adjacent = input_.graph.neighbors[index(state.player)];
            if (std::find(adjacent.begin(), adjacent.end(), index(*destination)) != adjacent.end()) {
                result.player = *destination;
                const std::size_t vertex = index(*destination);
                result.energy = std::max(0, result.energy - input_.graph.entry_costs[vertex]);
                moved = true;

                const int battery = battery_indices_[vertex];
                if (battery >= 0) {
                    const std::size_t group = static_cast<std::size_t>(battery) / 64;
                    const std::uint64_t bit = std::uint64_t{1} << (static_cast<unsigned int>(battery) % 64);
                    if ((result.consumed_batteries[group] & bit) == 0) {
                        result.consumed_batteries[group] |= bit;
                        const std::int64_t recharged =
                            static_cast<std::int64_t>(result.energy) + effects_.battery_recharge;
                        result.energy = static_cast<int>(std::min(
                            recharged, static_cast<std::int64_t>(input_.maximum_energy)
                        ));
                    }
                }
                const int resource = resource_indices_[vertex];
                if (resource >= 0) {
                    const std::size_t group = static_cast<std::size_t>(resource) / 64;
                    const std::uint64_t bit = std::uint64_t{1} << (resource % 64);
                    if ((result.collected_resources[group] & bit) == 0) {
                        result.collected_resources[group] |= bit;
                        ++result.resource_count;
                        const std::int64_t score = static_cast<std::int64_t>(result.score) +
                            effects_.resources[resource].second;
                        result.score = static_cast<int>(std::min(
                            score, static_cast<std::int64_t>(std::numeric_limits<int>::max())));
                    }
                }
                if (trap_cells_[vertex]) {
                    result.energy = std::max(0, result.energy - effects_.trap_energy_penalty);
                    const std::int64_t score = static_cast<std::int64_t>(result.score) -
                        trap_score_values_[vertex];
                    result.score = static_cast<int>(std::max(
                        score, static_cast<std::int64_t>(std::numeric_limits<int>::min())));
                }
            }
        }
    }

    if (!moved) {
        result.energy = std::max(0, result.energy - effects_.wait_energy_cost);
    }

    if (result.player == result.guard) {
        result.outcome = GameOutcome::captured;
        return result;
    }
    if (result.turn % 2 == 0) {
        result.guard = next_guard_position(result.guard, result.player);
    }
    if (result.player == result.guard) {
        result.outcome = GameOutcome::captured;
    } else if (result.energy <= 0) {
        result.outcome = GameOutcome::energy_depleted;
    } else {
        if (result.player == input_.switch_a) result.switch_a_active = true;
        if (result.player == input_.switch_b) result.switch_b_active = true;
        if (result.player == input_.exit &&
            result.switch_a_active && result.switch_b_active) {
            result.outcome = GameOutcome::victory;
        } else if (result.turn >= input_.turn_limit) {
            result.outcome = GameOutcome::turn_limit;
        }
    }
    return result;
}

GameVerification GameVerifier::replay(const std::vector<Action>& actions) const {
    GameVerification result;
    result.history.push_back(initial_state());
    for (Action action : actions) {
        if (result.history.back().outcome != GameOutcome::in_progress) {
            break;
        }
        result.actions.push_back(action);
        result.history.push_back(step(result.history.back(), action));
    }
    result.outcome = result.history.back().outcome;
    return result;
}

std::vector<std::size_t> GameVerifier::distances_to(Position goal) const {
    std::vector<std::size_t> distance(reversed_neighbors_.size(), unreachable);
    std::queue<std::size_t> pending;
    distance[index(goal)] = 0;
    pending.push(index(goal));
    while (!pending.empty()) {
        const std::size_t current = pending.front();
        pending.pop();
        for (std::size_t preceding : reversed_neighbors_[current]) {
            if (distance[preceding] != unreachable) continue;
            distance[preceding] = distance[current] + 1;
            pending.push(preceding);
        }
    }
    return distance;
}

std::size_t GameVerifier::minimum_remaining_turns(
    const GameState& state,
    const std::vector<std::size_t>& to_a,
    const std::vector<std::size_t>& to_b,
    const std::vector<std::size_t>& to_exit,
    std::size_t a_to_b,
    std::size_t b_to_a,
    std::size_t a_to_exit,
    std::size_t b_to_exit
) const {
    const std::size_t player = index(state.player);
    if (state.switch_a_active && state.switch_b_active) return to_exit[player];
    if (state.switch_a_active) return safe_sum(to_b[player], b_to_exit);
    if (state.switch_b_active) return safe_sum(to_a[player], a_to_exit);
    return std::min(
        safe_sum(to_a[player], safe_sum(a_to_b, b_to_exit)),
        safe_sum(to_b[player], safe_sum(b_to_a, a_to_exit))
    );
}

std::optional<GameVerification> GameVerifier::find_winning_game(std::size_t maximum_states) const {
    if (maximum_states == 0) return std::nullopt;
    const auto validation = validate_map_routes(input_);
    for (const auto& plan : {validation.recommended, validation.a_then_b, validation.b_then_a}) {
        if (!plan) continue;
        std::vector<Action> actions;
        bool valid = true;
        for (std::size_t i = 1; i < plan->complete_path.size(); ++i) {
            const Position previous = plan->complete_path[i - 1];
            const Position next = plan->complete_path[i];
            if (next.row + 1 == previous.row && next.column == previous.column) actions.push_back(Action::up);
            else if (previous.row + 1 == next.row && next.column == previous.column) actions.push_back(Action::down);
            else if (next.column + 1 == previous.column && next.row == previous.row) actions.push_back(Action::left);
            else if (previous.column + 1 == next.column && next.row == previous.row) actions.push_back(Action::right);
            else { valid = false; break; }
        }
        if (valid) {
            GameVerification candidate = replay(actions);
            if (candidate.outcome == GameOutcome::victory) return candidate;
        }
    }

    const auto to_a = distances_to(input_.switch_a);
    const auto to_b = distances_to(input_.switch_b);
    const auto to_exit = distances_to(input_.exit);
    const std::size_t a_to_b = to_b[index(input_.switch_a)];
    const std::size_t b_to_a = to_a[index(input_.switch_b)];
    const std::size_t a_to_exit = to_exit[index(input_.switch_a)];
    const std::size_t b_to_exit = to_exit[index(input_.switch_b)];

    std::vector<SearchNode> nodes;
    nodes.reserve(std::min(maximum_states, std::size_t{100'000}));
    std::unordered_set<StateKey, StateKeyHash> visited;
    using Queue = std::priority_queue<PendingNode, std::vector<PendingNode>, std::greater<>>;
    Queue pending;
    const GameState initial = initial_state();
    nodes.push_back({initial, 0, Action::wait});
    visited.insert({index(initial.player), index(initial.guard), initial.turn, initial.energy, 0, initial.consumed_batteries});
    pending.push({minimum_remaining_turns(initial, to_a, to_b, to_exit,
        a_to_b, b_to_a, a_to_exit, b_to_exit), 0, -initial.energy, 0});

    const std::array<Action, 5> actions{
        Action::up, Action::left, Action::right, Action::down, Action::wait
    };

    while (!pending.empty() && nodes.size() < maximum_states) {
        const std::size_t node_index = pending.top().index;
        pending.pop();
        const GameState current = nodes[node_index].state;
        if (current.turn >= input_.turn_limit) continue;

        for (Action action : actions) {
            const GameState next = step(current, action);
            if (next.outcome != GameOutcome::in_progress &&
                next.outcome != GameOutcome::victory) continue;
            const std::size_t heuristic = minimum_remaining_turns(next, to_a, to_b,
                to_exit, a_to_b, b_to_a, a_to_exit, b_to_exit);
            if (heuristic == unreachable ||
                next.turn > input_.turn_limit ||
                heuristic > input_.turn_limit - next.turn) continue;

            unsigned int switches = 0;
            if (next.switch_a_active) switches |= 1U;
            if (next.switch_b_active) switches |= 2U;
            StateKey key{index(next.player), index(next.guard), next.turn,
                next.energy, switches, next.consumed_batteries};
            if (!visited.insert(std::move(key)).second) continue;

            const std::size_t next_index = nodes.size();
            nodes.push_back({next, node_index, action});
            if (next.outcome == GameOutcome::victory) {
                std::vector<Action> reversed;
                std::size_t cursor = next_index;
                while (cursor != 0) {
                    reversed.push_back(nodes[cursor].action);
                    cursor = nodes[cursor].parent;
                }
                std::reverse(reversed.begin(), reversed.end());
                GameVerification verified = replay(reversed);
                if (verified.outcome != GameOutcome::victory ||
                    verified.actions != reversed) {
                    throw std::logic_error("Winning sequence failed reproducibility check");
                }
                return verified;
            }
            if (nodes.size() >= maximum_states) break;
            pending.push({safe_sum(next.turn, heuristic), next.turn,
                -next.energy, next_index});
        }
    }
    return std::nullopt;
}
