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

Project2Session::Project2Session(SavedMap map)
    : map_(std::move(map)),
      verifier_(map_.input, map_.effects),
      state_(verifier_.initial_state()) {}

const SavedMap& Project2Session::map() const noexcept { return map_; }

const GameState& Project2Session::state() const noexcept { return state_; }

GameState Project2Session::step(Action action) {
    state_ = verifier_.step(state_, action);
    return state_;
}

void Project2Session::reset() { state_ = verifier_.initial_state(); }

bool Project2Session::battery_consumed(Position position) const {
    const auto found = std::find(map_.effects.batteries.begin(), map_.effects.batteries.end(), position);
    if (found == map_.effects.batteries.end()) return false;
    const std::size_t index = static_cast<std::size_t>(found - map_.effects.batteries.begin());
    const std::size_t group = index / 64;
    return group < state_.consumed_batteries.size() &&
        (state_.consumed_batteries[group] & (std::uint64_t{1} << (index % 64))) != 0;
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
