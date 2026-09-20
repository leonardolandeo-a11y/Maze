#include <cassert>
#include <queue>
#include <set>
#include <variant>

#include "circuit_escape/scenarios/scenario_2.h"

void test_scenario2_tamano() {
    auto grid = createScenario2();

    assert(Grid<Cell, 20, 30>::rows() == 20);
    assert(Grid<Cell, 20, 30>::columns() == 30);
}

void test_scenario2_muros() {
    auto grid = createScenario2();

    assert(std::holds_alternative<Wall>(grid.at({1, 3})));
    assert(std::holds_alternative<Wall>(grid.at({1, 4})));
    assert(std::holds_alternative<Wall>(grid.at({1, 5})));
    assert(std::holds_alternative<Wall>(grid.at({2, 5})));
    assert(std::holds_alternative<Wall>(grid.at({3, 5})));
    assert(std::holds_alternative<Wall>(grid.at({4, 5})));
    assert(std::holds_alternative<Wall>(grid.at({6, 8})));
    assert(std::holds_alternative<Wall>(grid.at({7, 8})));
}

void test_scenario2_rough_terrain() {
    auto grid = createScenario2();

    assert(std::holds_alternative<RoughTerrain>(grid.at({3, 2})));
    assert(std::holds_alternative<RoughTerrain>(grid.at({3, 3})));
    assert(std::holds_alternative<RoughTerrain>(grid.at({4, 2})));
    assert(std::holds_alternative<RoughTerrain>(grid.at({4, 3})));
    assert(std::holds_alternative<RoughTerrain>(grid.at({8, 10})));
    assert(std::holds_alternative<RoughTerrain>(grid.at({8, 11})));
    assert(std::holds_alternative<RoughTerrain>(grid.at({9, 10})));
}

void test_scenario2_resources() {
    auto grid = createScenario2();

    assert(std::holds_alternative<ResourceCell<int>>(grid.at({2, 2})));
    assert(std::holds_alternative<ResourceCell<int>>(grid.at({7, 6})));
    assert(std::holds_alternative<ResourceCell<int>>(grid.at({10, 12})));

    auto resource1 = std::get<ResourceCell<int>>(grid.at({2, 2}));
    auto resource2 = std::get<ResourceCell<int>>(grid.at({7, 6}));
    auto resource3 = std::get<ResourceCell<int>>(grid.at({10, 12}));

    assert(resource1.reward == 10);
    assert(resource2.reward == 20);
    assert(resource3.reward == 30);

    assert(!resource1.collected);
    assert(!resource2.collected);
    assert(!resource3.collected);
}

void test_scenario2_baterias() {
    auto grid = createScenario2();

    assert(std::holds_alternative<Battery>(grid.at({4, 8})));
    assert(std::holds_alternative<Battery>(grid.at({9, 14})));
}

void test_scenario2_trampas() {
    auto grid = createScenario2();

    assert(std::holds_alternative<Trap>(grid.at({5, 6})));
    assert(std::holds_alternative<Trap>(grid.at({8, 13})));
    assert(std::holds_alternative<Trap>(grid.at({11, 16})));
}

void test_scenario2_salida() {
    auto grid = createScenario2();

    assert(std::holds_alternative<Exit>(grid.at({12, 18})));
}

void test_scenario2_tiene_ruta() {
    auto grid = createScenario2();

    Position start{1, 1};
    Position exit{12, 18};

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
