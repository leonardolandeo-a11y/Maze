#include "circuit_escape/environment.h"
#include "circuit_escape/agent.h"
#include "circuit_escape/game_rules.h"

/*---------------------------------------------------------------------
EndReason evalúa las condiciones de término respetando su prioridad:
    1. alcanzar la salida con energía disponible;
    2. quedarse sin energía;
    3. alcanzar el límite de turnos.

firstSatisfiedTermination(): procesa las condiciones mediante una
fold expression y devuelve la primera que se cumple.
---------------------------------------------------------------------*/
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