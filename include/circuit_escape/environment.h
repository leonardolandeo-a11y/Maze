#pragma once

#include "circuit_escape/agent.h"
#include "circuit_escape/cells.h"
#include "circuit_escape/game_rules.h"
#include "circuit_escape/grid.h"

#include <cstddef>
#include <cstdint>
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

//struct que indica por que acabo el juego
enum class EndReason {
    none,
    goalReached,
    noEnergy,
    turnLimit
};
//struct que guarda toda la informacion del agent. necesario para el controller
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
/*structs eventos
Guardan los detalles de lo que ocurrio en un turno
Se agrupan en std::variant NavigationEvent
Estos structs son guardados en std::variant NavigationEvent
Seran pasados a std::visit para el motor grafico
*/
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
//navigationevent es un tipo que represente cualquier tipo de evento
using NavigationEvent = std::variant<
    MovedEvent,
    MovementRejectedEvent,
    ResourceCollectedEvent,
    EnergyChangedEvent,
    TrapTriggeredEvent,
    GoalReachedEvent
>;
/*
strcut stepresult guarda:
- la informacion de observation (estado del agente) 
- el vector de eventos
- un flag para ver si termino
- un endreason,  si termino

En resumen. informacion tras realizar un paso
*/
struct StepResult {
    Observation observation;
    std::vector<NavigationEvent> events;
    bool finished{false};
    EndReason reason{EndReason::none};
};
//funcion que evalua si ya termino el juego, retorna un endreason
[[nodiscard]] EndReason evaluateTermination(
    bool agentOnExit,
    int energy,
    std::size_t turn,
    std::size_t turnLimit
) noexcept;


// NavigationEnvironment for movement costs.
/*
Esta clase se encarga de lo siguiente:
- Proceso la accion solicita mediante step()
- Calcula los costos de mov y efectos de celda
- Evalua la condicion en que termino el juego
- Genera el registro de eventos vector<NavigationEvent> events
- Crear un OBservation inofensivo que se pasara a los controllers para que sepan que pasa en cada turno
*/
template<std::size_t Rows, std::size_t Columns>
class NavigationEnvironment {
    Grid<Cell, Rows, Columns> initialGrid_;
    Grid<Cell, Rows, Columns> grid_;

    Agent initialAgent_;
    Agent agent;
    /*
    Se tiene un obj inicial para agente y tablero para guarda la informacion del inicio del juego
    Los otros obj se modifican durante el juego
    se instancian las reglas y el turno=0
    */
    GameRules rules;
    std::size_t turn{0};

    //condiciones lo validamos
    //metodo para que se verifica que la posicion de inicializacion sea correcta
    void validateInitialState() const {
        const Position start = agent.getPosition();
        
        if (!grid_.contains(start)) {
            throw std::invalid_argument(
                "Initial position is outside the grid"
            );
        }

        if (std::holds_alternative<Wall>(grid_.at(start))) {
            throw std::invalid_argument(
                "Initial position is not traversable"
            );
        }
        //contador de salidas
        //una vez inicializao el tablero, se cuenta cuatnas salidas hay
        std::size_t exitCount = 0;
        
        for (const Cell &cell: grid_) {
            if (std::holds_alternative<Exit>(cell)) {
                ++exitCount;
            }
        }

        if (exitCount != 1) {
            throw std::invalid_argument(
                "Environment must contain exactly one exit"
            );
        }
        //tirar errores si algo falla
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
    //este metodo retorna la posicion de cell donde se encuentra la salida 
    Position goalPosition() const {
        for (std::size_t row = 0; row < Rows; ++row) {
            for (std::size_t column = 0; column < Columns; ++column) {
                Position position{row, column};

                if (std::holds_alternative<Exit>(grid_.at(position))) {
                    return position;
                }
            }
        }

        throw std::logic_error(
            "Environment invariant violated: exit not found"
        );
    }
    /*
    el metodo movcost usa el patron el struct Overloaded que guarda funciones lambda
    usa la misma logica que applyeffectcell para obtener el tipo de cell y aplicar 
    el costo de moviemiento
    */
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

    // funcion auxiliar para aplicar los efectos de la celda
    void applyEffectCell(Cell &target_cell, std::vector<NavigationEvent> &events) {
        // std::visit permite revisar el tipo de celda y escoger
        // la funcion lambda correspondiente
        std::visit(Overloaded{

                       // logica para celdas con recompensas
                       [&](ResourceCell<int> &resource) {
                           if (resource.collected) {
                               return;
                           }
                           //se agrega la recomepnsa puntos  y colecciona la recompensa
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
                       [&](Battery &battery) {
                           if (battery.consumed) {
                               return;
                           }
                           //se guarda la bateria actual, se recarga, y si la energia cambia (no es max) se registra el evento
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

                       // caso generico
                       [&](auto &) {
                           // nada
                       }

                   }, target_cell);
    }
    //Al final de cada turno, evalua si ya termino, los eventos y retorna un stepresult con la informacion
    [[nodiscard]] StepResult finalizeStep(
        std::vector<NavigationEvent> events
    ) {
        //verifica si el agente esta en la salida
        const bool agentOnExit =
                std::holds_alternative<Exit>(
                    grid_.at(agent.getPosition())
                );
        //llama a la funcion evaluateTermination para corrobar si ya llego al final
        const EndReason reason = evaluateTermination(
            agentOnExit,
            agent.getEnergy(),
            turn,
            rules.turnLimit
        );
        //VERIFICA SI SE LLEGA A LA META
        if (reason == EndReason::goalReached) {
            events.push_back(
                GoalReachedEvent{
                    agent.getPosition()
                }
            );
        }
        //VERIFICA SI TERMINO EL JUEGO
        if (reason != EndReason::none) {
            agent.finish();
        }
        //
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

    void reset(std::uint32_t seed) {
        // entorno no realiza decisiones aleatorias
        // semilla para reproducciones
        (void) seed;

        grid_ = initialGrid_;
        agent = initialAgent_;
        turn = 0;
    }
    /*
    Realiza lo sgiguiente:
    - Filtra mov invalidos
    - Registra acciones invalidas
    - 
    */
    [[nodiscard]] std::vector<Action> availableActions() const {
        if (isFinished()) {//verifica si ya acabo la partida
            return {};
        }

        std::vector<Action> actions;
        
        const Action movementActions[] = {
            Action::up,
            Action::down,
            Action::left,
            Action::right
        };
        //mediante un for verifica que acciones son validas para agregarlas al vector actions
        for (Action action: movementActions) {
            //instancia cada posicion de casilla
            auto candidate = neighbor(agent.getPosition(), action);

            if (!candidate.has_value()) {
                continue;
            }

            if (!grid_.contains(candidate.value())) {
                continue;
            }

            const Cell &destination = grid_.at(candidate.value());

            if (std::holds_alternative<Wall>(destination)) {
                continue;
            }

            actions.push_back(action);
        }
        //wait siempre es valido
        actions.push_back(Action::wait);

        return actions;
    }
    //funcion que retirna un bjeto observation con toda la informacion del momento
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
    //se explica solo
    [[nodiscard]] bool isFinished() const noexcept {
        return !agent.isActive();
    }
    //retorna el tableroS
    [[nodiscard]] const grid_type &grid() const noexcept {
        return grid_;
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
        //verifica y rechaza mov fueras del tablerp (inncesario)
        // movimiento fuera del tablero
        if (!candidate.has_value() ||
            !grid_.contains(candidate.value())) {
            const int previousEnergy = agent.getEnergy();

            agent.setEnergy(
                agent.getEnergy() - rules.waitOrInvalidCost
            );
            //guarda evento cambio de energia
            if (agent.getEnergy() != previousEnergy) {
                events.push_back(
                    EnergyChangedEvent{
                        previousEnergy,
                        agent.getEnergy()
                    }
                );
            }
            //gaurda evento movimeinto rechazados
            events.push_back(
                MovementRejectedEvent{
                    previousPosition,
                    action
                }
            );

            return finalizeStep(events);
        }

        Cell &destination = grid_.at(candidate.value());

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
