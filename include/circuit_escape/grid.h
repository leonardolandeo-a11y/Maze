#pragma once
#include <cstddef>
#include <array>
#include "types.h"
#include <stdexcept>


template<typename CellType, std::size_t Rows, std::size_t Columns>
class Grid {
    //La rubrica exige que se verifique que la instancion de filas/colms sea diferente de 0
    static_assert(Rows > 0, "Grid must have at least one row");
    static_assert(Columns > 0, "Grid must have at least one column");
private:
    std::array<CellType, Rows * Columns> cells_{};
public:
    using value_type = CellType;

    // Iterator that allows modification of cells
    // typename tell us that ::iterator is a type which depending of the type of the array 
    using iterator = typename std::array<CellType, Rows * Columns>::iterator;

    // Iterator that does NOT allow modification of cells
    using const_iterator = typename std::array<CellType, Rows * Columns>::const_iterator;

    static constexpr std::size_t rows() noexcept {
        return Rows;
    }

    static constexpr std::size_t columns() noexcept {
        return Columns;
    }

    [[nodiscard]] constexpr bool contains(Position position) const noexcept{
        if (position.row < Rows && position.column < Columns) {
        return true;
    }

    return false;
    }

    CellType& at(Position position) {
        //La rubrica exige explicitamente que que se validen ambas coordenadas
        if (!contains(position)) {
            throw std::out_of_range("Grid position out of range");
        }
        return cells_[position.row * Columns + position.column];
    }

    const CellType& at(Position position) const {
        //La rubrica exige explicitamente que que se validen ambas coordenadas
        if (!contains(position)) {
            throw std::out_of_range("Grid position out of range");
        }
        return cells_[position.row * Columns + position.column];
    }


    iterator begin() noexcept{
        return cells_.begin();
    }
    iterator end() noexcept{
        return cells_.end();
    }
    const_iterator begin() const noexcept{
        return cells_.begin();
    }
    const_iterator end() const noexcept{
        return cells_.end();
    }
    const_iterator cbegin() const noexcept{
        return cells_.cbegin();
    }
    const_iterator cend() const noexcept{
        return cells_.cend();
    }

};