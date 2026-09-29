#include "circuit_escape/scenarios/scenario_2.h"

namespace {

using ScenarioGrid = Grid<Cell, 20, 30>;

void addBorder(ScenarioGrid& grid) {
    for (std::size_t column = 0; column < ScenarioGrid::columns(); ++column) {
        grid.at({0, column}) = Wall{};
        grid.at({ScenarioGrid::rows() - 1, column}) = Wall{};
    }

    for (std::size_t row = 0; row < ScenarioGrid::rows(); ++row) {
        grid.at({row, 0}) = Wall{};
        grid.at({row, ScenarioGrid::columns() - 1}) = Wall{};
    }
}

void addHorizontalWall(
    ScenarioGrid& grid,
    std::size_t row,
    std::size_t firstColumn,
    std::size_t lastColumn
) {
    for (std::size_t column = firstColumn; column <= lastColumn; ++column) {
        grid.at({row, column}) = Wall{};
    }
}

void addVerticalWall(
    ScenarioGrid& grid,
    std::size_t column,
    std::size_t firstRow,
    std::size_t lastRow
) {
    for (std::size_t row = firstRow; row <= lastRow; ++row) {
        grid.at({row, column}) = Wall{};
    }
}

void openPassage(ScenarioGrid& grid, Position position) {
    grid.at(position) = Empty{};
}

} // namespace

Grid<Cell, 20, 30> createScenario2() {
    ScenarioGrid grid;

    // Escenario 2: núcleo fragmentado.
    // Tiene más obstáculos, rutas secundarias y zonas peligrosas
    // que el escenario anterior.

    addBorder(grid);

    addVerticalWall(grid, 5, 1, 17);
    addHorizontalWall(grid, 4, 5, 18);
    addVerticalWall(grid, 11, 4, 18);
    addHorizontalWall(grid, 9, 1, 24);
    addVerticalWall(grid, 18, 2, 17);
    addHorizontalWall(grid, 14, 6, 28);
    addVerticalWall(grid, 24, 8, 18);

    // Muros secundarios.
    addHorizontalWall(grid, 2, 7, 15);
    addHorizontalWall(grid, 6, 1, 8);
    addHorizontalWall(grid, 7, 13, 18);
    addHorizontalWall(grid, 11, 18, 27);
    addHorizontalWall(grid, 17, 11, 22);

    // Aperturas.
    for (const Position passage : {
        Position{3, 5},
        Position{9, 5},
        Position{10, 5},
        Position{16, 5},

        Position{2, 10},
        Position{2, 13},

        Position{4, 8},
        Position{4, 14},
        Position{4, 18},

        Position{6, 3},
        Position{6, 5},

        Position{7, 11},
        Position{7, 16},
        Position{7, 18},

        Position{9, 4},
        Position{9, 10},
        Position{9, 11},
        Position{9, 16},
        Position{9, 18},
        Position{9, 22},

        Position{11, 21},
        Position{11, 24},

        Position{12, 11},
        Position{13, 18},

        Position{14, 11},
        Position{14, 18},
        Position{14, 25},

        Position{16, 24},

        Position{17, 11},
        Position{17, 15},
        Position{17, 18},
        Position{17, 21}
    }) {
        openPassage(grid, passage);
    }

    // Terreno difícil.
    for (const Position position : {
        Position{2, 16},
        Position{3, 16},
        Position{3, 17},

        Position{5, 9},
        Position{5, 10},
        Position{6, 9},
        Position{6, 10},

        Position{8, 6},
        Position{8, 7},
        Position{8, 8},

        Position{10, 12},
        Position{10, 13},
        Position{11, 12},
        Position{11, 13},

        Position{13, 19},
        Position{13, 20},
        Position{13, 21},

        Position{15, 23},
        Position{15, 24},
        Position{15, 25}
    }) {
        grid.at(position) = RoughTerrain{};
    }

    // Recursos.
    grid.at({2, 3}) = ResourceCell<int>{10};
    grid.at({3, 9}) = ResourceCell<int>{15};
    grid.at({6, 7}) = ResourceCell<int>{20};
    grid.at({8, 14}) = ResourceCell<int>{25};
    grid.at({11, 8}) = ResourceCell<int>{30};
    grid.at({12, 20}) = ResourceCell<int>{35};
    grid.at({15, 14}) = ResourceCell<int>{40};
    grid.at({16, 27}) = ResourceCell<int>{50};

    // Baterías.
    grid.at({3, 6}) = Battery{};
    grid.at({8, 10}) = Battery{};
    grid.at({10, 8}) = Battery{};
    grid.at({10, 19}) = Battery{};
    grid.at({12, 15}) = Battery{};
    grid.at({16, 22}) = Battery{};

    // Trampas.
    grid.at({3, 4}) = Trap{};
    grid.at({4, 13}) = Trap{};
    grid.at({7, 15}) = Trap{};
    grid.at({8, 17}) = Trap{};
    grid.at({10, 17}) = Trap{};
    grid.at({12, 23}) = Trap{};
    grid.at({15, 19}) = Trap{};
    grid.at({16, 25}) = Trap{};

    // Salida.
    grid.at({13, 23}) = Exit{};

    return grid;
}