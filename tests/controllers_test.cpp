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

int main() {
    test_random_policy_selects_legal_action();
    test_random_policy_single_legal_action();
    test_random_policy_empty_actions_throws();

    return 0;
}