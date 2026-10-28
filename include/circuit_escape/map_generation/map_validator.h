#pragma once

#include "circuit_escape/project2_contracts.h"

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

enum class SwitchOrder {
    a_then_b,
    b_then_a
};

enum class MapRouteStatus {
    unreachable,
    insufficient_energy,
    requires_battery_verification,
    ready_for_simulation
};

struct MapValidationInput {
    GraphData graph;
    Position player_start;
    Position guard_start;
    Position switch_a;
    Position switch_b;
    Position exit;
    int initial_energy{};
    int maximum_energy{};
    std::size_t turn_limit{};
    bool has_batteries{};
};

struct MapRoutePlan {
    SwitchOrder order{SwitchOrder::a_then_b};
    std::array<PathResult, 3> segments;
    std::vector<Position> complete_path;
    int total_energy_cost{};
};

struct MapRouteValidation {
    MapRouteStatus status{MapRouteStatus::unreachable};
    std::optional<MapRoutePlan> a_then_b;
    std::optional<MapRoutePlan> b_then_a;
    std::optional<MapRoutePlan> recommended;
    std::string diagnostic;
};

MapRouteValidation validate_map_routes(const MapValidationInput& input);
