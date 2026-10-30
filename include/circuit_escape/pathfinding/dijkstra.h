#pragma once

#include "circuit_escape/project2_contracts.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

struct DijkstraSearch {
    Position source{};
    std::vector<std::optional<std::int64_t>> distances;
    std::vector<std::optional<std::size_t>> predecessors;
    std::size_t explored{};
};

DijkstraSearch run_dijkstra(const GraphData& graph, Position source);

PathResult dijkstra_path(
    const GraphData& graph,
    const DijkstraSearch& search,
    Position destination
);

PathResult shortest_energy_path(
    const GraphData& graph,
    Position source,
    Position destination
);
