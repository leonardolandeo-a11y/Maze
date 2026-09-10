#pragma once

#include <variant>

struct Empty {};

struct Wall {};
struct RoughTerrain {
    int energyCost{2};
};

template<typename Reward>
struct ResourceCell {
    Reward reward;  //struct template recompensa
    bool collected{false}; //Permite controlar que ya fueron recogias
};
struct Battery {
    int energy{3}; // Puede ser configurable por la dificultad
    bool consumed{false}; //Permite controlar que ya fue recogida
};
struct Trap {
    int energyPenalty{2};
    int scorePenalty{1};
};
struct Exit {};

using Cell = std::variant<Empty, Wall, RoughTerrain, ResourceCell<int>, Battery, Trap, Exit>;
