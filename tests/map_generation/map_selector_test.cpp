#include "circuit_escape/map_generation/map_selector.h"


#include <cassert>
#include <iostream>
#include <vector>

void test_empty_collection() {
    std::vector<MapCandidate> candidates;

    auto result = select_maps(candidates);

    assert(!result.has_value());
}


void test_no_verified_winning_maps() {
    std::vector<MapCandidate> candidates = {
        {1, 10, false},
        {2, 18, false},
        {3, 32, false}
    };

    auto result = select_maps(candidates);

    assert(!result.has_value());
}


void test_input_not_modified() {
    std::vector<MapCandidate> candidates = {
        {1, 10, true},
        {2, 18, false},
        {3, 32, true}
    };

    const auto original = candidates;

    select_maps(candidates);

    assert(candidates.size() == original.size());

    for (std::size_t i = 0; i < candidates.size(); ++i) {
        assert(candidates[i].id == original[i].id);

        assert(
            candidates[i].minimum_energy_cost ==
            original[i].minimum_energy_cost
        );

        assert(
            candidates[i].verified_win ==
            original[i].verified_win
        );
    }
}


void run_map_selector_tests() {
    test_empty_collection();
    test_no_verified_winning_maps();
    test_input_not_modified();

    std::cout << "All map selector tests passed!\n";
}