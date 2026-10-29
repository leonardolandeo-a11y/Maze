#include "circuit_escape/graph/disjoint_set.h"

#include <cassert>
#include <iostream>
#include <stdexcept>


void test_disjoint_set_initial_state() {

    DisjointSet sets(5);

    assert(sets.size() == 5);

    assert(sets.set_count() == 5);

    for (std::size_t i = 0; i < 5; ++i) {

        assert(sets.find(i) == i);
    }
}


void test_disjoint_set_union() {

    DisjointSet sets(5);

    const bool merged =
        sets.unite(0, 1);

    assert(merged);

    assert(
        sets.connected(0, 1)
    );

    assert(
        sets.set_count() == 4
    );
}


void test_repeated_union() {

    DisjointSet sets(4);

    assert(
        sets.unite(0, 1)
    );

    assert(
        !sets.unite(0, 1)
    );

    assert(
        sets.set_count() == 3
    );
}


void test_transitive_connectivity() {

    DisjointSet sets(6);

    sets.unite(0, 1);
    sets.unite(1, 2);
    sets.unite(2, 3);

    assert(
        sets.connected(0, 3)
    );

    assert(
        sets.find(0) ==
        sets.find(3)
    );

    assert(
        !sets.connected(0, 5)
    );
}


void test_multiple_components() {

    DisjointSet sets(6);

    sets.unite(0, 1);

    sets.unite(2, 3);

    sets.unite(4, 5);

    assert(
        sets.set_count() == 3
    );

    assert(
        sets.connected(0, 1)
    );

    assert(
        sets.connected(2, 3)
    );

    assert(
        sets.connected(4, 5)
    );

    assert(
        !sets.connected(0, 2)
    );
}


void test_components_can_be_merged() {

    DisjointSet sets(6);

    sets.unite(0, 1);

    sets.unite(2, 3);

    assert(
        sets.set_count() == 4
    );


    sets.unite(1, 2);


    assert(
        sets.connected(0, 3)
    );

    assert(
        sets.set_count() == 3
    );
}


void test_invalid_index() {

    DisjointSet sets(3);

    bool exception_thrown = false;

    try {

        sets.find(3);
    }

    catch (
        const std::out_of_range&
    ) {

        exception_thrown = true;
    }

    assert(exception_thrown);
}


void test_empty_disjoint_set() {

    DisjointSet sets(0);

    assert(
        sets.size() == 0
    );

    assert(
        sets.set_count() == 0
    );
}


void run_disjoint_set_tests() {

    test_disjoint_set_initial_state();

    test_disjoint_set_union();

    test_repeated_union();

    test_transitive_connectivity();

    test_multiple_components();

    test_components_can_be_merged();

    test_invalid_index();

    test_empty_disjoint_set();


    std::cout
        << "All DisjointSet tests passed!\n";
}