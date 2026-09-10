#pragma once

#include "circuit_escape/types.h"

#include <cstdint>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

struct Observation;

class IController {
public:
    virtual ~IController() = default;

    virtual Action selectAction(
        const Observation& observation,
        std::span<const Action> legalActions
    ) = 0;
};

    virtual ~IController() = default;
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
