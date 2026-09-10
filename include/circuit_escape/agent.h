#pragma once

#include "types.h"

#include <cstddef>

class Agent {

private:
    Position position;
    int energy;
    int maximumEnergy;
    int score;
    std::size_t collectedResources;
    bool active;

public:
    Agent(Position start, int initialEnergy, int maximumEnergy);

    Position getPosition() const ;
    int getEnergy() const ;
    int getMaximumEnergy() const ;
    int getScore() const ;
    std::size_t getCollectedResources() const ;
    bool isActive() const ;

    void setPosition(Position newPosition) ;
    void setEnergy(int amount) ;
    void addScore(int points) ;
    void addcollectedResources(size_t resources) ;
    void finish() ;
};