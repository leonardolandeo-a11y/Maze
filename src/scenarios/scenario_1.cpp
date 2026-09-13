#include "circuit_escape/scenarios/scenario_1.h"

Grid<Cell, 20, 30> createScenario1() {
    Grid<Cell, 20, 30> grid;

    // Muros
    grid.at({0, 3}) = Wall{};
    grid.at({0, 4}) = Wall{};
    grid.at({0, 5}) = Wall{};

    grid.at({1, 5}) = Wall{};
    grid.at({2, 5}) = Wall{};
    grid.at({3, 5}) = Wall{};

    // Terreno difícil
    grid.at({2, 2}) = RoughTerrain{};
    grid.at({2, 3}) = RoughTerrain{};
    grid.at({3, 2}) = RoughTerrain{};

    // Recursos
    grid.at({4, 4}) = ResourceCell<int>{10};
    grid.at({6, 8}) = ResourceCell<int>{20};

    // Baterías
    grid.at({1, 8}) = Battery{};
    grid.at({8, 12}) = Battery{};

    // Trampas
    grid.at({5, 5}) = Trap{};
    grid.at({7, 10}) = Trap{};

    // Salida
    grid.at({10, 15}) = Exit{};

    return grid;
}