#pragma once

#include "circuit_escape/types.h"
#include "circuit_escape/environment.h"

#include <concepts>
#include <cstdint>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


class IController {
public:
    virtual ~IController() = default;

    virtual Action selectAction(
        const Observation& observation,
        std::span<const Action> legalActions
    ) = 0;
};

class RandomPolicy {
    std::mt19937 generator_;

public:
    explicit RandomPolicy(std::uint32_t seed)
        : generator_(seed) {}

    Action selectAction(
        const Observation&,
        std::span<const Action> legalActions
    ) {
        if (legalActions.empty()) {
            throw std::invalid_argument(
                "RandomPolicy requires at least one legal action"
            );
        }

        std::uniform_int_distribution<std::size_t> distribution(
            0, legalActions.size() - 1
        );

        return legalActions[distribution(generator_)];
    }
};


template<typename Policy>
concept NavigationPolicy = requires(
    Policy& policy,
    const Observation& observation,
    std::span<const Action> actions
) {
    { policy.selectAction(observation, actions) } -> std::same_as<Action>;
};


template<typename Policy>
requires NavigationPolicy<Policy>
class PolicyController : public IController {
    Policy policy_;

public:
    
    explicit PolicyController(Policy policy)
        : policy_(std::move(policy)) {}

    Action selectAction(
        const Observation& observation,
        std::span<const Action> legalActions
    ) override {
        return policy_.selectAction(observation, legalActions);
    }
};

struct HeuristicPolicy {
    Action selectAction(
        const Observation& observation,
        std::span<const Action> legalActions
    );
};