#include "circuit_escape/pathfinding/dijkstra.h"

#include <functional>
#include <limits>
#include <queue>
#include <stdexcept>
#include <utility>

namespace {

std::size_t checked_vertex_count(const GraphData& graph) {
    if (graph.rows == 0 || graph.columns == 0 ||
        graph.rows > std::numeric_limits<std::size_t>::max() / graph.columns) {
        throw std::invalid_argument("Invalid graph dimensions");
    }

    const std::size_t vertex_count = graph.rows * graph.columns;
    if (graph.neighbors.size() != vertex_count ||
        graph.blocked.size() != vertex_count ||
        graph.entry_costs.size() != vertex_count) {
        throw std::invalid_argument("Graph data size does not match its dimensions");
    }

    for (std::size_t vertex = 0; vertex < vertex_count; ++vertex) {
        if (graph.entry_costs[vertex] < 0) {
            throw std::invalid_argument("Entry costs must be nonnegative");
        }
        for (std::size_t adjacent : graph.neighbors[vertex]) {
            if (adjacent >= vertex_count) {
                throw std::invalid_argument("Graph neighbor index out of range");
            }
        }
    }

    return vertex_count;
}

std::size_t checked_index(const GraphData& graph, Position position) {
    if (position.row >= graph.rows || position.column >= graph.columns) {
        throw std::out_of_range("Graph position out of range");
    }
    return position.row * graph.columns + position.column;
}

}

DijkstraSearch run_dijkstra(const GraphData& graph, Position source) {
    const std::size_t vertex_count = checked_vertex_count(graph);
    const std::size_t source_index = checked_index(graph, source);

    DijkstraSearch result;
    result.source = source;
    result.distances.resize(vertex_count);
    result.predecessors.resize(vertex_count);

    if (graph.blocked[source_index]) {
        return result;
    }

    using QueueEntry = std::pair<std::int64_t, std::size_t>;
    std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<>> pending;
    std::vector<bool> settled(vertex_count, false);

    result.distances[source_index] = 0;
    pending.push({0, source_index});

    while (!pending.empty()) {
        const auto [current_cost, current] = pending.top();
        pending.pop();

        if (settled[current]) {
            continue;
        }
        settled[current] = true;
        ++result.explored;

        for (std::size_t adjacent : graph.neighbors[current]) {
            if (graph.blocked[adjacent] || settled[adjacent]) {
                continue;
            }

            const std::int64_t entry_cost = graph.entry_costs[adjacent];
            if (current_cost > std::numeric_limits<std::int64_t>::max() - entry_cost) {
                throw std::overflow_error("Dijkstra distance exceeds supported range");
            }

            const std::int64_t candidate = current_cost + entry_cost;
            if (!result.distances[adjacent].has_value() ||
                candidate < *result.distances[adjacent]) {
                result.distances[adjacent] = candidate;
                result.predecessors[adjacent] = current;
                pending.push({candidate, adjacent});
            }
        }
    }

    return result;
}

PathResult dijkstra_path(
    const GraphData& graph,
    const DijkstraSearch& search,
    Position destination
) {
    const std::size_t vertex_count = checked_vertex_count(graph);
    const std::size_t source_index = checked_index(graph, search.source);
    const std::size_t destination_index = checked_index(graph, destination);

    if (search.distances.size() != vertex_count ||
        search.predecessors.size() != vertex_count) {
        throw std::invalid_argument("Dijkstra search does not match graph dimensions");
    }

    PathResult result;
    result.explored = search.explored;

    if (!search.distances[destination_index].has_value()) {
        return result;
    }
    if (*search.distances[destination_index] > std::numeric_limits<int>::max()) {
        throw std::overflow_error("Path energy cost exceeds supported range");
    }

    std::vector<Position> reversed_path;
    std::size_t current = destination_index;

    for (std::size_t count = 0; count < vertex_count; ++count) {
        reversed_path.push_back({current / graph.columns, current % graph.columns});
        if (current == source_index) {
            break;
        }
        if (!search.predecessors[current].has_value()) {
            throw std::logic_error("Incomplete Dijkstra predecessor chain");
        }
        current = *search.predecessors[current];
    }

    if (current != source_index) {
        throw std::logic_error("Invalid Dijkstra predecessor chain");
    }

    result.found = true;
    result.cost = static_cast<int>(*search.distances[destination_index]);
    result.path.assign(reversed_path.rbegin(), reversed_path.rend());
    return result;
}

PathResult shortest_energy_path(
    const GraphData& graph,
    Position source,
    Position destination
) {
    return dijkstra_path(graph, run_dijkstra(graph, source), destination);
}
