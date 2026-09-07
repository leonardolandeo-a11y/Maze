#pragma once
#include <cstddef>
#include <array>
#include "types.h"


template<typename CellType, std::size_t Rows, std::size_t Columns>
class Grid {
private:
    std::array<CellType, Rows * Columns> cells_{};
public:
    using value_type = CellType;

    // using iterator = /* iterador de la representación */;
    // using const_iterator = /* iterador const de la representación */;

    static constexpr std::size_t rows() noexcept { return Rows; }

    static constexpr std::size_t columns() noexcept { return Columns; }

    [[nodiscard]] constexpr bool contains(Position position) const noexcept{
        if (position.row < Rows && position.column < Columns) {
        return true;
    }

    return false;
    }

    CellType& at(Position position) {
        return cells_[position.row * Columns + position.column];
    }

    const CellType& at(Position position) const {
        return cells_[position.row * Columns + position.column];
    }


    // iterator begin() noexcept;
    // iterator end() noexcept;
    // const_iterator begin() const noexcept;
    // const_iterator end() const noexcept;
    // const_iterator cbegin() const noexcept;
    // const_iterator cend() const noexcept;

};