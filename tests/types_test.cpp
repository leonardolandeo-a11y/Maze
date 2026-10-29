#include "circuit_escape/types.h"

#include <cassert>
#include <cstddef>
#include <limits>

void run_types_tests() {
    const Position origin{4, 7};
    assert(neighbor(origin, Action::up) == (Position{3, 7}));
    assert(neighbor(origin, Action::down) == (Position{5, 7}));
    assert(neighbor(origin, Action::left) == (Position{4, 6}));
    assert(neighbor(origin, Action::right) == (Position{4, 8}));
    assert(neighbor(origin, Action::wait) == origin);

    assert(!neighbor({0, 7}, Action::up).has_value());
    assert(!neighbor({4, 0}, Action::left).has_value());

    const std::size_t maximum = std::numeric_limits<std::size_t>::max();
    assert(!neighbor({maximum, 7}, Action::down).has_value());
    assert(!neighbor({4, maximum}, Action::right).has_value());
    assert(neighbor({maximum, 7}, Action::up) == (Position{maximum - 1, 7}));
    assert(neighbor({4, maximum}, Action::left) == (Position{4, maximum - 1}));

    assert(!neighbor(origin, static_cast<Action>(999)).has_value());
    assert((Position{1, 2} == Position{1, 2}));
    assert((!(Position{1, 2} == Position{2, 1})));
}
