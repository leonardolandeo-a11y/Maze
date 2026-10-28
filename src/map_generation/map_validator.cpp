#include "circuit_escape/map_generation/map_validator.h"

#include "circuit_escape/pathfinding/dijkstra.h"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

namespace {

void validate_input(const MapValidationInput& input) {
    if (input.initial_energy <= 0 || input.maximum_energy <= 0 ||
        input.initial_energy > input.maximum_energy || input.turn_limit == 0) {
        throw std::invalid_argument("Invalid map energy or turn configuration");
    }

    const Position positions[] = {
        input.player_start, input.guard_start, input.switch_a,
        input.switch_b, input.exit
    };

    for (std::size_t i = 0; i < 5; ++i) {
        const Position position = positions[i];
        if (position.row >= input.graph.rows || position.column >= input.graph.columns) {
            throw std::invalid_argument("Map objective position is outside the graph");
        }
        if (input.graph.blocked.at(position.row * input.graph.columns + position.column)) {
            throw std::invalid_argument("Map objective position is blocked");
        }
        for (std::size_t j = 0; j < i; ++j) {
            if (position == positions[j]) {
                throw std::invalid_argument("Map objective positions must be distinct");
            }
        }
    }
}

std::optional<MapRoutePlan> combine_route(
    SwitchOrder order,
    PathResult first,
    PathResult second,
    PathResult third
) {
    if (!first.found || !second.found || !third.found) {
        return std::nullopt;
    }

    const std::int64_t total =
        static_cast<std::int64_t>(first.cost) + second.cost + third.cost;
    if (total > std::numeric_limits<int>::max()) {
        throw std::overflow_error("Complete route energy cost exceeds supported range");
    }

    MapRoutePlan plan;
    plan.order = order;
    plan.total_energy_cost = static_cast<int>(total);
    plan.segments = {std::move(first), std::move(second), std::move(third)};

    for (std::size_t segment = 0; segment < plan.segments.size(); ++segment) {
        const std::vector<Position>& path = plan.segments[segment].path;
        const std::size_t first_index = segment == 0 ? 0 : 1;
        plan.complete_path.insert(
            plan.complete_path.end(),
            path.begin() + static_cast<std::ptrdiff_t>(first_index),
            path.end()
        );
    }

    return plan;
}

}

MapRouteValidation validate_map_routes(const MapValidationInput& input) {
    const DijkstraSearch from_start = run_dijkstra(input.graph, input.player_start);
    validate_input(input);
    const DijkstraSearch from_a = run_dijkstra(input.graph, input.switch_a);
    const DijkstraSearch from_b = run_dijkstra(input.graph, input.switch_b);

    const PathResult start_to_a = dijkstra_path(input.graph, from_start, input.switch_a);
    const PathResult start_to_b = dijkstra_path(input.graph, from_start, input.switch_b);
    const PathResult a_to_b = dijkstra_path(input.graph, from_a, input.switch_b);
    const PathResult b_to_a = dijkstra_path(input.graph, from_b, input.switch_a);
    const PathResult a_to_exit = dijkstra_path(input.graph, from_a, input.exit);
    const PathResult b_to_exit = dijkstra_path(input.graph, from_b, input.exit);

    MapRouteValidation result;
    result.a_then_b = combine_route(
        SwitchOrder::a_then_b, start_to_a, a_to_b, b_to_exit
    );
    result.b_then_a = combine_route(
        SwitchOrder::b_then_a, start_to_b, b_to_a, a_to_exit
    );

    if (!result.a_then_b && !result.b_then_a) {
        result.status = MapRouteStatus::unreachable;
        result.diagnostic = "No route reaches both switches and the exit";
        return result;
    }

    if (!result.b_then_a ||
        (result.a_then_b &&
         result.a_then_b->total_energy_cost <= result.b_then_a->total_energy_cost)) {
        result.recommended = result.a_then_b;
    } else {
        result.recommended = result.b_then_a;
    }

    if (input.initial_energy > result.recommended->total_energy_cost) {
        result.status = MapRouteStatus::ready_for_simulation;
        result.diagnostic = "Energy is sufficient for the route without batteries; full simulation required";
    } else if (input.has_batteries) {
        result.status = MapRouteStatus::requires_battery_verification;
        result.diagnostic = "Batteries are required; full simulation must prove a win";
    } else {
        result.status = MapRouteStatus::insufficient_energy;
        result.diagnostic = "Initial energy is insufficient and no batteries are available";
    }
    return result;
}
