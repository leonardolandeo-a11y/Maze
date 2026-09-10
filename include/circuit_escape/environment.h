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
