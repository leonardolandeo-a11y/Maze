#pragma once

#include <cstddef>
#include <vector>

class DisjointSet {
public:
    explicit DisjointSet(std::size_t element_count);

    std::size_t find(std::size_t element);
    bool unite(std::size_t first, std::size_t second);

    [[nodiscard]] std::size_t component_count() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::vector<std::size_t> parent_;
    std::vector<std::size_t> sizes_;
    std::size_t component_count_{};
};
