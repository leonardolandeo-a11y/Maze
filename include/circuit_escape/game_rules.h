#pragma once
#include <cstddef>


enum class Difficulty {
    easy,
    standard,
    hard,
};

struct GameRules {
    int initialEnergy;
    int maximumEnergy;
    std::size_t turnLimit;

    int normalCellCost;
    int roughTerrainCost;
    int waitOrInvalidCost;

    int resoucePoints;
    int batteryRecharge;
    int trapEnergyPenalty;
    int trapScorePenalty;
};

GameRules rulesFor(Difficulty difficulty);