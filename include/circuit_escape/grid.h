#pragma once
#include <cstddef>
#include <array>
#include <limits>
#include <vector>
#include "types.h"
#include <stdexcept>


template<typename CellType, std::size_t Rows = 0, std::size_t Columns = 0>
class Grid {

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

        if (!contains(position)) {
            throw std::out_of_range("Grid position out of range");
        }
        return cells_[position.row * Columns + position.column];
    }

    const CellType& at(Position position) const {

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

template<typename CellType>
class Grid<CellType, 0, 0> {
private:
    std::size_t rows_{};
    std::size_t columns_{};
    std::vector<CellType> cells_;

public:
    using value_type = CellType;
    using iterator = typename std::vector<CellType>::iterator;
    using const_iterator = typename std::vector<CellType>::const_iterator;

    Grid(std::size_t rows, std::size_t columns)
        : rows_(rows), columns_(columns) {
        if (rows == 0 || columns == 0 ||
            rows > std::numeric_limits<std::size_t>::max() / columns) {
            throw std::invalid_argument("Invalid grid dimensions");
        }

        const std::size_t cell_count = rows * columns;
        if (cell_count > cells_.max_size()) {
            throw std::length_error("Grid dimensions exceed supported capacity");
        }
        cells_.resize(cell_count);
    }

    [[nodiscard]] std::size_t rows() const noexcept { return rows_; }
    [[nodiscard]] std::size_t columns() const noexcept { return columns_; }

    [[nodiscard]] bool contains(Position position) const noexcept {
        return position.row < rows_ && position.column < columns_;
    }

    CellType& at(Position position) {
        if (!contains(position)) {
            throw std::out_of_range("Grid position out of range");
        }
        return cells_[position.row * columns_ + position.column];
    }

    const CellType& at(Position position) const {
        if (!contains(position)) {
            throw std::out_of_range("Grid position out of range");
        }
        return cells_[position.row * columns_ + position.column];
    }

    iterator begin() noexcept { return cells_.begin(); }
    iterator end() noexcept { return cells_.end(); }
    const_iterator begin() const noexcept { return cells_.begin(); }
    const_iterator end() const noexcept { return cells_.end(); }
    const_iterator cbegin() const noexcept { return cells_.cbegin(); }
    const_iterator cend() const noexcept { return cells_.cend(); }
};
