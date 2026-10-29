#include "circuit_escape/map_generation/map_selector.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>


void test_empty_collection() {

    std::vector<MapCandidate> candidates;

    const auto result =
        select_maps(candidates);

    assert(!result.has_value());
}


void test_no_verified_winning_maps() {

    const std::vector<MapCandidate> candidates = {
        {1, 10, false},
        {2, 18, false},
        {3, 32, false}
    };

    const auto result =
        select_maps(candidates);

    assert(!result.has_value());
}


void test_basic_selection() {

    const std::vector<MapCandidate> candidates = {
        {1, 10, true},
        {2, 18, true},
        {3, 32, true}
    };


    const auto result =
        select_maps(candidates);


    assert(result.has_value());

    assert(result->easy_id == 1);
    assert(result->medium_id == 2);
    assert(result->hard_id == 3);
}


void test_unverified_maps_are_ignored() {

    const std::vector<MapCandidate> candidates = {

        // Should not participate
        {1, 1, false},

        {2, 10, true},
        {3, 18, true},
        {4, 32, true},

        // Should not participate
        {5, 100, false}
    };


    const auto result =
        select_maps(candidates);


    assert(result.has_value());

    assert(result->easy_id == 2);
    assert(result->medium_id == 3);
    assert(result->hard_id == 4);
}


void test_less_than_three_distinct_costs() {

    const std::vector<MapCandidate> candidates = {
        {1, 10, true},
        {2, 10, true},
        {3, 20, true},
        {4, 20, true}
    };


    const auto result =
        select_maps(candidates);


    assert(!result.has_value());
}


void test_minimum_and_maximum_ties_use_smallest_id() {

    const std::vector<MapCandidate> candidates = {

        {8, 10, true},
        {2, 10, true},

        {5, 20, true},

        {9, 30, true},
        {4, 30, true}
    };


    const auto result =
        select_maps(candidates);


    assert(result.has_value());

    // Cost 10 → IDs 8 and 2
    assert(result->easy_id == 2);

    assert(result->medium_id == 5);

    // Cost 30 → IDs 9 and 4
    assert(result->hard_id == 4);
}


void test_medium_tie_uses_smallest_id() {

    const std::vector<MapCandidate> candidates = {

        {1, 10, true},

        // Both are equally distant from average = 20
        {8, 15, true},
        {3, 25, true},

        {4, 30, true}
    };


    const auto result =
        select_maps(candidates);


    assert(result.has_value());

    assert(result->easy_id == 1);

    // Distance:
    //
    // 15 -> |15 - 20| = 5
    // 25 -> |25 - 20| = 5
    //
    // ID 3 < ID 8

    assert(result->medium_id == 3);

    assert(result->hard_id == 4);
}


void test_input_order_does_not_change_result() {

    const std::vector<MapCandidate> first = {
        {1, 10, true},
        {2, 18, true},
        {3, 32, true},
        {4, 21, true}
    };


    const std::vector<MapCandidate> second = {
        {4, 21, true},
        {3, 32, true},
        {2, 18, true},
        {1, 10, true}
    };


    const auto first_result =
        select_maps(first);

    const auto second_result =
        select_maps(second);


    assert(first_result.has_value());
    assert(second_result.has_value());


    assert(
        first_result->easy_id ==
        second_result->easy_id
    );

    assert(
        first_result->medium_id ==
        second_result->medium_id
    );

    assert(
        first_result->hard_id ==
        second_result->hard_id
    );
}


void test_input_not_modified() {

    std::vector<MapCandidate> candidates = {
        {1, 10, true},
        {2, 18, false},
        {3, 32, true},
        {4, 20, true}
    };


    const auto original =
        candidates;


    select_maps(candidates);


    assert(
        candidates.size() ==
        original.size()
    );


    for (
        std::size_t i = 0;
        i < candidates.size();
        ++i
    ) {

        assert(
            candidates[i].id ==
            original[i].id
        );

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

void test_average_preserves_decimal_part() {

    const std::vector<MapCandidate> candidates = {

        {1, 0, true},

        // Average = (0 + 4 + 5 + 9) / 4 = 4.5
        //
        // Cost 4 and cost 5 are equally distant.
        // Therefore the tie must be resolved by ID.
        //
        // ID 2 belongs to cost 5,
        // so Medium must be map 2.

        {8, 4, true},
        {2, 5, true},

        {3, 9, true}
    };


    const auto result =
        select_maps(candidates);


    assert(result.has_value());

    assert(result->easy_id == 1);
    assert(result->medium_id == 2);
    assert(result->hard_id == 3);
}
void test_duplicate_ids_are_rejected() {

    const std::vector<MapCandidate> candidates = {
        {1, 10, true},
        {2, 18, true},
        {2, 25, true},
        {3, 32, true}
    };

    const auto result =
        select_maps(candidates);

    assert(!result.has_value());
}
void test_negative_verified_cost_is_rejected() {

    const std::vector<MapCandidate> candidates = {
        {1, 10, true},
        {2, -5, true},
        {3, 20, true},
        {4, 30, true}
    };

    const auto result =
        select_maps(candidates);

    assert(!result.has_value());
}

void test_negative_cost_on_unverified_map_is_rejected() {

    const std::vector<MapCandidate> candidates = {
        {1, -100, false},

        {2, 10, true},
        {3, 18, true},
        {4, 32, true}
    };

    const auto result =
        select_maps(candidates);

    assert(!result.has_value());
}

void run_map_selector_tests() {

    test_empty_collection();

    test_no_verified_winning_maps();

    test_basic_selection();

    test_unverified_maps_are_ignored();

    test_less_than_three_distinct_costs();

    test_minimum_and_maximum_ties_use_smallest_id();

    test_medium_tie_uses_smallest_id();

    test_input_order_does_not_change_result();

    test_input_not_modified();

    test_average_preserves_decimal_part();

    test_duplicate_ids_are_rejected();

    test_negative_verified_cost_is_rejected();

    test_negative_cost_on_unverified_map_is_rejected();
    std::cout
        << "All map selector tests passed!\n";
}