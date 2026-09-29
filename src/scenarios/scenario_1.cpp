#include "circuit_escape/scenarios/scenario_1.h"

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

Grid<Cell, 20, 30> createScenario1() {
    ScenarioGrid grid;

    // Escenario 1: circuito de entrenamiento.
    // Tiene una ruta principal sencilla, pero también incluye
    // desvíos opcionales con recursos y zonas peligrosas.

    addBorder(grid);

    addVerticalWall(grid, 6, 1, 12);
    addHorizontalWall(grid, 6, 6, 18);
    addVerticalWall(grid, 14, 2, 17);
    addHorizontalWall(grid, 12, 4, 25);
    addVerticalWall(grid, 22, 7, 18);
    addHorizontalWall(grid, 16, 14, 28);

    // Aperturas entre las distintas zonas del escenario.
    for (const Position passage : {
        Position{4, 6},
        Position{10, 6},
        Position{6, 10},
        Position{6, 15},
        Position{12, 8},
        Position{12, 14},
        Position{12, 20},
        Position{10, 22},
        Position{15, 22},
        Position{16, 18},
        Position{16, 22},
        Position{16, 26}
    }) {
        openPassage(grid, passage);
    }

    // Terreno difícil.
    for (const Position position : {
        Position{2, 8},
        Position{2, 9},
        Position{3, 8},
        Position{3, 9},
        Position{9, 7},
        Position{9, 8},
        Position{10, 7},
        Position{10, 8},
        Position{13, 15},
        Position{13, 16},
        Position{14, 15},
        Position{14, 16},
        Position{15, 15}
    }) {
        grid.at(position) = RoughTerrain{};
    }

    // Recursos.
    grid.at({2, 3}) = ResourceCell<int>{10};
    grid.at({4, 9}) = ResourceCell<int>{15};
    grid.at({9, 12}) = ResourceCell<int>{20};
    grid.at({13, 18}) = ResourceCell<int>{25};
    grid.at({15, 25}) = ResourceCell<int>{30};

    // Baterías.
    grid.at({4, 7}) = Battery{};
    grid.at({10, 15}) = Battery{};
    grid.at({15, 21}) = Battery{};

    // Trampas.
    grid.at({3, 5}) = Trap{};
    grid.at({5, 12}) = Trap{};
    grid.at({8, 18}) = Trap{};
    grid.at({13, 23}) = Trap{};

    // Salida.
    grid.at({12, 26}) = Exit{};

    return grid;
}