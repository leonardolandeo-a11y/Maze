#include "include/circuit_escape/agent.h"

Agent::Agent(Position start, int initialEnergy, int maximumEnergy)
: position(start), energy(initialEnergy), maximumEnergy(maximumEnergy), score(0), collectedResources(0), active(true){}


Position Agent::getPosition() const{
    return position;
}

int Agent::getEnergy() const {
    return energy;
}

int Agent::getMaximumEnergy() const {
    return maximumEnergy;
}

int Agent::getScore() const {
    return score;
}

std::size_t Agent::getCollectedResources() const {
    return collectedResources;
}
bool Agent::isActive() const{
    return active;
}
void Agent::setPosition(Position newPosition) {
    position = newPosition;
}

void Agent::setEnergy(int amount) {
    if (amount < 0){
        energy = 0;
    }
    if (amount > maximumEnergy){
        energy = maximumEnergy;
    }
    if (amount >= 0 && amount <= maximumEnergy){
        energy = amount; 
    }
}

void Agent::addScore(int points) {
    score += points;
}

void Agent::addcollectedResources(size_t resources) {
    collectedResources += resources;
}

void Agent::addEnergy(int energy_) {
    energy += energy_;
}

void Agent::finish() {
    active = false;
}
