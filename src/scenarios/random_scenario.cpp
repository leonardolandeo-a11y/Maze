#include "circuit_escape/scenarios/random_scenario.h"

#include <random>
#include <set>

Grid<Cell, 20, 30> createRandomScenario() {
    Grid<Cell, 20, 30> grid;

    std::random_device rd;
    std::mt19937 generator(rd());

    Position start{1, 1};

    std::uniform_int_distribution<int> exitRowDistribution(10, 18);
    std::uniform_int_distribution<int> exitColumnDistribution(15, 28);

    Position exit{
        exitRowDistribution(generator),
        exitColumnDistribution(generator)
    };

    grid.at(exit) = Exit{};

    std::set<Position> safePath;

    Position current = start;
    safePath.insert(current);

    while (current.column != exit.column ||
           current.row != exit.row) {

        bool moveDown = false;
        bool moveRight = false;

        if (current.row < exit.row && current.column < exit.column) {
            std::uniform_int_distribution<int> direction(0, 1);

            if (direction(generator) == 0) {
                moveRight = true;
            } else {
                moveDown = true;
            }
        } else if (current.column < exit.column) {
            moveRight = true;
        } else if (current.row < exit.row) {
            moveDown = true;
        }

        if (moveRight) {
            ++current.column;
        } else if (moveDown) {
            ++current.row;
        }

        safePath.insert(current);
    }

    std::uniform_int_distribution<int> rowDistribution(0, 19);
    std::uniform_int_distribution<int> columnDistribution(0, 29);

    auto randomPosition = [&]() {
        return Position{
            rowDistribution(generator),
            columnDistribution(generator)
        };
    };

    auto placeCell = [&](Cell cell) {
        Position position;

        do {
            position = randomPosition();
        } while (
            safePath.contains(position) ||
            position == start ||
            position == exit ||
            !std::holds_alternative<Empty>(grid.at(position))
        );

        grid.at(position) = cell;
    };

    for (int i = 0; i < 50; ++i) {
        placeCell(Wall{});
    }

    for (int i = 0; i < 25; ++i) {
        placeCell(RoughTerrain{});
    }

    for (int i = 0; i < 5; ++i) {
        placeCell(ResourceCell<int>{10});
    }

    for (int i = 0; i < 3; ++i) {
        placeCell(Battery{});
    }

    for (int i = 0; i < 5; ++i) {
        placeCell(Trap{});
    }

    return grid;
}
