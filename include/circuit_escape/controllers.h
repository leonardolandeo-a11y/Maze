#pragma once

#include "circuit_escape/environment.h"
#include <span>
#include <concepts>

template<typename Policy>
concept NavigationPolicy = requires(Policy& policy, const Observation& observation,
    std::span<const Action> actions) {
        { policy.selectAction(observation, actions) } -> std::same_as<Action>;
    }