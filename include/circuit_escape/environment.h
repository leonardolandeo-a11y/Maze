#pragma once

#include "circuit_escape/agent.h"
#include "circuit_escape/cells.h"
#include "circuit_escape/game_rules.h"
#include "circuit_escape/grid.h"

#include <cstddef>
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
    void applyEffectCell(Cell& target_cell) {
        // std::visit permite revisar el tipo de celda y escoger
        // la funcion lambda correspondiente
        std::visit(Overloaded{

            // logica para celdas con recompensas
            [&](ResourceCell<int>& resource) {
                if (resource.collected) {
                    return;
                }

                agent.addcollectedResources(rules.resourcePoints);
                resource.collected = true;
            },

            // logica para recarga de bateria
            [&](Battery& battery) {
                if (battery.consumed) {
                    return;
                }

                agent.addEnergy(rules.batteryRecharge);
                battery.consumed = true;
            },

            // caso generico
            [&](auto&) {
                // nada
            }

        }, target_cell);
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

    void step(Action action) {
        if (action == Action::wait) {
            agent.setEnergy(
                agent.getEnergy() - rules.waitOrInvalidCost
            );

            ++turn;
            return;
        }

        auto candidate = neighbor(agent.getPosition(), action);

        if (!candidate.has_value() || !grid.contains(candidate.value())) {
            agent.setEnergy(
                agent.getEnergy() - rules.waitOrInvalidCost
            );

            ++turn;
            return;
        }

        Cell& destination = grid.at(candidate.value());

        if (std::holds_alternative<Wall>(destination)) {
            agent.setEnergy(
                agent.getEnergy() - rules.waitOrInvalidCost
            );

            ++turn;
            return;
        }

        const int cost = movementCost(destination);

        agent.setPosition(candidate.value());
        agent.setEnergy(agent.getEnergy() - cost);

        applyEffectCell(destination);

        ++turn;
    }
};