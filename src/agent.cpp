#include "circuit_escape/agent.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>

Agent::Agent(Position start, int initialEnergy, int maximumEnergy)
: position(start), energy(initialEnergy), maximumEnergy(maximumEnergy), score(0), collectedResources(0), active(true) {
    if (maximumEnergy <= 0 || initialEnergy < 0 || initialEnergy > maximumEnergy) {
        throw std::invalid_argument("Invalid agent energy bounds");
    }
}


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
    const std::int64_t result = static_cast<std::int64_t>(score) + points;
    score = static_cast<int>(std::clamp(result,
        static_cast<std::int64_t>(std::numeric_limits<int>::min()),
        static_cast<std::int64_t>(std::numeric_limits<int>::max())));
}

void Agent::addcollectedResources(size_t resources) {
    const std::size_t available =
        std::numeric_limits<std::size_t>::max() - collectedResources;
    collectedResources += std::min(resources, available);
}

void Agent::addEnergy(int energy_) {
    const std::int64_t result = static_cast<std::int64_t>(energy) + energy_;
    setEnergy(static_cast<int>(std::clamp(result,
        std::int64_t{0}, static_cast<std::int64_t>(maximumEnergy))));
}

void Agent::finish() {
    active = false;
}
