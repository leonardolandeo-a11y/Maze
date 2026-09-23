#include <cassert>
#include <queue>
#include <set>
#include <variant>

#include "circuit_escape/scenarios/random_scenario.h"

void test_random_scenario_tamano() {
    auto grid = createRandomScenario();

    assert(Grid<Cell, 20, 30>::rows() == 20);
    assert(Grid<Cell, 20, 30>::columns() == 30);
}

void test_random_scenario_inicio_libre() {
    auto grid = createRandomScenario();

    assert(std::holds_alternative<Empty>(grid.at({1, 1})));
}

void test_random_scenario_tiene_una_salida() {
    auto grid = createRandomScenario();

    int exits = 0;

    for (const auto& cell : grid) {
        if (std::holds_alternative<Exit>(cell)) {
            ++exits;
        }
    }

    assert(exits == 1);
}

void test_random_scenario_tiene_ruta() {
    auto grid = createRandomScenario();

    Position start{1, 1};
    Position exit{-1, -1};

    for (int row = 0; row < 20; ++row) {
        for (int column = 0; column < 30; ++column) {
            Position position{row, column};

            if (std::holds_alternative<Exit>(grid.at(position))) {
                exit = position;
            }
        }
    }

    std::queue<Position> pendientes;
    std::set<Position> visitadas;

    pendientes.push(start);
    visitadas.insert(start);

    const int dr[] = {-1, 1, 0, 0};
    const int dc[] = {0, 0, -1, 1};

    bool encontrada = false;

    while (!pendientes.empty()) {
        Position actual = pendientes.front();
        pendientes.pop();

        if (actual == exit) {
            encontrada = true;
            break;
        }

        for (int i = 0; i < 4; ++i) {
            Position siguiente{
                actual.row + dr[i],
                actual.column + dc[i]
            };

            if (siguiente.row < 0 || siguiente.row >= 20 ||
                siguiente.column < 0 || siguiente.column >= 30) {
                continue;
            }

            if (std::holds_alternative<Wall>(grid.at(siguiente))) {
                continue;
            }

            if (visitadas.insert(siguiente).second) {
                pendientes.push(siguiente);
            }
        }
    }

    assert(encontrada);
}

void test_random_scenario_tiene_elementos() {
    auto grid = createRandomScenario();

    int walls = 0;
    int rough = 0;
    int resources = 0;
    int batteries = 0;
    int traps = 0;

    for (const auto& cell : grid) {
        if (std::holds_alternative<Wall>(cell)) {
            ++walls;
        } else if (std::holds_alternative<RoughTerrain>(cell)) {
            ++rough;
        } else if (std::holds_alternative<ResourceCell<int>>(cell)) {
            ++resources;
        } else if (std::holds_alternative<Battery>(cell)) {
            ++batteries;
        } else if (std::holds_alternative<Trap>(cell)) {
            ++traps;
        }
    }

    assert(walls == 50);
    assert(rough == 25);
    assert(resources == 5);
    assert(batteries == 3);
    assert(traps == 5);
}
