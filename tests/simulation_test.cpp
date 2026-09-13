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

//Comprueba que la misma semilla produzca la misma secuencia de acciones
void test_same_seed_produces_same_action_sequence() {
    Grid<Cell, 2, 3> firstGrid;
    Grid<Cell, 2, 3> secondGrid;

    firstGrid.at({1, 2}) = Exit{};
    secondGrid.at({1, 2}) = Exit{};

    Agent firstAgent({0, 0}, 10, 10);
    Agent secondAgent({0, 0}, 10, 10);

    NavigationEnvironment<2, 3> firstEnvironment(firstGrid, firstAgent, simulationTestRules());
    NavigationEnvironment<2, 3> secondEnvironment(secondGrid, secondAgent, simulationTestRules());

    const std::uint32_t seed = 54321;

    SimulationResult firstResult = runRandomSimulation(firstEnvironment, seed);
    SimulationResult secondResult = runRandomSimulation(secondEnvironment, seed);

    assert(!firstResult.actions.empty());
    assert(!secondResult.actions.empty());
    assert(firstResult.actions == secondResult.actions);
}

int main() {
    test_random_simulation_completes_automatically();
    test_same_seed_produces_same_action_sequence();

    return 0;
}
