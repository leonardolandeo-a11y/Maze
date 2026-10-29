#include "circuit_escape/graph/disjoint_set.h"

#include <numeric>
#include <stdexcept>
#include <utility>

DisjointSet::DisjointSet(std::size_t element_count)
    : parent_(element_count), sizes_(element_count, 1), component_count_(element_count) {
    std::iota(parent_.begin(), parent_.end(), std::size_t{0});
}

std::size_t DisjointSet::find(std::size_t element) {
    if (element >= parent_.size()) {
        throw std::out_of_range("DisjointSet element out of range");
    }

    while (element != parent_[element]) {
        parent_[element] = parent_[parent_[element]];
        element = parent_[element];
    }
    return element;
}

bool DisjointSet::unite(std::size_t first, std::size_t second) {
    first = find(first);
    second = find(second);
    if (first == second) {
        return false;
    }
    if (sizes_[first] < sizes_[second]) {
        std::swap(first, second);
    }
    parent_[second] = first;
    sizes_[first] += sizes_[second];
    --component_count_;
    return true;
}

std::size_t DisjointSet::component_count() const noexcept {
    return component_count_;
}

std::size_t DisjointSet::size() const noexcept {
    return parent_.size();
}

bool DisjointSet::connected(std::size_t first, std::size_t second) {
    return find(first) == find(second);
}

std::size_t DisjointSet::set_count() const noexcept {
    return component_count_;
}