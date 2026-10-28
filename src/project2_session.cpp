#include "circuit_escape/project2_session.h"

#include "circuit_escape/pathfinding/dijkstra.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>

namespace {

struct EnergyPlan {
    std::vector<HelpTarget> objectives;
    std::vector<int> segment_costs;
    std::vector<Position> complete_path;
    int total_cost{};
};

std::optional<EnergyPlan> create_plan(
    const GraphData& graph,
    const DijkstraSearch& current,
    const std::vector<HelpTarget>& objectives,
    Position switch_a,
    Position switch_b,
    Position exit
) {
    EnergyPlan result;
    result.objectives = objectives;
    const DijkstraSearch* search = &current;
    std::optional<DijkstraSearch> next_search;
    std::int64_t total = 0;

    for (HelpTarget target : objectives) {
        Position destination = exit;
        if (target == HelpTarget::switch_a) destination = switch_a;
        if (target == HelpTarget::switch_b) destination = switch_b;
        PathResult path = dijkstra_path(graph, *search, destination);
        if (!path.found) return std::nullopt;
        total += path.cost;
        if (total > std::numeric_limits<int>::max()) {
            throw std::overflow_error("Help route cost is too large");
        }
        result.segment_costs.push_back(path.cost);
        const auto first = result.complete_path.empty() ? path.path.begin() : path.path.begin() + 1;
        result.complete_path.insert(result.complete_path.end(), first, path.path.end());
        if (target != objectives.back()) {
            next_search = run_dijkstra(graph, destination);
            search = &*next_search;
        }
    }
    result.total_cost = static_cast<int>(total);
    return result;
}

}

namespace {
GameRules project2_rules(const SavedMap& map) {
    return GameRules{
        map.input.initial_energy, map.input.maximum_energy, map.input.turn_limit,
        1, 2, map.effects.wait_energy_cost, 0,
        map.effects.battery_recharge, map.effects.trap_energy_penalty, 0
    };
}

// Both the game and verifier operate on the validated map's orthogonal graph.
// The engine uses the saved cells for the exact terrain and item values.
}

Project2Session::Project2Session(SavedMap map)
    : map_(std::move(map)),
      verifier_(map_.input, map_.effects),
      environment_(map_.grid,
          Agent(map_.input.player_start, map_.input.initial_energy, map_.input.maximum_energy),
          project2_rules(map_), NavigationOptions{true, true}),
      state_(verifier_.initial_state()) {}

const SavedMap& Project2Session::map() const noexcept { return map_; }

const GameState& Project2Session::state() const noexcept { return state_; }

GameState Project2Session::step(Action action) {
    if (state_.outcome != GameOutcome::in_progress) {
        throw std::logic_error("Cannot advance a finished game");
    }

    // Reuse the Project 1 navigation engine for movement, energy, rewards,
    // consumed items, and the turn counter, without duplicating those rules.
    const StepResult result = environment_.step(action);
    state_.player = result.observation.agent;
    state_.energy = result.observation.energy;
    state_.turn = result.observation.turn;
    state_.score = result.observation.score;
    state_.resource_count = result.observation.collectedResources;

    const auto& cells = environment_.grid();
    for (std::size_t i = 0; i < map_.effects.batteries.size(); ++i) {
        const Position position = map_.effects.batteries[i];
        if (const auto* battery = std::get_if<Battery>(&cells.at(position));
            battery && battery->consumed) {
            state_.consumed_batteries[i / 64] |= std::uint64_t{1} << (i % 64);
        }
    }
    for (std::size_t i = 0; i < map_.effects.resources.size(); ++i) {
        const Position position = map_.effects.resources[i].first;
        if (const auto* resource = std::get_if<ResourceCell<int>>(&cells.at(position));
            resource && resource->collected) {
            state_.collected_resources[i / 64] |= std::uint64_t{1} << (i % 64);
        }
    }

    // Project 2 precedence: capture, energy, switches/exit, and turn limit.
    if (state_.player == state_.guard) {
        state_.outcome = GameOutcome::captured;
    } else {
        if (state_.turn % 2 == 0) {
            state_.guard = verifier_.guard_next_step(state_.guard, state_.player);
        }
        if (state_.player == state_.guard) {
            state_.outcome = GameOutcome::captured;
        } else if (state_.energy <= 0) {
            state_.outcome = GameOutcome::energy_depleted;
        } else {
            if (state_.player == map_.input.switch_a) state_.switch_a_active = true;
            if (state_.player == map_.input.switch_b) state_.switch_b_active = true;
            if (state_.player == map_.input.exit &&
                state_.switch_a_active && state_.switch_b_active) {
                state_.outcome = GameOutcome::victory;
            } else if (state_.turn >= map_.input.turn_limit) {
                state_.outcome = GameOutcome::turn_limit;
            }
        }
    }
    if (state_.outcome != GameOutcome::in_progress) {
        environment_.finish();
    }
    return state_;
}

void Project2Session::reset() {
    environment_.reset(0);
    state_ = verifier_.initial_state();
}

bool Project2Session::battery_consumed(Position position) const {
    const auto found = std::find(map_.effects.batteries.begin(), map_.effects.batteries.end(), position);
    if (found == map_.effects.batteries.end()) return false;
    const std::size_t index = static_cast<std::size_t>(found - map_.effects.batteries.begin());
    const std::size_t group = index / 64;
    return group < state_.consumed_batteries.size() &&
        (state_.consumed_batteries[group] & (std::uint64_t{1} << (index % 64))) != 0;
}

bool Project2Session::resource_collected(Position position) const {
    const auto found = std::find_if(map_.effects.resources.begin(), map_.effects.resources.end(),
        [position](const auto& resource) { return resource.first == position; });
    if (found == map_.effects.resources.end()) return false;
    const std::size_t index = static_cast<std::size_t>(found - map_.effects.resources.begin());
    const std::size_t group = index / 64;
    return group < state_.collected_resources.size() &&
        (state_.collected_resources[group] & (std::uint64_t{1} << (index % 64))) != 0;
}

HelpReport Project2Session::help_report() const {
    HelpReport report;
    report.current_energy = state_.energy;
    report.switch_a.status = state_.switch_a_active ? ObjectiveStatus::activated : ObjectiveStatus::pending;
    report.switch_b.status = state_.switch_b_active ? ObjectiveStatus::activated : ObjectiveStatus::pending;
    report.exit_goal.status = state_.switch_a_active && state_.switch_b_active
        ? ObjectiveStatus::enabled : ObjectiveStatus::locked;

    const auto& input = map_.input;
    const GraphData& graph = input.graph;
    const DijkstraSearch current = run_dijkstra(graph, state_.player);

    if (!state_.switch_a_active) {
        const PathResult path = dijkstra_path(graph, current, input.switch_a);
        if (path.found) {
            report.switch_a.cost_from_current = path.cost;
            report.path_to_a = path.path;
        }
    }
    if (!state_.switch_b_active) {
        const PathResult path = dijkstra_path(graph, current, input.switch_b);
        if (path.found) {
            report.switch_b.cost_from_current = path.cost;
            report.path_to_b = path.path;
        }
    }
    if (state_.switch_a_active && state_.switch_b_active) {
        const PathResult path = dijkstra_path(graph, current, input.exit);
        if (path.found) report.exit_goal.cost_from_current = path.cost;
    }

    std::vector<std::vector<HelpTarget>> orders;
    if (state_.switch_a_active && state_.switch_b_active) {
        orders.push_back({HelpTarget::exit});
    } else if (state_.switch_a_active) {
        orders.push_back({HelpTarget::switch_b, HelpTarget::exit});
    } else if (state_.switch_b_active) {
        orders.push_back({HelpTarget::switch_a, HelpTarget::exit});
    } else {
        orders.push_back({HelpTarget::switch_a, HelpTarget::switch_b, HelpTarget::exit});
        orders.push_back({HelpTarget::switch_b, HelpTarget::switch_a, HelpTarget::exit});
    }

    std::optional<EnergyPlan> best;
    for (const auto& order : orders) {
        auto candidate = create_plan(
            graph, current, order, input.switch_a, input.switch_b, input.exit
        );
        if (candidate && (!best || candidate->total_cost < best->total_cost)) {
            best = std::move(candidate);
        }
    }
    if (best) {
        report.total_cost = best->total_cost;
        report.recommended_order = std::move(best->objectives);
        report.segment_costs = std::move(best->segment_costs);
        report.complete_path = std::move(best->complete_path);
    }
    return report;
}
