#include <cassert>
#include <variant>

#include "circuit_escape/environment.h"

GameRules interactionTestRules() {
    return GameRules{
        10, // energia inicial
        10, // energia maxima
        20, // limite de turnos
        1,  // costo normal
        2,  // costo de terreno elevado
        1,  // costo de wait
        7,  // puntos por recurso
        3,  // recarga de bateria
        2,  // penalizacion de energia por trampa
        1   // penalizacion de puntaje por trampa
    };
}

// evento producido por un recurso y que la celda quede marcada como recolectada
void test_resource_event_and_collected_state() {
    Grid<Cell, 1, 4> grid;

    grid.at({0, 1}) = ResourceCell<int>{100};
    grid.at({0, 3}) = Exit{};

    Agent agent({0, 0}, 10, 10);

    NavigationEnvironment<1, 4> environment(
        grid,
        agent,
        interactionTestRules()
    );

    StepResult firstResult =
        environment.step(Action::right);

    bool resourceEventFound = false;

    for (const auto& event : firstResult.events) {
        if (const auto* resourceEvent =
                std::get_if<ResourceCollectedEvent>(&event)) {

            resourceEventFound = true;

            assert((resourceEvent->at == Position{0,1}));

            //Informar cantidad de puntos que recibio el agente
            assert(resourceEvent->points == firstResult.observation.score);
        }
    }

    assert(resourceEventFound);

    const auto& resource =
        std::get<ResourceCell<int>>(
            environment.grid().at({0, 1})
        );

    assert(resource.collected);

    // Salimos de la celda del recurso
    [[maybe_unused]] const StepResult moveAwayResult =
    environment.step(Action::right);

    // Volvemos al mismo recurso
    [[maybe_unused]] const StepResult secondResult =
        environment.step(Action::left);

    bool secondResourceEventFound = false;

    for (const auto& event : secondResult.events) {
        if (std::holds_alternative<ResourceCollectedEvent>(event)) {
            secondResourceEventFound = true;
        }
    }

    // Un recurso ya recolectado no genera otro evento de recoleccion
    assert(!secondResourceEventFound);
}

//Comprueba eventos de energia por bateria y celda marcada como consumida
void test_battery_event_and_consumed_state() {
    Grid<Cell, 1, 4> grid;

    grid.at({0, 1}) = Battery{};
    grid.at({0, 3}) = Exit{};

    Agent agent({0, 0}, 5, 10);

    NavigationEnvironment<1, 4> environment(grid, agent, interactionTestRules());

    StepResult firstResult = environment.step(Action::right);

    bool movementEnergyEventFound = false;
    bool rechargeEventFound = false;

    for (const auto& event : firstResult.events) {
        if (const auto* energyEvent = std::get_if<EnergyChangedEvent>(&event)) {

            //Costo de movimiento
            if (energyEvent->previous == 5 && energyEvent->current == 4) {
                movementEnergyEventFound = true;
            }

            //Recarga de bateria
            if (energyEvent->previous == 4 && energyEvent->current == 7) {
                rechargeEventFound = true;
            }
        }
    }

    assert(movementEnergyEventFound);
    assert(rechargeEventFound);

    const auto& battery = std::get<Battery>(environment.grid().at({0, 1}));

    assert(battery.consumed);

    //Salimos de la bateria
    [[maybe_unused]] const StepResult moveAwayResult = environment.step(Action::right);

    //Volvemos a la bateria ya consumida
    StepResult secondResult = environment.step(Action::left);

    bool secondRechargeEventFound = false;

    for (const auto& event : secondResult.events) {
        if (const auto* energyEvent = std::get_if<EnergyChangedEvent>(&event)) {

            //Si current > previous entonces hubo nueva recarga
            if (energyEvent->current > energyEvent->previous) {
                secondRechargeEventFound = true;
            }
        }
    }

    assert(!secondRechargeEventFound);
}

int main() {
    test_resource_event_and_collected_state();
    test_battery_event_and_consumed_state();

    return 0;
}
