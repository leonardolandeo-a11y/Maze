#pragma once

#include "circuit_escape/agent.h"
#include "circuit_escape/cells.h"
#include "circuit_escape/game_rules.h"
#include "circuit_escape/grid.h"

#include <cstddef>
#include <stdexcept>
#include <variant>
#include <vector>

/*---------------------------------------------------------------------*/
/*Patron de C++ para trabajar con std::visit (necesario por la rubrica)
Overloaded es un struct que guarda funciones lambda.
FLambda es el variadic template
{ using FLambda::operator()...; }: es un pack expansion que trae las
sobrecargas de () de cada funcion lambda
*/
template<class... FLambda>
struct Overloaded : FLambda... {
    using FLambda::operator()...;
};

/*Le indica al compilador que deduzca automaticamente los tipos de template*/
template<class... FLambda>
Overloaded(FLambda...) -> Overloaded<FLambda...>;
/*---------------------------------------------------------------------*/


enum class EndReason {
    none,
    goalReached,
    noEnergy,
    turnLimit
};

struct Observation {
    Position agent;
    Position goal;
    int energy{};
    int maximumEnergy{};
    int score{};
    std::size_t collectedResources{};
    std::size_t turn{};
    std::vector<Action> availableActions;
};

struct MovedEvent {
    Position from;
    Position to;
    int energyCost;
};

struct MovementRejectedEvent {
    Position from;
    Action action;
};

struct ResourceCollectedEvent {
    Position at;
    int points;
};

struct EnergyChangedEvent {
    int previous;
    int current;
};

struct TrapTriggeredEvent {
    Position at;
};

struct GoalReachedEvent {
    Position at;
};

using NavigationEvent = std::variant<
    MovedEvent,
    MovementRejectedEvent,
    ResourceCollectedEvent,
    EnergyChangedEvent,
    TrapTriggeredEvent,
    GoalReachedEvent
>;

struct StepResult {
    Observation observation;
    std::vector<NavigationEvent> events;
    bool finished{false};
    EndReason reason{EndReason::none};
};

[[nodiscard]] EndReason evaluateTermination(
    bool agentOnExit,
    int energy,
    std::size_t turn,
    std::size_t turnLimit
) noexcept;


// NavigationEnvironment for movement costs.
template<std::size_t Rows, std::size_t Columns>
class NavigationEnvironment {
private:
    Grid<Cell, Rows, Columns> grid;
    Agent agent;
    GameRules rules;
    std::size_t turn{0};

    Position goalPosition() const {
        for (std::size_t row = 0; row < Rows; ++row) {
            for (std::size_t column = 0; column < Columns; ++column) {
                Position position{row, column};

                if (std::holds_alternative<Exit>(grid.at(position))) {
                    return position;
                }
            }
        }

        return agent.getPosition(); // Temporal hasta validar salida
    }

    int movementCost(const Cell& cell) const {
        return std::visit(Overloaded{
            [&](const Empty&) {
                return rules.normalCellCost;
            },

            [&](const RoughTerrain&) {
                return rules.roughTerrainCost;
            },

            [&](const ResourceCell<int>&) {
                return rules.normalCellCost;
            },

            [&](const Battery&) {
                return rules.normalCellCost;
            },

            [&](const Trap&) {
                return rules.normalCellCost;
            },

            [&](const Exit&) {
                return rules.normalCellCost;
            },

            [&](const Wall&) {
                return rules.waitOrInvalidCost;
            }

        }, cell);
    }

    // funcion auxiliar para aplicar los efectos de la celda
    void applyEffectCell(Cell& target_cell, std::vector<NavigationEvent>& events) {
        // std::visit permite revisar el tipo de celda y escoger
        // la funcion lambda correspondiente
        std::visit(Overloaded{

            // logica para celdas con recompensas
            [&](ResourceCell<int>& resource) {
                if (resource.collected) {
                    return;
                }

                agent.addScore(rules.resourcePoints);
                agent.addcollectedResources(1);
                resource.collected = true;

                events.push_back(
                ResourceCollectedEvent{
                    agent.getPosition(),
                    rules.resourcePoints
                    }
                );
            },

            // logica para recarga de bateria
            [&](Battery& battery) {
                if (battery.consumed) {
                    return;
                }

                const int previousEnergy = agent.getEnergy();

                agent.addEnergy(rules.batteryRecharge);
                battery.consumed = true;

                if (agent.getEnergy() != previousEnergy) {
                events.push_back(
                    EnergyChangedEvent{
                        previousEnergy,
                        agent.getEnergy()
                        }
                    );
                }
            },

            // logica para trampas
            [&](Trap&) {
                const int previousEnergy = agent.getEnergy();
                agent.setEnergy(agent.getEnergy() - rules.trapEnergyPenalty);
                agent.addScore(-rules.trapScorePenalty);

                if (agent.getEnergy() != previousEnergy) {
                events.push_back(
                    EnergyChangedEvent{
                        previousEnergy,
                        agent.getEnergy()
                        }
                    );
                }

            events.push_back(
                TrapTriggeredEvent{
                    agent.getPosition()
                    }
                );
            },

            // caso generico
            [&](auto&) {
                // nada
            }

        }, target_cell);
    }

    [[nodiscard]] StepResult finalizeStep(
        std::vector<NavigationEvent> events
    ) {
        const bool agentOnExit =
            std::holds_alternative<Exit>(
                grid.at(agent.getPosition())
            );

        const EndReason reason = evaluateTermination(
            agentOnExit,
            agent.getEnergy(),
            turn,
            rules.turnLimit
        );

        if (reason == EndReason::goalReached) {
            events.push_back(
                GoalReachedEvent{
                    agent.getPosition()
                }
            );
        }

        if (reason != EndReason::none) {
            agent.finish();
        }

        return StepResult{
            state(),
            events,
            reason != EndReason::none,
            reason
        };
    }

public:
    NavigationEnvironment(
        Grid<Cell, Rows, Columns> grid_,
        Agent agent_,
        GameRules rules_
    )
        : grid(grid_),
          agent(agent_),
          rules(rules_) {
    }

    [[nodiscard]] std::vector<Action> availableActions() const {
        if (isFinished()) {
            return {};
        }

        std::vector<Action> actions;

        const Action movementActions[] = {
            Action::up,
            Action::down,
            Action::left,
            Action::right
        };

        for (Action action : movementActions) {
            auto candidate = neighbor(agent.getPosition(), action);

            if (!candidate.has_value()) {
                continue;
            }

            if (!grid.contains(candidate.value())) {
                continue;
            }

            const Cell& destination = grid.at(candidate.value());

            if (std::holds_alternative<Wall>(destination)) {
                continue;
            }

            actions.push_back(action);
        }

        actions.push_back(Action::wait);

        return actions;
    }

    [[nodiscard]] Observation state() const {
        return Observation{
            agent.getPosition(),
            goalPosition(),
            agent.getEnergy(),
            agent.getMaximumEnergy(),
            agent.getScore(),
            agent.getCollectedResources(),
            turn,
            availableActions()
        };
    }

    [[nodiscard]] bool isFinished() const noexcept {
        return !agent.isActive();
    }
    [[nodiscard]] StepResult step(Action action) {
        if (isFinished()) {
            throw std::logic_error(
                "Cannot execute step after the game has finished"
                );
            }

    std::vector<NavigationEvent> events;

    ++turn;

    const Position previousPosition = agent.getPosition();

    // accion wait
    if (action == Action::wait) {
        const int previousEnergy = agent.getEnergy();

        agent.setEnergy(
            agent.getEnergy() - rules.waitOrInvalidCost
        );

        if (agent.getEnergy() != previousEnergy) {
            events.push_back(
                EnergyChangedEvent{
                    previousEnergy,
                    agent.getEnergy()
                }
            );
        }

        return finalizeStep(events);
    }

    auto candidate = neighbor(
        agent.getPosition(),
        action
    );

    // movimiento fuera del tablero
    if (!candidate.has_value() ||
        !grid.contains(candidate.value())) {

        const int previousEnergy = agent.getEnergy();

        agent.setEnergy(
            agent.getEnergy() - rules.waitOrInvalidCost
        );

        if (agent.getEnergy() != previousEnergy) {
            events.push_back(
                EnergyChangedEvent{
                    previousEnergy,
                    agent.getEnergy()
                }
            );
        }

        events.push_back(
            MovementRejectedEvent{
                previousPosition,
                action
            }
        );

        return finalizeStep(events);
    }

    Cell& destination = grid.at(candidate.value());

    // movimiento hacia un muro
    if (std::holds_alternative<Wall>(destination)) {
        const int previousEnergy = agent.getEnergy();

        agent.setEnergy(
            agent.getEnergy() - rules.waitOrInvalidCost
        );

        if (agent.getEnergy() != previousEnergy) {
            events.push_back(
                EnergyChangedEvent{
                    previousEnergy,
                    agent.getEnergy()
                }
            );
        }

        events.push_back(
            MovementRejectedEvent{
                previousPosition,
                action
            }
        );

        return finalizeStep(events);
    }

    // movimiento valido
    const int cost = movementCost(destination);

    agent.setPosition(candidate.value());

    events.push_back(
        MovedEvent{
            previousPosition,
            agent.getPosition(),
            cost
        }
    );

    const int previousEnergy = agent.getEnergy();

    agent.setEnergy(
        agent.getEnergy() - cost
    );

    if (agent.getEnergy() != previousEnergy) {
        events.push_back(
            EnergyChangedEvent{
                previousEnergy,
                agent.getEnergy()
            }
        );
    }

    // aplicar el efecto despues del costo de entrada
    applyEffectCell(destination, events);

    return finalizeStep(events);
}
};