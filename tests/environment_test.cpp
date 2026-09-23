#include <cassert>
#include <stdexcept>
#include <variant>
#include <algorithm>
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

    assert((result.observation.agent == Position{0, 1}));
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
    assert((result.observation.agent == Position{0, 0}));

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

    assert((result.observation.agent == Position{0, 0}));
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

    assert((result.observation.agent == Position{0, 1}));
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

    assert((result.observation.agent == Position{0, 0}));
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
    (void)environment.step(Action::right);
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
    (void)environment.step(Action::right);

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
    (void)environment.step(Action::right);

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

    auto actions = environment.availableActions();

    assert(actions.size() == 2);
    assert(std::find(actions.begin(), actions.end(), Action::down) != actions.end());
    assert(std::find(actions.begin(), actions.end(), Action::wait) != actions.end());
}

// Comprueba las acciones disponibles en una esquina.
void test_acciones_en_esquina() {
    Grid<Cell, 2, 2> grid;

    grid.at({1, 1}) = Exit{};

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<2, 2> environment(
        grid,
        agent,
        testRules()
    );

    auto actions = environment.availableActions();

    assert(actions.size() == 3);
    assert(std::find(actions.begin(), actions.end(), Action::down) != actions.end());
    assert(std::find(actions.begin(), actions.end(), Action::right) != actions.end());
    assert(std::find(actions.begin(), actions.end(), Action::wait) != actions.end());
}

// Comprueba que no haya acciones disponibles después de terminar.
void test_acciones_despues_del_termino() {
    Grid<Cell, 1, 2> grid;

    grid.at({0, 1}) = Exit{};

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 2> environment(
        grid,
        agent,
        testRules()
    );

    (void)environment.step(Action::right);

    assert(environment.isFinished());

    auto actions = environment.availableActions();

    assert(actions.empty());
}

void assert_resource_points(
    const GameRules& rules,
    int expectedPoints
) {
    Grid<Cell, 1, 3> grid;

    // Ponemos un reward distinto a propósito.
    // El puntaje debe salir de GameRules.
    grid.at({0, 1}) = ResourceCell<int>{100};

    // El Environment necesita una salida.
    grid.at({0, 2}) = Exit{};

    Agent agent(
        {0, 0},
        rules.initialEnergy,
        rules.maximumEnergy
    );

    NavigationEnvironment<1, 3> environment(
        grid,
        agent,
        rules
    );

    StepResult result =
        environment.step(Action::right);

    // Verifica el score del jugador.
    assert(
        result.observation.score == expectedPoints
    );

    // Verifica también ResourceCollectedEvent.
    bool eventFound = false;

    for (const auto& event : result.events) {

        if (
            const auto* resourceEvent =
                std::get_if<ResourceCollectedEvent>(&event)
        ) {
            eventFound = true;

            assert(
                resourceEvent->points == expectedPoints
            );
        }
    }

    assert(eventFound);
}

void test_resource_points_by_difficulty() {

    GameRules easyRules =
        rulesFor(Difficulty::easy);

    GameRules standardRules =
        rulesFor(Difficulty::standard);

    GameRules hardRules =
        rulesFor(Difficulty::hard);


    assert(easyRules.resourcePoints == 15);
    assert(standardRules.resourcePoints == 10);
    assert(hardRules.resourcePoints == 8);


    assert_resource_points(
        easyRules,
        15
    );

    assert_resource_points(
        standardRules,
        10
    );

    assert_resource_points(
        hardRules,
        8
    );
}
//Test para probar la correcta carga de configuraciones por dificultad
void test_difficulty_profiles() {

    GameRules easy = rulesFor(Difficulty::easy);

    assert(easy.initialEnergy == 80);
    assert(easy.maximumEnergy == 80);
    assert(easy.turnLimit == 240);
    assert(easy.normalCellCost == 1);
    assert(easy.roughTerrainCost == 2);
    assert(easy.waitOrInvalidCost == 1);
    assert(easy.resourcePoints == 15);
    assert(easy.batteryRecharge == 5);
    assert(easy.trapEnergyPenalty == 1);
    assert(easy.trapScorePenalty == 0);


    GameRules standard = rulesFor(Difficulty::standard);

    assert(standard.initialEnergy == 60);
    assert(standard.maximumEnergy == 60);
    assert(standard.turnLimit == 180);
    assert(standard.normalCellCost == 1);
    assert(standard.roughTerrainCost == 2);
    assert(standard.waitOrInvalidCost == 1);
    assert(standard.resourcePoints == 10);
    assert(standard.batteryRecharge == 3);
    assert(standard.trapEnergyPenalty == 2);
    assert(standard.trapScorePenalty == 1);


    GameRules hard = rulesFor(Difficulty::hard);

    assert(hard.initialEnergy == 40);
    assert(hard.maximumEnergy == 40);
    assert(hard.turnLimit == 140);
    assert(hard.normalCellCost == 1);
    assert(hard.roughTerrainCost == 3);
    assert(hard.waitOrInvalidCost == 1);
    assert(hard.resourcePoints == 8);
    assert(hard.batteryRecharge == 2);
    assert(hard.trapEnergyPenalty == 3);
    assert(hard.trapScorePenalty == 2);
}

void run_environment_tests() {
    test_movimiento_libre();
    test_movimiento_contra_muro();
    test_movimiento_fuera_del_tablero();

    test_costo_movimiento_normal();
    test_costo_terreno_elevado();
    test_costo_wait();
    test_costo_movimiento_invalido();

    test_recurso_una_sola_vez();

    test_bateria();
    test_bateria_no_supera_maximo();
    test_bateria_consumible();

    test_trampa_repetida();
    test_penalizaciones_trampa();

    test_llegada_a_la_salida();
    test_terminacion_por_energia();
    test_terminacion_por_turnos();

    test_precedencia_energia_y_turnos();
    test_precedencia_llegada_a_salida();
    test_salida_con_energia_cero();

    test_acciones_junto_a_muro();
    test_acciones_en_esquina();
    test_acciones_despues_del_termino();
    //test agregado para verificar si resource points es controlado por difficulty
    test_resource_points_by_difficulty();
    //otro test
    test_difficulty_profiles();
}