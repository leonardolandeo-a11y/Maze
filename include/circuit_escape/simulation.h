#pragma once

#include "circuit_escape/controllers.h"
#include "circuit_escape/environment.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

struct SimulationResult {
    std::vector<Action> actions;
    EndReason reason{EndReason::none};
    std::size_t turns{};
    int score{};
    int remainingEnergy{};
};

template<std::size_t Rows, std::size_t Columns>
SimulationResult runRandomSimulation(
    NavigationEnvironment<Rows, Columns>& environment,
    std::uint32_t seed
) {
    environment.reset(seed);

    PolicyController<RandomPolicy> controller{
        RandomPolicy{seed}
    };

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
            controller.selectAction(observation, legalActions);

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