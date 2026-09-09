#pragma once
#include <cstddef>

// Available difficulty levels for the game.
enum class Difficulty {
    easy,
    standard,
    hard,
};

// Stores all configurable rules for a difficulty profile.
struct GameRules {
    // Energy and turn limits.
    int initialEnergy;
    int maximumEnergy;
    std::size_t turnLimit;

    // Movement costs.
    int normalCellCost;
    int roughTerrainCost;
    int waitOrInvalidCost;

    // Rewards and penalties.
    int resourcePoints;
    int batteryRecharge;
    int trapEnergyPenalty;
    int trapScorePenalty;
};

// Returns the rules corresponding to the selected difficulty.
GameRules rulesFor(Difficulty difficulty);