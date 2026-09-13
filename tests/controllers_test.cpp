#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <vector>

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

int main() {
    test_random_policy_selects_legal_action();
    test_random_policy_single_legal_action();
    test_random_policy_empty_actions_throws();
    test_random_policy_same_seed_same_sequence();

    test_heuristic_policy_moves_closer_to_goal();
    test_heuristic_policy_keeps_first_action_on_tie();
    test_heuristic_policy_empty_actions_throws();

    return 0;
}