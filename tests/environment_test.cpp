#include <cassert>
#include <stdexcept>
#include <variant>
#include "circuit_escape/environment.h"

// Reglas simples para poder controlar fácilmente los resultados de los tests.
GameRules testRules() {
    return GameRules{
        10, // energía inicial
        10, // energía máxima
        10, // límite de turnos
        1,  // costo de movimiento normal
        2,  // costo de terreno elevado
        1,  // costo de wait e intento inválido
        5,  // puntos del recurso
        3,  // recarga de batería
        2,  // penalización de energía de trampa
        1   // penalización de puntos de trampa
    };
}

// Comprueba que el agente pueda moverse por una celda libre.
void test_movimiento_libre() {
    Grid<Cell, 1, 3> grid;
    grid.at({0, 2}) = Exit{};

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 3> environment(
        grid,
        agent,
        testRules()
    );

    StepResult result = environment.step(Action::right);

    assert(result.observation.agent == Position{0, 1});
    assert(result.observation.energy == 9);
    assert(result.observation.turn == 1);
    assert(!result.finished);
    assert(result.reason == EndReason::none);

    // El movimiento debe generar un evento MovedEvent.
    assert(std::holds_alternative<MovedEvent>(result.events[0]));
}

// Comprueba que un muro impida el movimiento.
void test_movimiento_contra_muro() {
    Grid<Cell, 1, 3> grid;
    grid.at({0, 1}) = Wall{};
    grid.at({0, 2}) = Exit{};

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 3> environment(
        grid,
        agent,
        testRules()
    );

    StepResult result = environment.step(Action::right);

    // El agente no debe cambiar de posición.
    assert(result.observation.agent == Position{0, 0});

    // El intento inválido consume energía y un turno.
    assert(result.observation.energy == 9);
    assert(result.observation.turn == 1);

    assert(!result.finished);

    assert(std::holds_alternative<MovementRejectedEvent>(
        result.events.back()
    ));
}

// Comprueba un movimiento que intenta salir del tablero.
void test_movimiento_fuera_del_tablero() {
    Grid<Cell, 2, 2> grid;
    grid.at({1, 1}) = Exit{};

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<2, 2> environment(
        grid,
        agent,
        testRules()
    );

    StepResult result = environment.step(Action::up);

    assert(result.observation.agent == Position{0, 0});
    assert(result.observation.energy == 9);
    assert(result.observation.turn == 1);
    assert(!result.finished);

    assert(std::holds_alternative<MovementRejectedEvent>(
        result.events.back()
    ));
}

// Comprueba el costo configurado para una celda normal.
void test_costo_movimiento_normal() {
    Grid<Cell, 1, 3> grid;
    grid.at({0, 2}) = Exit{};

    GameRules rules = testRules();
    rules.normalCellCost = 3;

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 3> environment(
        grid,
        agent,
        rules
    );

    StepResult result = environment.step(Action::right);

    assert(result.observation.energy == 7);
}

// Comprueba el costo de entrar a terreno elevado.
void test_costo_terreno_elevado() {
    Grid<Cell, 1, 3> grid;
    grid.at({0, 1}) = RoughTerrain{};
    grid.at({0, 2}) = Exit{};

    GameRules rules = testRules();
    rules.roughTerrainCost = 3;

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 3> environment(
        grid,
        agent,
        rules
    );

    StepResult result = environment.step(Action::right);

    assert(result.observation.agent == Position{0, 1});
    assert(result.observation.energy == 7);
}

// Comprueba el costo de esperar.
void test_costo_wait() {
    Grid<Cell, 1, 2> grid;
    grid.at({0, 1}) = Exit{};

    GameRules rules = testRules();
    rules.waitOrInvalidCost = 3;

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 2> environment(
        grid,
        agent,
        rules
    );

    StepResult result = environment.step(Action::wait);

    assert(result.observation.energy == 7);
    assert(result.observation.turn == 1);
}

// Comprueba el costo de un intento de movimiento inválido.
void test_costo_movimiento_invalido() {
    Grid<Cell, 1, 2> grid;
    grid.at({0, 1}) = Exit{};

    GameRules rules = testRules();
    rules.waitOrInvalidCost = 4;

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 2> environment(
        grid,
        agent,
        rules
    );

    StepResult result = environment.step(Action::left);

    assert(result.observation.agent == Position{0, 0});
    assert(result.observation.energy == 6);
    assert(result.observation.turn == 1);
}

// Comprueba que un recurso solo pueda recogerse una vez.
void test_recurso_una_sola_vez() {
    Grid<Cell, 1, 4> grid;
    grid.at({0, 1}) = ResourceCell<int>{100};
    grid.at({0, 3}) = Exit{};

    GameRules rules = testRules();
    rules.resourcePoints = 7;

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 4> environment(
        grid,
        agent,
        rules
    );

    StepResult firstResult = environment.step(Action::right);

    assert(firstResult.observation.score == 7);
    assert(firstResult.observation.collectedResources == 1);

    // Salimos de la celda y luego volvemos a ella.
    environment.step(Action::right);
    StepResult secondResult = environment.step(Action::left);

    // El recurso no debe volver a dar puntos.
    assert(secondResult.observation.score == 7);
    assert(secondResult.observation.collectedResources == 1);
}

// Comprueba que la batería recargue energía al entrar.
void test_bateria() {
    Grid<Cell, 1, 3> grid;
    grid.at({0, 1}) = Battery{};
    grid.at({0, 2}) = Exit{};

    Agent agent({0, 0}, 5, 10);

    NavigationEnvironment<1, 3> environment(
        grid,
        agent,
        testRules()
    );

    StepResult result = environment.step(Action::right);

    // 5 - 1 por entrar + 3 de batería = 7.
    assert(result.observation.energy == 7);
}

// Comprueba que la batería no supere la energía máxima.
void test_bateria_no_supera_maximo() {
    Grid<Cell, 1, 3> grid;
    grid.at({0, 1}) = Battery{};
    grid.at({0, 2}) = Exit{};

    Agent agent({0, 0}, 9, 10);

    NavigationEnvironment<1, 3> environment(
        grid,
        agent,
        testRules()
    );

    StepResult result = environment.step(Action::right);

    // 9 - 1 + 3 = 11, pero el máximo permitido es 10.
    assert(result.observation.energy == 10);
}

// Comprueba que la batería sea consumible.
void test_bateria_consumible() {
    Grid<Cell, 1, 4> grid;
    grid.at({0, 1}) = Battery{};
    grid.at({0, 3}) = Exit{};

    Agent agent({0, 0}, 5, 10);

    NavigationEnvironment<1, 4> environment(
        grid,
        agent,
        testRules()
    );

    StepResult firstResult = environment.step(Action::right);

    // Primera entrada: 5 - 1 + 3 = 7.
    assert(firstResult.observation.energy == 7);

    // Nos alejamos de la batería.
    environment.step(Action::right);

    // Volvemos a la batería.
    StepResult secondResult = environment.step(Action::left);

    // La batería ya fue consumida.
    assert(secondResult.observation.energy == 5);
}

// Comprueba que una trampa pueda activarse varias veces.
void test_trampa_repetida() {
    Grid<Cell, 1, 4> grid;
    grid.at({0, 1}) = Trap{};
    grid.at({0, 3}) = Exit{};

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 4> environment(
        grid,
        agent,
        testRules()
    );

    StepResult firstResult = environment.step(Action::right);

    // 10 - 1 por entrar - 2 de penalización = 7.
    assert(firstResult.observation.energy == 7);
    assert(firstResult.observation.score == -1);

    // Salimos de la trampa.
    environment.step(Action::right);

    // Volvemos a entrar en la trampa.
    StepResult secondResult = environment.step(Action::left);

    assert(secondResult.observation.energy == 3);
    assert(secondResult.observation.score == -2);
}

// Comprueba que la trampa aplique las penalizaciones configuradas.
void test_penalizaciones_trampa() {
    Grid<Cell, 1, 3> grid;
    grid.at({0, 1}) = Trap{};
    grid.at({0, 2}) = Exit{};

    GameRules rules = testRules();
    rules.trapEnergyPenalty = 4;
    rules.trapScorePenalty = 3;

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 3> environment(
        grid,
        agent,
        rules
    );

    StepResult result = environment.step(Action::right);

    // 10 - 1 por entrar - 4 de trampa = 5.
    assert(result.observation.energy == 5);
    assert(result.observation.score == -3);

    bool trapEventFound = false;

    for (const auto& event : result.events) {
        if (std::holds_alternative<TrapTriggeredEvent>(event)) {
            trapEventFound = true;
        }
    }

    assert(trapEventFound);
}

// Comprueba que llegar a la salida termine correctamente.
void test_llegada_a_la_salida() {
    Grid<Cell, 1, 2> grid;
    grid.at({0, 1}) = Exit{};

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 2> environment(
        grid,
        agent,
        testRules()
    );

    StepResult result = environment.step(Action::right);

    assert(result.finished);
    assert(result.reason == EndReason::goalReached);
    assert(environment.isFinished());

    bool goalEventFound = false;

    for (const auto& event : result.events) {
        if (std::holds_alternative<GoalReachedEvent>(event)) {
            goalEventFound = true;
        }
    }

    assert(goalEventFound);
}

// Comprueba la terminación por falta de energía.
void test_terminacion_por_energia() {
    Grid<Cell, 1, 2> grid;
    grid.at({0, 1}) = Exit{};

    Agent agent({0, 0}, 1, 10);

    NavigationEnvironment<1, 2> environment(
        grid,
        agent,
        testRules()
    );

    StepResult result = environment.step(Action::wait);

    assert(result.observation.energy == 0);
    assert(result.finished);
    assert(result.reason == EndReason::noEnergy);
}

// Comprueba la terminación por límite de turnos.
void test_terminacion_por_turnos() {
    Grid<Cell, 1, 2> grid;
    grid.at({0, 1}) = Exit{};

    GameRules rules = testRules();
    rules.turnLimit = 1;

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 2> environment(
        grid,
        agent,
        rules
    );

    StepResult result = environment.step(Action::wait);

    assert(result.observation.turn == 1);
    assert(result.finished);
    assert(result.reason == EndReason::turnLimit);
}

// Comprueba que la falta de energía tenga prioridad sobre el límite de turnos.
void test_precedencia_energia_y_turnos() {
    Grid<Cell, 1, 2> grid;
    grid.at({0, 1}) = Exit{};

    GameRules rules = testRules();
    rules.turnLimit = 1;

    Agent agent({0, 0}, 1, 10);

    NavigationEnvironment<1, 2> environment(
        grid,
        agent,
        rules
    );

    StepResult result = environment.step(Action::wait);

    assert(result.observation.energy == 0);
    assert(result.observation.turn == 1);
    assert(result.finished);
    assert(result.reason == EndReason::noEnergy);
}

// Comprueba que llegar a la salida con energía positiva sea victoria.
void test_precedencia_llegada_a_salida() {
    Grid<Cell, 1, 2> grid;
    grid.at({0, 1}) = Exit{};

    GameRules rules = testRules();
    rules.turnLimit = 1;

    Agent agent({0, 0}, 2, 10);

    NavigationEnvironment<1, 2> environment(
        grid,
        agent,
        rules
    );

    StepResult result = environment.step(Action::right);

    assert(result.observation.energy == 1);
    assert(result.observation.turn == 1);
    assert(result.finished);
    assert(result.reason == EndReason::goalReached);
}

// Comprueba que llegar a la salida con energía cero termine por falta de energía.
void test_salida_con_energia_cero() {
    Grid<Cell, 1, 2> grid;
    grid.at({0, 1}) = Exit{};

    Agent agent({0, 0}, 1, 1);

    NavigationEnvironment<1, 2> environment(
        grid,
        agent,
        testRules()
    );

    StepResult result = environment.step(Action::right);

    assert(result.observation.energy == 0);
    assert(result.finished);
    assert(result.reason == EndReason::noEnergy);
}

// Comprueba las acciones disponibles junto a un muro.
void test_acciones_junto_a_muro() {
    Grid<Cell, 2, 2> grid;

    grid.at({0, 1}) = Wall{};
    grid.at({1, 1}) = Exit{};

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<2, 2> environment(
        grid,
        agent,
        testRules()
    );

    auto actions = environment.