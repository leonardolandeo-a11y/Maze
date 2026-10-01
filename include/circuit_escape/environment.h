#pragma once

#include "circuit_escape/agent.h"
#include "circuit_escape/cells.h"
#include "circuit_escape/game_rules.h"
#include "circuit_escape/grid.h"
#include "circuit_escape/generic_functions.h"

#include <iterator> 
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <variant>
#include <vector>

/*---------------------------------------------------------------------
Helper variádico utilizado junto con std::visit.

Hereda de varias lambdas y expone todos sus operator(), permitiendo
definir una operación diferente para cada alternativa de un std::variant.
---------------------------------------------------------------------*/
template<class... FLambda>
struct Overloaded : FLambda... {
    using FLambda::operator()...;
};

// Deduction guide para construir Overloaded sin indicar explícitamente
// los tipos de las lambdas.
template<class... FLambda>
Overloaded(FLambda...) -> Overloaded<FLambda...>;


enum class EndReason {
    none,
    goalReached,
    noEnergy,
    turnLimit
};

// Representa una posible condición de término y el motivo asociado.
struct TerminationCondition {
    bool satisfied;
    EndReason reason;
};

/*---------------------------------------------------------------------
Evalúa un conjunto de condiciones en el orden recibido.

La fold expression aplica evaluate() sobre cada condición y conserva
únicamente la primera que se satisface. De esta forma el orden de los
argumentos define la precedencia de las condiciones de término.
---------------------------------------------------------------------*/
template<typename... Conditions>
[[nodiscard]] EndReason firstSatisfiedTermination(
    const Conditions&... conditions
) noexcept {

    EndReason result = EndReason::none;

    auto evaluate = [&result](const auto& condition) {
        if (
            result == EndReason::none &&
            condition.satisfied
        ) {
            result = condition.reason;
        }
    };

    (evaluate(conditions), ...);

    return result;
}

/*---------------------------------------------------------------------
Snapshot del estado observable del entorno.

Los controladores reciben este struct para tomar decisiones
sin obtener acceso directo al estado interno de NavigationEnvironment.
---------------------------------------------------------------------*/
struct Observation {
    Position agent;
    Position goal;
    int energy{};
    int maximumEnergy{};
    int score{};
    std::size_t collectedResources{};
    std::size_t turn{};
    std::size_t turnLimit{};
    std::vector<Action> availableActions;
};

/*---------------------------------------------------------------------
Los eventos describen los cambios producidos durante un turno.

NavigationEnvironment los genera, mientras que otras capas, como
ConsoleUI, pueden procesarlos sin conocer ni modificar la lógica interna
del entorno.
---------------------------------------------------------------------*/

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
/*---------------------------------------------------------------------
Un NavigationEvent puede representar cualquiera de los eventos
producidos por el entorno durante la ejecución de una acción.
---------------------------------------------------------------------*/
using NavigationEvent = std::variant<
    MovedEvent,
    MovementRejectedEvent,
    ResourceCollectedEvent,
    EnergyChangedEvent,
    TrapTriggeredEvent,
    GoalReachedEvent
>;

/*---------------------------------------------------------------------
Resultado completo de ejecutar una acción.

Contiene el nuevo estado observable, los eventos producidos durante
el turno y la información de término de la partida.
---------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------
NavigationEnvironment -> Motor principal de la simulación.

NavigationEnvironment mantiene el tablero y el agente, procesa cada
Action mediante step(), aplica los costos y efectos de las celdas,
genera NavigationEvent y determina cuándo termina la partida.

La clase no realiza entrada/salida de consola. La interfaz gráfica
solamente consulta su estado y le entrega acciones.

initialGrid_ e initialAgent_ conservan el estado inicial para que
reset() pueda reproducir una nueva simulación.
---------------------------------------------------------------------*/
template<std::size_t Rows, std::size_t Columns>
class NavigationEnvironment {
    Grid<Cell, Rows, Columns> initialGrid_;
    Grid<Cell, Rows, Columns> grid_;

    Agent initialAgent_;
    Agent agent;
    GameRules rules;
    std::size_t turn{0};

    /*---------------------------------------------------------------------
    Verifica las invariantes necesarias para crear un entorno válido:
    posición inicial dentro del tablero y transitable, exactamente una
    salida, energía inicial positiva y límite de turnos válido.
    ---------------------------------------------------------------------*/
    void validateInitialState() const {
        const Position start = agent.getPosition();
        
        if (!grid_.contains(start)) {
            throw std::invalid_argument(
                "Initial position is outside the grid"
            );
        }

        if (!isTraversable(grid_.at(start))) {
            throw std::invalid_argument(
                "Initial position is not traversable"
            );
        }
        const std::size_t exitCount = countMatching(
            grid_.cbegin(), 
            grid_.cend(), 
            [](const Cell& cell) {return std::holds_alternative<Exit>(cell);} 
        );

        if (exitCount != 1) {
            throw std::invalid_argument(
                "Environment must contain exactly one exit"
            );
        }

        if (agent.getEnergy() <= 0) {
            throw std::invalid_argument(
                "Initial energy must be positive"
            );
        }
        
        if (rules.turnLimit == 0) {
            throw std::invalid_argument(
                "Turn limit must be positive"
            );
        }
    }

    /*---------------------------------------------------------------------
    Localiza la única salida del tablero mediante LinearSearch y convierte
    la posición lineal del iterador en coordenadas fila-columna. 
    ---------------------------------------------------------------------*/
    Position goalPosition() const {
        const auto exitIterator = LinearSearch(
            grid_.cbegin(),
            grid_.cend(),
            [](const Cell& cell) {
                return std::holds_alternative<Exit>(cell);
            }
        );

        if (exitIterator == grid_.cend()) {
            throw std::logic_error(
                "Environment invariant violated: exit not found"
            );
        }

        const std::size_t index =
            static_cast<std::size_t>(
                std::distance(
                    grid_.cbegin(),
                    exitIterator
                )
            );

        return Position{
            index/Columns, 
            index%Columns 
        };
    }
    /*---------------------------------------------------------------------
    Obtiene el costo de entrada de una celda.

    std::visit despacha según el tipo almacenado en Cell y obtiene el
    costo correspondiente desde GameRules.
    ---------------------------------------------------------------------*/
    int movementCost(const Cell &cell) const {
        return std::visit(Overloaded{
                              [&](const Empty &) {
                                  return rules.normalCellCost;
                              },

                              [&](const RoughTerrain &) {
                                  return rules.roughTerrainCost;
                              },

                              [&](const ResourceCell<int> &) {
                                  return rules.normalCellCost;
                              },

                              [&](const Battery &) {
                                  return rules.normalCellCost;
                              },

                              [&](const Trap &) {
                                  return rules.normalCellCost;
                              },

                              [&](const Exit &) {
                                  return rules.normalCellCost;
                              },

                              [&](const Wall &) {
                                  return rules.waitOrInvalidCost;
                              }

                          }, cell);
    }
    /*---------------------------------------------------------------------
    Aplica el efecto específico de la celda después de pagar su costo
    de entrada.

    std::visit selecciona la lógica correspondiente:
    - ResourceCell suma puntaje y se consume una sola vez.
    - Battery recupera energía y se consume una sola vez.
    - Trap aplica sus penalizaciones cada vez que se activa.
    - Las demás celdas no producen un efecto adicional.

    Los cambios relevantes se registran como NavigationEvent.
    ---------------------------------------------------------------------*/
    void applyEffectCell(Cell &target_cell, std::vector<NavigationEvent> &events) {
        std::visit(Overloaded{

                    [&](auto& resource)
                        requires CellTraits<std::remove_cvref_t<decltype(resource)>>::resource {
                        if (resource.collected) {
                        return;
                    }

                    agent.addScore(rules.resourcePoints);
                    agent.addcollectedResources(1);
                    resource.collected = true;

                    events.push_back(
                        ResourceCollectedEvent{
                            agent.getPosition(), rules.resourcePoints
                    }
                );
    },

                       [&](Battery &battery) {
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

                       
                       [&](Trap &) {
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

                       [&](auto &) {
                           
                       }

                   }, target_cell);
    }
    /*---------------------------------------------------------------------
    Finaliza un turno después de procesar la acción.

    Evalúa las condiciones de término, genera GoalReachedEvent cuando
    corresponde, marca al agente como inactivo y construye el StepResult
    que será entregado al resto del programa.
    ---------------------------------------------------------------------*/
    [[nodiscard]] StepResult finalizeStep(
        std::vector<NavigationEvent> events
    ) {
        
        const bool agentOnExit =
                std::holds_alternative<Exit>(
                    grid_.at(agent.getPosition())
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
    using grid_type = Grid<Cell, Rows, Columns>;

    NavigationEnvironment(
        grid_type initialGrid,
        Agent agent_,
        GameRules rules_
    )
        : initialGrid_(initialGrid),
          grid_(initialGrid),
          initialAgent_(agent_),
          agent(agent_),
          rules(rules_) {
        validateInitialState();
    }
    /*---------------------------------------------------------------------
    Restaura el tablero, el agente y el contador de turnos al estado inicial.
    
    NavigationEnvironment no toma decisiones aleatorias, por lo que la
    semilla se recibe únicamente para mantener una interfaz reproducible
    compatible con las simulaciones.
    ---------------------------------------------------------------------*/
    void reset(std::uint32_t seed) {
        (void) seed;

        grid_ = initialGrid_;
        agent = initialAgent_;
        turn = 0;
    }
    /*---------------------------------------------------------------------
    Construye las acciones legales desde la posición actual.

    Se descartan movimientos fuera del tablero o hacia celdas no
    transitables. Action::wait permanece disponible mientras la partida
    siga activa.
    ---------------------------------------------------------------------*/
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

        for (Action action: movementActions) {
            auto candidate = neighbor(agent.getPosition(), action);

            if (!candidate.has_value()) {
                continue;
            }

            if (!grid_.contains(candidate.value())) {
                continue;
            }

            const Cell &destination = grid_.at(candidate.value());

            if (!isTraversable(destination)) {
                continue;
            }

            actions.push_back(action);
        }

        actions.push_back(Action::wait);

        return actions;
    }
    /*---------------------------------------------------------------------
    Construye una copia del estado observable actual.
    Los controladores pueden usarla para decidir sin acceder directamente
    a los miembros internos del entorno.
    ---------------------------------------------------------------------*/
    [[nodiscard]] Observation state() const {
        return Observation{
            agent.getPosition(),
            goalPosition(),
            agent.getEnergy(),
            agent.getMaximumEnergy(),
            agent.getScore(),
            agent.getCollectedResources(),
            turn,
            rules.turnLimit,
            availableActions()
        };
    }

    [[nodiscard]] bool isFinished() const noexcept {
        return !agent.isActive();
    }
    [[nodiscard]] const grid_type &grid() const noexcept {
        return grid_;
    }

    
    /*---------------------------------------------------------------------
    Step() -> Ejecuta un turno completo del entorno.

    Flujo:
    1. Incrementa el turno.
    2. Procesa wait o movimientos inválidos aplicando su costo.
    3. Para un movimiento válido, actualiza la posición y paga el costo
    de entrada de la celda destino.
    4. Aplica el efecto de la celda mediante applyEffectCell().
    5. Registra los eventos producidos en el orden en que ocurren.
    6. Llama a finalizeStep() para evaluar el término y construir
    el StepResult.

    Una llamada después de finalizar la partida produce std::logic_error.
    ---------------------------------------------------------------------*/
    [[nodiscard]] StepResult step(Action action) {
        if (isFinished()) {
            throw std::logic_error(
                "Cannot execute step after the game has finished"
            );
        }

        std::vector<NavigationEvent> events;

        ++turn;

        const Position previousPosition = agent.getPosition();

        
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
        if (!candidate.has_value() ||
            !grid_.contains(candidate.value())) {
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

        Cell &destination = grid_.at(candidate.value());

        if (!isTraversable(destination)) { 
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

        applyEffectCell(destination, events);

        return finalizeStep(events);
    }
};
