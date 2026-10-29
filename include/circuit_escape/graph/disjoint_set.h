#pragma once

#include <cstddef>
#include <vector>


class DisjointSet {

public:

    explicit DisjointSet(std::size_t size);

    std::size_t find(std::size_t value);

    bool unite(
        std::size_t first,
        std::size_t second
    );

    bool connected(
        std::size_t first,
        std::size_t second
    );

    std::size_t size() const noexcept;

    std::size_t set_count() const noexcept;


private:

    std::vector<std::size_t> parent_;

    std::vector<std::size_t> rank_;

    std::size_t set_count_{};

    void validate_index(
        std::size_t value
    ) const;
};