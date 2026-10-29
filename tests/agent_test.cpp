#include "circuit_escape/agent.h"

#include <cassert>
#include <limits>
#include <stdexcept>

namespace {

void test_agent_initial_state() {
    Agent agent({1, 2}, 6, 10);
    assert((agent.getPosition() == Position{1, 2}));
    assert(agent.getEnergy() == 6);
    assert(agent.getMaximumEnergy() == 10);
    assert(agent.getScore() == 0);
    assert(agent.getCollectedResources() == 0);
    assert(agent.isActive());

    agent.setPosition({3, 4});
    assert((agent.getPosition() == Position{3, 4}));
    agent.finish();
    assert(!agent.isActive());
    agent.finish();
    assert(!agent.isActive());
}

void test_agent_invalid_energy_bounds() {
    const auto rejects = [](int initial, int maximum) {
        try {
            [[maybe_unused]] Agent invalid({0, 0}, initial, maximum);
        } catch (const std::invalid_argument&) {
            return true;
        }
        return false;
    };
    assert(rejects(-1, 10));
    assert(rejects(11, 10));
    assert(rejects(1, 0));
    assert(rejects(1, -1));

    Agent depleted({0, 0}, 0, 10);
    assert(depleted.getEnergy() == 0);
}

void test_agent_energy_saturation() {
    Agent agent({0, 0}, 5, 10);
    agent.setEnergy(-1);
    assert(agent.getEnergy() == 0);
    agent.setEnergy(15);
    assert(agent.getEnergy() == 10);
    agent.setEnergy(5);
    agent.addEnergy(std::numeric_limits<int>::max());
    assert(agent.getEnergy() == 10);
    agent.addEnergy(std::numeric_limits<int>::min());
    assert(agent.getEnergy() == 0);
}

void test_agent_score_saturation() {
    Agent agent({0, 0}, 5, 10);
    agent.addScore(std::numeric_limits<int>::max());
    agent.addScore(1);
    assert(agent.getScore() == std::numeric_limits<int>::max());
    agent.addScore(std::numeric_limits<int>::min());
    assert(agent.getScore() == -1);
    agent.addScore(std::numeric_limits<int>::min());
    assert(agent.getScore() == std::numeric_limits<int>::min());
}

void test_agent_resources_saturation() {
    Agent agent({0, 0}, 5, 10);
    agent.addcollectedResources(2);
    assert(agent.getCollectedResources() == 2);
    agent.addcollectedResources(std::numeric_limits<std::size_t>::max());
    assert(agent.getCollectedResources() == std::numeric_limits<std::size_t>::max());
    agent.addcollectedResources(1);
    assert(agent.getCollectedResources() == std::numeric_limits<std::size_t>::max());
}

} // namespace

void run_agent_tests() {
    test_agent_initial_state();
    test_agent_invalid_energy_bounds();
    test_agent_energy_saturation();
    test_agent_score_saturation();
    test_agent_resources_saturation();
}
