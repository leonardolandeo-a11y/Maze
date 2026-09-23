#pragma once

#include <variant>
#include <type_traits>

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

/*
Agregamos lo siguiente:
    - Especialización total y parcial a traves CellTraits
    - Este struct es usado para verificar si la celda es atravesable/tiene recursos 
    y evitar codigo repeitivo
*/
template<typename CellType>
struct CellTraits {
    static constexpr bool traversable = true;
    static constexpr bool resource = false;
};

template<>
struct CellTraits<Wall> {
    static constexpr bool traversable = false;
    static constexpr bool resource = false;
};

template<typename Reward>
struct CellTraits<ResourceCell<Reward>> {
    static constexpr bool traversable = true;
    static constexpr bool resource = true;
};

using Cell = std::variant<Empty, Wall, RoughTerrain, ResourceCell<int>, Battery, Trap, Exit>;

//Funcion aux para verificar si la celda es atravesable 
inline bool isTraversable(const Cell& cell) {
    return std::visit(
        [](const auto& currentCell) {using CellType = std::remove_cvref_t<decltype(currentCell)>;
            return CellTraits<CellType>::traversable;
        },
        cell
    );
}