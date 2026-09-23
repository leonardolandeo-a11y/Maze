#include <cassert>
#include <variant>

#include "circuit_escape/environment.h"


GameRules eventTestRules() {
    return GameRules{
        10, // initialEnergy
        10, // maximumEnergy
        20, // turnLimit
        1,  // normalCellCost
        2,  // roughTerrainCost
        1,  // waitOrInvalidCost
        5,  // resourcePoints
        3,  // batteryRecharge
        2,  // trapEnergyPenalty
        1   // trapScorePenalty
    };
}

void test_moved_event() {
    Grid<Cell, 1, 3> grid;

    grid.at({0, 2}) = Exit{};

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 3> environment(
        grid,
        agent,
        eventTestRules()
    );

    StepResult result =
        environment.step(Action::right);

    bool found = false;

    for (const auto& event : result.events) {

        if (const auto* moved = std::get_if<MovedEvent>(&event)) {
            found = true;

            assert((moved->from == Position{0, 0}));
            assert((moved->to == Position{0, 1}));
            assert(moved->energyCost == 1);
        }
    }

    assert(found);
}

void test_movement_rejected_event() {
    Grid<Cell, 1, 3> grid;

    grid.at({0, 1}) = Wall{};
    grid.at({0, 2}) = Exit{};

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 3> environment(
        grid,
        agent,
        eventTestRules()
    );

    StepResult result =
        environment.step(Action::right);

    bool found = false;

    for (const auto& event : result.events) {

        if (const auto* rejected = std::get_if<MovementRejectedEvent>(&event)) {
            found = true;

            assert((rejected->from == Position{0, 0}));
            assert(rejected->action == Action::right);
        }
    }

    assert(found);
}

void test_resource_collected_event() {
    Grid<Cell, 1, 3> grid;

    grid.at({0, 1}) =
        ResourceCell<int>{10, false};

    grid.at({0, 2}) = Exit{};

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 3> environment(
        grid,
        agent,
        eventTestRules()
    );

    StepResult result =
        environment.step(Action::right);

    bool found = false;

    for (const auto& event : result.events) {

        if (const auto* resource = std::get_if<ResourceCollectedEvent>(&event)) {
            found = true;

            assert((resource->at == Position{0, 1}));
            assert(resource->points == 5);
        }
    }

    assert(found);
}

void test_energy_changed_event() {
    Grid<Cell, 1, 2> grid;
    grid.at({0, 1}) = Exit{};

    Agent agent({0, 0}, 10, 10);
    NavigationEnvironment<1, 2> environment(
        grid,
        agent,
        eventTestRules()
    );

    StepResult result =
        environment.step(Action::wait);

    bool found = false;

    for (const auto& event : result.events) {

        if (const auto* energy = std::get_if<EnergyChangedEvent>(&event)) {
            found = true;

            assert(energy->previous == 10);
            assert(energy->current == 9);
        }
    }

    assert(found);
}

void test_trap_triggered_event() {
    Grid<Cell, 1, 3> grid;

    grid.at({0, 1}) = Trap{};
    grid.at({0, 2}) = Exit{};

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 3> environment(
        grid,
        agent,
        eventTestRules()
    );

    StepResult result =
        environment.step(Action::right);

    bool found = false;

    for (const auto& event : result.events) {

        if (const auto* trap = std::get_if<TrapTriggeredEvent>(&event)) {
            found = true;

            assert((trap->at == Position{0, 1}));
        }
    }

    assert(found);
}

void test_goal_reached_event() {
    Grid<Cell, 1, 2> grid;
    grid.at({0, 1}) = Exit{};

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 2> environment(
        grid,
        agent,
        eventTestRules()
    );

    StepResult result =
        environment.step(Action::right);

    bool found = false;

    for (const auto& event : result.events) {

        if (
            const auto* goal =
                std::get_if<GoalReachedEvent>(&event)
        ) {
            found = true;

            assert((goal->at == Position{0, 1}));
        }
    }

    assert(found);
    assert(result.finished);
    assert(result.reason == EndReason::goalReached);
}

void run_event_tests() {
    test_moved_event();
    test_movement_rejected_event();
    test_resource_collected_event();
    test_energy_changed_event();
    test_trap_triggered_event();
    test_goal_reached_event();
}