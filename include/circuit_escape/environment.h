#pragma once

#include <cstddef>

enum class EndReason {
    none,
    goalReached,
    noEnergy,
    turnLimit
};

[[nodiscard]] EndReason evaluateTermination(
    bool agentOnExit,
    int energy,
    std::size_t turn,
    std::size_t turnLimit
) noexcept;
