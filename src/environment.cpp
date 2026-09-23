#include "circuit_escape/environment.h"
#include "circuit_escape/agent.h"
#include "circuit_escape/game_rules.h"

EndReason evaluateTermination(
    bool agentOnExit,
    int energy,
    std::size_t turn,
    std::size_t turnLimit
) noexcept {

    return firstSatisfiedTermination(
        TerminationCondition{
            agentOnExit && energy > 0,
            EndReason::goalReached
        },

        TerminationCondition{
            energy == 0,
            EndReason::noEnergy
        },

        TerminationCondition{
            turn >= turnLimit,
            EndReason::turnLimit
        }
    );
}