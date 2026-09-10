#pragma once

#include "circuit_escape/agent.h"
#include "circuit_escape/cells.h"
#include "circuit_escape/game_rules.h"
#include <variant>

/*---------------------------------------------------------------------*/
/*Patron de C++ para trabajar con std::visit (necesario por la rubrica)
Overloaded es un struct que guarda funciones lambda. 
FLambda es el variadic template
{ using FLambda::operator()...; }: es un pack expansion que trae las sobrecargas de () de cada funcion lambda
*/
template<class... FLambda>
struct Overloaded : FLambda... { using FLambda::operator()...; };

/*Le indica al compilador que deduzca automaticamente los tipos de template*/
template<class... FLambda>
Overloaded(FLambda...) -> Overloaded<FLambda...>;
/*---------------------------------------------------------------------*/

//funcion auxiliar para aplicar los efectos de la celda
void applyEffectCell(Agent& agent_, const GameRules&rules_, Cell& new_cell) {
    //std::visis permite ir a la celda, revisar el tipo y escoger la funcion que pueda manejar el tipo de celda
    //la funcion apply... solo se encarga de definir overloaded para pasarlo usarlo en std::visit
    std::visit(Overloaded{
        //logica para celdas con recompensas
        [&](ResourceCell<int>& resource){
            if (resource.collected) return;
            agent_.addcollectedResources(rules_.resourcePoints); //funcion modificada en games_rules.cpp
            resource.collected = true;
        },
        //logica para recarga de bateria
        [&](Battery& battery){
            if (battery.consumed) return;
            agent_.setEnergy(rules_.batteryRecharge);
            battery.consumed = true;
        },
        //caso generico
        [&](auto& unknown_cell){
            //nada
        },
            //etc etc etc
    },new_cell);
}

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

// NavegationEnvironment para movement_cost
template<std::size_t Rows, std::size_t Columns>
class NavigationEnvironment {
private:
    Grid<Cell, Rows, Columns> grid;
    Agent agent;
    GameRules rules;
    std::size_t turn{0};

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

public:
    NavigationEnvironment(
        Grid<Cell, Rows, Columns> grid_, Agent agent_, GameRules rules_) 
        : grid(grid_), agent(agent_), rules(rules_) {}

    void step(Action action) {
        if (action == Action::wait) {
            agent.setEnergy(agent.getEnergy() - rules.waitOrInvalidCost);
            ++turn;
            return;
        }

        auto candidate = neighbor(agent.getPosition(), action);

        if (!candidate.has_value()) {
            agent.setEnergy(agent.getEnergy() - rules.waitOrInvalidCost);
            ++turn;
            return;
        }

        if (!grid.contains(candidate.value())) {
            agent.setEnergy(agent.getEnergy() - rules.waitOrInvalidCost);
            ++turn;
            return;
        }

        Cell& destination = grid.at(
            candidate.value()
        );

        if (std::holds_alternative<Wall>(destination)) {
            agent.setEnergy(agent.getEnergy() - rules.waitOrInvalidCost);
            ++turn;
            return;
        }

        int cost = movementCost(destination);

        agent.setPosition(candidate.value());

        agent.setEnergy(agent.getEnergy() - cost);
        ++turn;
    }
};