#include "circuit_escape/agent.h"
#include "circuit_escape/game_rules.h"

namespace {

    void applyTrap(Agent& agent, const GameRules& rules) {
        agent.setEnergy(
            agent.getEnergy() - rules.trapEnergyPenalty
        );

        agent.addScore(
            -rules.trapScorePenalty
        );
    }

}