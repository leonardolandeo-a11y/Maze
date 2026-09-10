#include "circuit_escape/environment.h"
#include "circuit_escape/agent.h"
#include "circuit_escape/game_rules.h"

EndReason evaluateTermination(
    bool agentOnExit,
    int energy,
    std::size_t turn,
    std::size_t turnLimit
) noexcept {
    if (agentOnExit && energy > 0) {
        return EndReason::goalReached;
    }

    if (energy == 0) {
        return EndReason::noEnergy;
    }

    if (turn >= turnLimit) {
        return EndReason::turnLimit;
    }

    return EndReason::none;
}
