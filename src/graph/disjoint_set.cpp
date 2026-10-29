#include "circuit_escape/graph/disjoint_set.h"

#include <numeric>
#include <stdexcept>


DisjointSet::DisjointSet(
    std::size_t size
)
    : parent_(size),
      rank_(size, 0),
      set_count_(size)
{
    std::iota(
        parent_.begin(),
        parent_.end(),
        0
    );
}


void DisjointSet::validate_index(
    std::size_t value
) const {

    if (value >= parent_.size()) {

        throw std::out_of_range(
            "DisjointSet index out of range"
        );
    }
}


std::size_t DisjointSet::find(
    std::size_t value
) {

    validate_index(value);


    if (parent_[value] != value) {

        parent_[value] =
            find(parent_[value]);
    }


    return parent_[value];
}


bool DisjointSet::unite(
    std::size_t first,
    std::size_t second
) {

    const std::size_t first_root =
        find(first);

    const std::size_t second_root =
        find(second);


    if (first_root == second_root) {
        return false;
    }


    if (
        rank_[first_root] <
        rank_[second_root]
    ) {

        parent_[first_root] =
            second_root;
    }

    else if (
        rank_[first_root] >
        rank_[second_root]
    ) {

        parent_[second_root] =
            first_root;
    }

    else {

        parent_[second_root] =
            first_root;

        ++rank_[first_root];
    }


    --set_count_;

    return true;
}


bool DisjointSet::connected(
    std::size_t first,
    std::size_t second
) {

    return find(first) == find(second);
}


std::size_t DisjointSet::size() const noexcept {

    return parent_.size();
}


std::size_t
DisjointSet::set_count() const noexcept {

    return set_count_;
}