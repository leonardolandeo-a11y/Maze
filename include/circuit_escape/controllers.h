#pragma once

#include "circuit_escape/types.h"
#include <vector>

struct Observation;

class IController {
public:
    virtual Action chooseAction(
        const Observation& observation,
        const std::vector<Action>& legalActions
        ) = 0;

    virtual ~IController() = default;
};