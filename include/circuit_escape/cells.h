#pragma once

#include <variant>

struct Empty {};
struct Wall {};
struct RoughTerrain { int energyCost{2}; };
template<typename Reward>
struct ResourceCell {
    Reward reward;
    bool collected{false};
};
struct Battery { int energy{3}; bool consumed{false}; };
struct Trap { int energyPenalty{2}; int scorePenalty{1}; };
struct Exit {};
using Cell = std::variant<Empty, Wall, RoughTerrain, ResourceCell<int>, Battery, Trap, Exit>;