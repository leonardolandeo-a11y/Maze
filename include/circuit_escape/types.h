#pragma once

#include <string>
#include <optional>
#include <cstddef>

struct Position {
    std::size_t row{};
    std::size_t column{};
    friend bool operator==(const Position&, const Position&);
};


enum class Action { up, down, left, right, wait };

std::optional<Position> neighbor(Position origin, Action action);

std::string toString(Position position);