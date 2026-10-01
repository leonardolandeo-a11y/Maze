#pragma once

#include "circuit_escape/controllers.h"
#include "circuit_escape/environment.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <memory>

struct SimulationResult {
    std::vector<Action> actions;
    EndReason reason{EndReason::none};
    std::size_t turns{};
    int score{};
    int remainingEnergy{};
};

/*---------------------------------------------------------------------
runRandomSimulation() -> Ejecuta automáticamente una partida utilizando RandomPolicy.

El controlador concreto se administra mediante std::unique_ptr<IController>,
permitiendo usar la interfaz polimórfica sin gestionar memoria manualmente.
La semilla controla la secuencia pseudoaleatoria para obtener simulaciones
reproducibles.
---------------------------------------------------------------------*/
template<std::size_t Rows, std::size_t Columns>
SimulationResult runRandomSimulation(
    NavigationEnvironment<Rows, Columns>& environment,
    std::uint32_t seed
) {
    environment.reset(seed);

std::unique_ptr<IController> controller =
    std::make_unique<PolicyController<RandomPolicy>>(
        RandomPolicy{seed}
    );

    SimulationResult result;

    while (!environment.isFinished()) {
        const Observation observation = environment.state();
        const std::vector<Action> legalActions =
            environment.availableActions();

        if (legalActions.empty()) {
            throw std::logic_error(
                "Automatic simulation has no legal actions"
            );
        }

        const Action selectedAction =
            controller->selectAction(observation, legalActions);

        result.actions.push_back(selectedAction);

        const StepResult stepResult =
            environment.step(selectedAction);

        result.reason = stepResult.reason;
    }

    const Observation finalObservation = environment.state();

    result.turns = finalObservation.turn;
    result.score = finalObservation.score;
    result.remainingEnergy = finalObservation.energy;

    return result;
}