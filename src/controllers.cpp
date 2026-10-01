#include "circuit_escape/controllers.h"

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
