#include <cassert>
#include <variant>
#include "circuit_escape/scenarios/scenario_1.h"

void test_scenario1_tamano() {
    auto grid = createScenario1();

    assert(Grid<Cell, 20, 30>::rows() == 20);
    assert(Grid<Cell, 20, 30>::columns() == 30);
}

void test_scenario1_muros() {
    auto grid = createScenario1();

    assert(std::holds_alternative<Wall>(grid.at({0, 3})));
    assert(std::holds_alternative<Wall>(grid.at({0, 4})));
    assert(std::holds_alternative<Wall>(grid.at({0, 5})));

    assert(std::holds_alternative<Wall>(grid.at({1, 5})));
    assert(std::holds_alternative<Wall>(grid.at({2, 5})));
    assert(std::holds_alternative<Wall>(grid.at({3, 5})));
}

void test_scenario1_rough_terrain() {
    auto grid = createScenario1();

    assert(std::holds_alternative<RoughTerrain>(grid.at({2, 2})));
    assert(std::holds_alternative<RoughTerrain>(grid.at({2, 3})));
    assert(std::holds_alternative<RoughTerrain>(grid.at({3, 2})));
}

void test_scenario1_resources() {
    auto grid = createScenario1();

    assert(std::holds_alternative<ResourceCell<int>>(grid.at({4, 4})));
    assert(std::holds_alternative<ResourceCell<int>>(grid.at({6, 8})));

    auto resource1 = std::get<ResourceCell<int>>(grid.at({4, 4}));
    auto resource2 = std::get<ResourceCell<int>>(grid.at({6, 8}));

    assert(resource1.reward == 10);
    assert(resource2.reward == 20);
    assert(!resource1.collected);
    assert(!resource2.collected);
}

void test_scenario1_baterias() {
    auto grid = createScenario1();

    assert(std::holds_alternative<Battery>(grid.at({1, 8})));
    assert(std::holds_alternative<Battery>(grid.at({8, 12})));
}

void test_scenario1_trampas() {
    auto grid = createScenario1();

    assert(std::holds_alternative<Trap>(grid.at({5, 5})));
    assert(std::holds_alternative<Trap>(grid.at({7, 10})));
}

void test_scenario1_salida() {
    auto grid = createScenario1();

    assert(std::holds_alternative<Exit>(grid.at({10, 15})));
}
