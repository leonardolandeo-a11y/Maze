#pragma once

#include "circuit_escape/types.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

// Graph representation
struct GraphData {
    std::size_t rows{};
    std::size_t columns{};

    std::vector<std::vector<std::size_t>> neighbors;
    std::vector<bool> blocked;
    std::vector<int> entry_costs;
};

// Pathfinding result
struct PathResult {
    bool found{false};
    std::vector<Position> path;
    int cost{};
    std::size_t explored{};
};

// Map difficulty selection
struct MapCandidate {
    std::size_t id{};
    int minimum_energy_cost{};
    bool verified_win{false};
};

struct MapSelection {
    std::size_t easy_id{};
    std::size_t medium_id{};
    std::size_t hard_id{};
};

// Help information
enum class ObjectiveStatus {
    pending,
    activated,
    enabled,
    locked
};

struct ObjectiveInfo {
    ObjectiveStatus status{ObjectiveStatus::pending};
    std::optional<int> cost_from_current;
};

enum class HelpTarget {
    switch_a,
    switch_b,
    exit
};

struct HelpReport {
    ObjectiveInfo switch_a;
    ObjectiveInfo switch_b;
    ObjectiveInfo exit_goal;

    int current_energy{};
    std::optional<int> total_cost;

    std::vector<HelpTarget> recommended_order;
    std::vector<int> segment_costs;

    std::vector<Position> path_to_a;
    std::vector<Position> path_to_b;
    std::vector<Position> complete_path;
};

// Game status presentation
struct HudReport {
    std::size_t turn{};
    std::size_t turn_limit{};

    int energy{};
    int maximum_energy{};

    bool switch_a_activated{};
    bool switch_b_activated{};
    bool guard_moves_next_turn{};
};

// Game symbols
enum class GameSymbol {
    player,
    empty,
    wall,
    rough_terrain,
    resource,
    battery,
    trap,
    exit,
    switch_a_pending,
    switch_a_active,
    switch_b_pending,
    switch_b_active,
    guard
};

enum class SymbolMode {
    emoji,
    ascii
};
