#include <cassert>
#include <queue>
#include <variant>

#include "circuit_escape/scenarios/scenario_2.h"

namespace {

template<typename CellType>
std::size_t countCells(const Grid<Cell, 20, 30>& grid) {
    std::size_t count = 0;

    for (const Cell& cell : grid) {
        if (std::holds_alternative<CellType>(cell)) {
            ++count;
        }
    }

    return count;
}

bool hasRoute(
    const Grid<Cell, 20, 30>& grid,
    Position start,
    Position goal
) {
    std::queue<Position> pending;
    bool visited[20][30]{};

    pending.push(start);
    visited[start.row][start.column] = true;

    constexpr int dr[] = {-1, 1, 0, 0};
    constexpr int dc[] = {0, 0, -1, 1};

    while (!pending.empty()) {
        const Position current = pending.front();
        pending.pop();

        if (current == goal) {
            return true;
        }

        for (int direction = 0; direction < 4; ++direction) {
            const int nextRow =
                static_cast<int>(current.row) + dr[direction];

            const int nextColumn =
                static_cast<int>(current.column) + dc[direction];

            if (
                nextRow < 0 ||
                nextRow >= 20 ||
                nextColumn < 0 ||
                nextColumn >= 30
            ) {
                continue;
            }

            Position next{
                static_cast<std::size_t>(nextRow),
                static_cast<std::size_t>(nextColumn)
            };

            if (
                visited[next.row][next.column] ||
                std::holds_alternative<Wall>(grid.at(next))
            ) {
                continue;
            }

            visited[next.row][next.column] = true;
            pending.push(next);
        }
    }

    return false;
}

} // namespace

void test_scenario2_tamano() {
    auto grid = createScenario2();
    using ScenarioGrid = decltype(grid);

    static_assert(ScenarioGrid::rows() == 20);
    static_assert(ScenarioGrid::columns() == 30);
}

void test_scenario2_estructura() {
    auto grid = createScenario2();

    assert(countCells<Wall>(grid) >= 170);
    assert(countCells<RoughTerrain>(grid) >= 15);
    assert(countCells<ResourceCell<int>>(grid) == 8);
    assert(countCells<Battery>(grid) == 6);
    assert(countCells<Trap>(grid) == 8);
    assert(countCells<Exit>(grid) == 1);
}

void test_scenario2_bordes() {
    auto grid = createScenario2();

    for (std::size_t column = 0; column < 30; ++column) {
        assert(
            std::holds_alternative<Wall>(
                grid.at({0, column})
            )
        );

        assert(
            std::holds_alternative<Wall>(
                grid.at({19, column})
            )
        );
    }

    for (std::size_t row = 0; row < 20; ++row) {
        assert(
            std::holds_alternative<Wall>(
                grid.at({row, 0})
            )
        );

        assert(
            std::holds_alternative<Wall>(
                grid.at({row, 29})
            )
        );
    }
}

void test_scenario2_elementos_clave() {
    auto grid = createScenario2();

    assert(
        std::holds_alternative<Empty>(
            grid.at({1, 1})
        )
    );

    assert(
        std::holds_alternative<Battery>(
            grid.at({10, 8})
        )
    );

    assert(
        std::holds_alternative<RoughTerrain>(
            grid.at({13, 19})
        )
    );

    assert(
        std::holds_alternative<Trap>(
            grid.at({12, 23})
        )
    );

    assert(
        std::holds_alternative<Exit>(
            grid.at({13, 23})
        )
    );
}

void test_scenario2_tiene_ruta() {
    const auto grid = createScenario2();

    assert(
        hasRoute(
            grid,
            {1, 1},
            {13, 23}
        )
    );
}

void run_tests_scenario_2() {
    test_scenario2_tamano();
    test_scenario2_estructura();
    test_scenario2_bordes();
    test_scenario2_elementos_clave();
    test_scenario2_tiene_ruta();
}