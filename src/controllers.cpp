#include "circuit_escape/controllers.h"

#include <algorithm>
#include <array>
#include <limits>
#include <queue>
#include <stdexcept>

/*---------------------------------------------------------------------
ManhattanDistance() -> Calcula la distancia Manhattan entre dos posiciones.

La política heurística utiliza esta distancia como criterio simple
para elegir movimientos que acerquen al agente a la salida.
---------------------------------------------------------------------*/
std::size_t ManhattanDistance(Position first, Position second){
    std::size_t RowDistance ;
    std::size_t ColumnDistance ;
    if (first.row > second.row){
        RowDistance = first.row - second.row;
    }else{
        RowDistance = second.row - first.row ;
    }

    if (first.column > second.column){
        ColumnDistance = first.column - second.column;
    }else{
        ColumnDistance = second.column - first.column;
    }
    return RowDistance + ColumnDistance;
}

/*---------------------------------------------------------------------
selectAction() -> Selecciona, entre las acciones legales, aquella que deja al agente
más cerca de la salida según la distancia Manhattan.

La policy trabaja únicamente con Observation y las acciones legales:
no modifica directamente el entorno.

Si varias acciones producen la misma distancia, se conserva la
primera encontrada.
---------------------------------------------------------------------*/
Action HeuristicPolicy::selectAction(const Observation& observation,std::span<const Action> legalActions){
    if (legalActions.empty()) {
        throw std::invalid_argument("Require an action");
    }
    Action BestAction = legalActions.front();

    auto BestPosition = neighbor(observation.agent, BestAction);

    std::size_t BestDistance;
    if (BestPosition.has_value()){
        BestDistance = ManhattanDistance(BestPosition.value(), observation.goal);
    }else{
        BestDistance = ManhattanDistance(observation.agent, observation.goal);
    }

    for (Action action : legalActions){
        auto candidate = neighbor(observation.agent, action);
        if (!candidate.has_value()){
            continue;
        }
        const std::size_t distance = ManhattanDistance(candidate.value(),observation.goal);

        if (distance < BestDistance){
            BestDistance = distance;
            BestAction = action;
        }
    }
    return BestAction;

}


Action GuardBfsPolicy::selectAction(
    const Observation& observation,
    std::span<const Action> legalActions
) const {
    if (legalActions.empty()) {
        throw std::invalid_argument("GuardBfsPolicy requires a legal action");
    }
    if (graph_.columns == 0 || graph_.rows == 0 ||
        graph_.rows > std::numeric_limits<std::size_t>::max() / graph_.columns) {
        throw std::invalid_argument("GuardBfsPolicy requires valid graph dimensions");
    }
    const std::size_t count = graph_.rows * graph_.columns;
    if (graph_.neighbors.size() != count || graph_.blocked.size() != count) {
        throw std::invalid_argument("GuardBfsPolicy requires a complete graph");
    }
    const auto inside = [this](Position cell) {
        return cell.row < graph_.rows && cell.column < graph_.columns;
    };
    if (!inside(observation.agent) || !inside(observation.goal)) {
        throw std::out_of_range("Guard or player is outside the graph");
    }
    const auto index = [this](Position cell) {
        return cell.row * graph_.columns + cell.column;
    };
    const std::size_t source = index(observation.agent);
    const std::size_t target = index(observation.goal);
    if (graph_.blocked[source] || graph_.blocked[target]) {
        throw std::invalid_argument("Guard and player must be on open cells");
    }

    const auto can_wait = [&legalActions]() {
        return std::find(legalActions.begin(), legalActions.end(), Action::wait) !=
            legalActions.end();
    };
    if (source == target) {
        if (can_wait()) return Action::wait;
        throw std::invalid_argument("GuardBfsPolicy cannot stay without a legal wait");
    }

    // Each vertex is enqueued at most once. Traversing directions in a fixed
    // order gives deterministic tie-breaking independently of adjacency order.
    constexpr std::array<Action, 4> directions{
        Action::up, Action::left, Action::right, Action::down
    };
    std::vector<bool> visited(count, false);
    std::vector<Action> first_actions(count, Action::wait);
    std::queue<std::size_t> pending;
    visited[source] = true;
    pending.push(source);

    while (!pending.empty()) {
        const std::size_t current = pending.front();
        pending.pop();
        const Position current_position{current / graph_.columns, current % graph_.columns};
        for (const Action direction : directions) {
            const auto next = neighbor(current_position, direction);
            if (!next || !inside(*next)) continue;
            const std::size_t next_index = index(*next);
            if (visited[next_index] || graph_.blocked[next_index]) continue;
            const auto& edges = graph_.neighbors[current];
            if (std::find(edges.begin(), edges.end(), next_index) == edges.end()) continue;
            // The initial move must also be allowed by the invoking controller.
            if (current == source &&
                std::find(legalActions.begin(), legalActions.end(), direction) == legalActions.end()) {
                continue;
            }
            visited[next_index] = true;
            first_actions[next_index] = current == source ? direction : first_actions[current];
            if (next_index == target) return first_actions[next_index];
            pending.push(next_index);
        }
    }
    if (can_wait()) return Action::wait;
    throw std::invalid_argument("GuardBfsPolicy has no route and no legal wait");
}
