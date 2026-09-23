#include <array>
#include <cassert>
#include <vector>

#include "circuit_escape/generic_functions.h"


void test_count_matching_vector() {
    std::vector<int> values{1, 2, 3, 4, 5, 6};

    const std::size_t result = countMatching(
        values.begin(),
        values.end(),
        [](int value) {
            return value % 2 == 0;
        }
    );

    assert(result == 3);
}


void test_count_matching_array() {
    std::array<int, 5> values{10, 20, 30, 40, 50};

    const std::size_t result = countMatching(
        values.begin(),
        values.end(),
        [](int value) {
            return value > 25;
        }
    );

    assert(result == 3);
}


void test_count_matching_empty_range() {
    std::vector<int> values;

    const std::size_t result = countMatching(
        values.begin(),
        values.end(),
        [](int value) {
            return value > 0;
        }
    );

    assert(result == 0);
}


void test_find_first_matching() {
    std::vector<int> values{1, 3, 5, 8, 9};

    auto result = LinearSearch(
        values.begin(),
        values.end(),
        [](int value) {
            return value % 2 == 0;
        }
    );

    assert(result != values.end());
    assert(*result == 8);
}


void test_find_first_matching_empty_range() {
    std::array<int, 0> values{};

    auto result = LinearSearch(
        values.begin(),
        values.end(),
        [](int value) {
            return value == 10;
        }
    );

    assert(result == values.end());
}


void run_generic_function_tests() {
    test_count_matching_vector();
    test_count_matching_array();
    test_count_matching_empty_range();
    test_find_first_matching();
    test_find_first_matching_empty_range();
}