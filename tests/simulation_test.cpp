#include <cassert>

#include "circuit_escape/simulation.h"

GameRules simulationTestRules() {
    return GameRules{
        10, // energia inicial
       10, // energia maxima
       20, // limite de turnos
       1,  // costo normal
       2,  // costo de terreno elevado
       1,  // costo de wait o intento invalido
       7,  // puntos por recurso
       3,  // recarga de bateria
       2,  // penalizacion de energia por trampa
       1   // penalizacion de puntaje por trampa
   };
}

//Comprueba que una simulacion termine sin entrada del usuario
void test_random_simulation_completes_automatically() {
    Grid<Cell, 1, 4> grid;

    grid.at({0, 3}) = Exit{};

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 4> environment(grid, agent, simulationTestRules());

    SimulationResult result = runRandomSimulation(environment, 12345);

    assert(environment.isFinished());
    assert(result.reason != EndReason::none);
    assert(result.turns > 0);
    assert(!result.actions.empty());
    assert(result.actions.size() == result.turns);
}

int main() {
    test_random_simulation_completes_automatically();

    return 0;
}
