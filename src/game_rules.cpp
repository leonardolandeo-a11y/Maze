#include "circuit_escape/game_rules.h"
#include <stdexcept>

GameRules rulesFor(Difficulty difficulty) {
    switch (difficulty) {

        case Difficulty::easy:
            return GameRules{
            80,  // initialEnergy
            80,  // maximumEnergy
            240, // turnLimit
            1,   // normalCellCost
            2,   // roughTerrainCost
            1,   // waitOrInvalidCost
            15,  // resourcePoints
            5,   // batteryRecharge
            1,   // trapEnergyPenalty
            0    // trapScorePenalty
        };

        case Difficulty::standard:
            return GameRules{
            60,  // initialEnergy
            60,  // maximumEnergy
            180, // turnLimit
            1,   // normalCellCost
            2,   // roughTerrainCost
            1,   // waitOrInvalidCost
            10,  // resourcePoints
            3,   // batteryRecharge
            2,   // trapEnergyPenalty
            1    // trapScorePenalty
        };

        case Difficulty::hard:
            return GameRules{
            40,  // initialEnergy
            40,  // maximumEnergy
            140, // turnLimit
            1,   // normalCellCost
            3,   // roughTerrainCost
            1,   // waitOrInvalidCost
            8,   // resourcePoints
            2,   // batteryRecharge
            3,   // trapEnergyPenalty
            2    // trapScorePenalty
        };
    }

    // Throws an exception if an unknown difficulty is received.
    throw std::invalid_argument("Unknown difficulty");
}