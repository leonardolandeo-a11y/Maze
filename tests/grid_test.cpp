#include <cassert>
#include "grid.h"
#include "cells.h"

int main() {
    Grid<Cell, 3, 4> grid;

    assert(grid.rows() == 3);
    assert(grid.columns() == 4);

    assert(grid.contains({0, 0}));
    assert(grid.contains({2, 3}));

    assert(!grid.contains({3, 0}));
    assert(!grid.contains({0, 4}));

    grid.at({1, 2}) = Wall{};

    assert(std::holds_alternative<Wall>(grid.at({1, 2})));
}