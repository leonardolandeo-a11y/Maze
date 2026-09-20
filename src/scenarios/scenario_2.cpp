#include "circuit_escape/scenarios/scenario_2.h"

Grid<Cell, 20, 30> createScenario2() {
    Grid<Cell, 20, 30> grid;

    grid.at({1, 3}) = Wall{};
    grid.at({1, 4}) = Wall{};
    grid.at({1, 5}) = Wall{};

    grid.at({2, 5}) = Wall{};
    grid.at({3, 5}) = Wall{};
    grid.at({4, 5}) = Wall{};

    grid.at({6, 8}) = Wall{};
    grid.at({7, 8}) = Wall{};

    grid.at({3, 2}) = RoughTerrain{};
    grid.at({3, 3}) = RoughTerrain{};
    grid.at({4, 2}) = RoughTerrain{};
    grid.at({4, 3}) = RoughTerrain{};

    grid.at({8, 10}) = RoughTerrain{};
    grid.at({8, 11}) = RoughTerrain{};
    grid.at({9, 10}) = RoughTerrain{};

    grid.at({2, 2}) = ResourceCell<int>{10};
    grid.at({7, 6}) = ResourceCell<int>{20};
    grid.at({10, 12}) = ResourceCell<int>{30};

    grid.at({4, 8}) = Battery{};
    grid.at({9, 14}) = Battery{};

    grid.at({5, 6}) = Trap{};
    grid.at({8, 13}) = Trap{};
    grid.at({11, 16}) = Trap{};

    grid.at({12, 18}) = Exit{};

    return grid;
}
