#pragma once

#include "circuit_escape/project2_contracts.h"
#include <cstddef>
#include <vector>

GraphData build_graph(
    std::size_t rows,
    std::size_t columns,
    const std::vector<bool>& blocked,
    const std::vector<int>& entry_costs
);

const std::vector<std::size_t>& graph_neighbors(
    const GraphData& graph,
    std::size_t vertex
);