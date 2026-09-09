#include "circuit_escape/game_rules.h"
#include <stdexcept>


GameRules rulesFor(Difficulty difficulty) {
    switch (difficulty) {

        case Difficulty::easy:
            return GameRules{
            80, 80, 240,
            1, 2, 1,
            15, 5, 1, 0
        };

        case Difficulty::standard:
            return GameRules{
            60, 60, 180,
            1, 2, 1,
            10, 3, 2, 1
        };
        case Difficulty::hard:
            return GameRules{
            40, 40, 140,
            1, 3, 1,
            8, 2, 3, 2
        };
    }
    throw std::invalid_argument("Unknown difficulty");
}