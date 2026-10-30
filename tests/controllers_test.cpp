#include <algorithm>
#include <array>
#include <cstddef>
#include <utility>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <memory>

#include "circuit_escape/controllers.h"

// Comprueba que RandomPolicy este dentro del conjunto de acciones legales recibido
void test_random_policy_selects_legal_action() {
    RandomPolicy policy(12345);

    Observation observation{};

    std::vector<Action> legalActions{
        Action::up,
        Action::right,
        Action::wait
    };

    // Ejecutamos varias selecciones para comprobar que ninguno este fuera de lo permitido
    for (int i = 0; i < 100; ++i) {
        const Action selectedAction =
            policy.selectAction(observation, legalActions);

        assert(
            std::find(
                legalActions.begin(),
                legalActions.end(),
                selectedAction
            ) != legalActions.end()
        );
    }
}

// cuando solo existe una accion legal RandomPolicy siempre devuelva esa unica accion
void test_random_policy_single_legal_action() {
    RandomPolicy policy(54321);

    Observation observation{};

    std::vector<Action> legalActions{
        Action::left
    };

    for (int i = 0; i < 10; ++i) {
        assert(
            policy.selectAction(observation, legalActions)
            == Action::left
        );
    }
}

// Comprueba que RandomPolicy rechace un conjunto vacio de acciones legales
void test_random_policy_empty_actions_throws() {
    RandomPolicy policy(100);

    Observation observation{};
    std::vector<Action> legalActions;

    bool exceptionThrown = false;

    try {
        policy.selectAction(observation, legalActions);
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    assert(exceptionThrown);
}

// Comprueba que dos RandomPolicy inicializadas con la misma semilla haga o tenga una misma secuencia
void test_random_policy_same_seed_same_sequence() {
    RandomPolicy firstPolicy(2026);
    RandomPolicy secondPolicy(2026);

    Observation observation{};

    std::vector<Action> legalActions{
        Action::up,
        Action::down,
        Action::left,
        Action::right,
        Action::wait
    };

    for (int i = 0; i < 100; ++i) {
        const Action firstAction =
            firstPolicy.selectAction(observation, legalActions);

        const Action secondAction =
            secondPolicy.selectAction(observation, legalActions);

        assert(firstAction == secondAction);
    }
}

// Comprueba que HeuristicPolicy seleccione la accion que deja al agente mas cerca de la meta usando Manhattan
void test_heuristic_policy_moves_closer_to_goal() {
    HeuristicPolicy policy;

    Observation observation{};
    observation.agent = Position{2, 2};
    observation.goal = Position{2, 4};

    std::vector<Action> legalActions{
        Action::up,
        Action::left,
        Action::right,
        Action::wait
    };

    const Action selectedAction =
        policy.selectAction(observation, legalActions);

    assert(selectedAction == Action::right);
}

// Comprueba que, si dos acciones producen la misma distancia, Heuristic conserve la mejor accion recibida pero la primera
void test_heuristic_policy_keeps_first_action_on_tie() {
    HeuristicPolicy policy;

    Observation observation{};
    observation.agent = Position{1, 1};
    observation.goal = Position{0, 0};

    std::vector<Action> legalActions{
        Action::up,
        Action::left
    };

    const Action selectedAction =
        policy.selectAction(observation, legalActions);

    assert(selectedAction == Action::up);
}

//Comprueba que Heuristic rechace un conjunto vacio de acciones legales
void test_heuristic_policy_empty_actions_throws() {
    HeuristicPolicy policy;

    Observation observation{};
    std::vector<Action> legalActions;

    bool exceptionThrown = false;

    try {
        policy.selectAction(observation, legalActions);
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    assert(exceptionThrown);
}

// Comprobar que Policy delegue correcatmente la seleccion de acciones
struct FixedActionPolicy {
    Action action;

    Action selectAction(const Observation&, std::span<const Action>) {
            return action;
    }
};

//Comprueba tiempo de compilacion y que cumpla con el contrato definido por Navigation
static_assert(NavigationPolicy<RandomPolicy>);
static_assert(NavigationPolicy<HeuristicPolicy>);
static_assert(NavigationPolicy<FixedActionPolicy>);

struct InvalidPolicy {};

static_assert(!NavigationPolicy<InvalidPolicy>);

// Compueba que Policy delegue la seleccion de la accion a la politica que lo contiene
void test_policy_controller_delegates_to_policy() {
    FixedActionPolicy policy{
        Action::left,
    };

    PolicyController<FixedActionPolicy> controller{policy};

    Observation observation{};

    std::vector<Action> legalActions{
        Action::left,
        Action::right,
        Action::wait
    };

    const Action selectedAction = controller.selectAction(observation, legalActions);

    assert(selectedAction == Action::left);
}

// Comprueba que Policy pueda utilizarse mediante la interfaz IController
void test_policy_controller_through_icontroller() {
    PolicyController<FixedActionPolicy> controller{
        FixedActionPolicy{Action::right}
    };

    IController& controllerInterface = controller;

    Observation observation{};

    std::vector<Action> legalActions{
        Action::left,
        Action::right
    };

    const Action selectedAction =
        controllerInterface.selectAction(
            observation,
            legalActions
        );

    assert(selectedAction == Action::right);
}

//Comprueba la implementación de std::unique_ptr con la interfaz polimorfica IController
void test_policy_controller_through_unique_ptr() {
    std::unique_ptr<IController> controller =
        std::make_unique<PolicyController<FixedActionPolicy>>(
            FixedActionPolicy{
                Action::right
            }
        );

    Observation observation{};

    std::vector<Action> legalActions{
        Action::left,
        Action::right
    };

    const Action selectedAction =
        controller->selectAction(
            observation,
            legalActions
        );

    assert(selectedAction == Action::right);
}


namespace {

GraphData controller_test_graph(std::size_t rows, std::size_t columns) {
    GraphData graph;
    graph.rows = rows;
    graph.columns = columns;
    graph.neighbors.resize(rows * columns);
    graph.blocked.assign(rows * columns, false);
    graph.entry_costs.assign(rows * columns, 1);
    constexpr std::array<Action, 4> directions{
        Action::down, Action::right, Action::left, Action::up
    };
    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t column = 0; column < columns; ++column) {
            for (Action action : directions) {
                auto next = neighbor({row, column}, action);
                if (next && next->row < rows && next->column < columns) {
                    graph.neighbors[row * columns + column].push_back(
                        next->row * columns + next->column
                    );
                }
            }
        }
    }
    return graph;
}

void test_guard_bfs_shortest_path_and_tie_break() {
    auto graph = controller_test_graph(3, 3);
    PolicyController<GuardBfsPolicy> controller{GuardBfsPolicy{graph}};
    Observation observation{};
    observation.agent = {1, 1};
    observation.goal = {0, 0};
    const std::vector<Action> legal{Action::down, Action::right, Action::left, Action::up, Action::wait};
    // Reversed adjacency insertion does not affect the documented order.
    assert(controller.selectAction(observation, legal) == Action::up);
    observation.goal = {2, 2};
    assert(controller.selectAction(observation, legal) == Action::right);
    observation.agent = observation.goal;
    assert(controller.selectAction(observation, legal) == Action::wait);
}

void test_guard_bfs_detours_and_legal_actions() {
    auto graph = controller_test_graph(3, 5);
    graph.blocked[2 * graph.columns + 1] = true;
    GuardBfsPolicy policy(graph);
    Observation observation{};
    observation.agent = {2, 0};
    observation.goal = {2, 4};
    const std::vector<Action> legal{Action::up, Action::wait};
    assert(policy.selectAction(observation, legal) == Action::up);

    auto open_graph = controller_test_graph(3, 3);
    GuardBfsPolicy restricted(open_graph);
    observation.agent = {1, 1};
    observation.goal = {0, 0};
    const std::vector<Action> left_only{Action::left, Action::wait};
    assert(restricted.selectAction(observation, left_only) == Action::left);
}

void test_guard_bfs_disconnected_and_invalid() {
    auto graph = controller_test_graph(3, 3);
    for (std::size_t column = 0; column < 3; ++column) {
        graph.blocked[3 + column] = true;
    }
    GuardBfsPolicy policy(graph);
    Observation observation{};
    observation.agent = {2, 1};
    observation.goal = {0, 1};
    const std::vector<Action> allowed{Action::left, Action::right, Action::wait};
    assert(policy.selectAction(observation, allowed) == Action::wait);

    bool threw = false;
    try { (void)policy.selectAction(observation, {}); }
    catch (const std::invalid_argument&) { threw = true; }
    assert(threw);

    threw = false;
    try { (void)policy.selectAction(observation, std::array{Action::left}); }
    catch (const std::invalid_argument&) { threw = true; }
    assert(threw);

    observation.goal = {9, 9};
    threw = false;
    try { (void)policy.selectAction(observation, allowed); }
    catch (const std::out_of_range&) { threw = true; }
    assert(threw);
}

void test_guard_bfs_dynamic_map_dimensions_and_icontroller() {
    for (auto [rows, columns] : {
        std::pair<std::size_t, std::size_t>{10, 15},
        {20, 30}, {40, 60}
    }) {
        auto graph = controller_test_graph(rows, columns);
        std::unique_ptr<IController> controller =
            std::make_unique<PolicyController<GuardBfsPolicy>>(GuardBfsPolicy{graph});
        Observation observation{};
        observation.agent = {rows - 1, columns - 1};
        observation.goal = {0, 0};
        const std::vector<Action> allowed{Action::up, Action::left, Action::wait};
        assert(controller->selectAction(observation, allowed) == Action::up);
    }
}

}  // namespace


void run_controller_tests() {
    test_random_policy_selects_legal_action();
    test_random_policy_single_legal_action();
    test_random_policy_empty_actions_throws();
    test_random_policy_same_seed_same_sequence();

    test_heuristic_policy_moves_closer_to_goal();
    test_heuristic_policy_keeps_first_action_on_tie();
    test_heuristic_policy_empty_actions_throws();

    test_policy_controller_delegates_to_policy();
    test_policy_controller_through_icontroller();
    test_policy_controller_through_unique_ptr();

    test_guard_bfs_shortest_path_and_tie_break();
    test_guard_bfs_detours_and_legal_actions();
    test_guard_bfs_disconnected_and_invalid();
    test_guard_bfs_dynamic_map_dimensions_and_icontroller();
}