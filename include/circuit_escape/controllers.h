#pragma once

#include "circuit_escape/types.h"

#include <cstdint>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>
#include <concepts>
#include <utility>
/*
- clase abstracta de la que heredan todos los controllers
- destructor virtual necesario y todas las funciones abtractas
*/
class IController {
public:
    virtual ~IController() = default;
    virtual Action selectAction(const Observation& observation, std::span<const Action> legalActions) = 0;
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
/*
Concept que verifica que el tipo de dato Policy tenga un metodo un metodo llamada selectAction 
y verifica que este bien implementado: seleciona una accion que sea
std::span() objeto ligero hacia una lista contigua de elementos
std::same_As() verifica si los 2 tipos son exacatamente iguales
*/
template<typename Policy>
concept NavigationPolicy = requires(
    Policy& policy,
    const Observation& observation,
    std::span<const Action> actions
) {
    { policy.selectAction(observation, actions) }-> std::same_as<Action>;
};


template<typename Policy>
requires NavigationPolicy<Policy>
class PolicyController: public IController {
    Policy policy_;
public:
    //explicit es una buena practica para eivtar conversiones impliucitas
    // se usa la asignacion por movimiento por eficiencia (una copia seria innecesaria pues no necesitamos mantener el parametro policy despues del contructor)
    //PolicyController posee su propia politica (policy)
    explicit PolicyController(Policy policy) : policy_(std::move(policy)) {}
    Action selectAction(const Observation& observation, std::span<const Action> legalActions) override {
        return policy_.selectAction(observation,legalActions);
    }
};

struct HuristicPolicy {
    Action selectAction(
        const Observation& observation, 
        std::span<const Action> legalActions
    );
};
