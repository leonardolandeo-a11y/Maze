#include "circuit_escape/help.h"

#include <string>

//Por el momentoss, lo dejamos como ftxui::texto plano
ftxui::Element Help::RenderMenu() const {
    auto content = ftxui::vbox({
        ftxui::text("OBJETIVO") | ftxui::bold | ftxui::color(ftxui::Color::CyanLight),
        ftxui::text("- Activa los interruptores A y B en cualquier orden."),
        ftxui::text("- Luego llega a la salida con energia restante,"),
        ftxui::text("- sin ser capturado y dentro del limite de turnos."),

        ftxui::separator(),

        ftxui::text("CONTROLES") | ftxui::bold | ftxui::color(ftxui::Color::CyanLight),
        ftxui::text("W / Flecha arriba     Mover arriba"),
        ftxui::text("S / Flecha abajo      Mover abajo"),
        ftxui::text("A / Flecha izquierda  Mover izquierda"),
        ftxui::text("D / Flecha derecha    Mover derecha"),
        ftxui::text("E                     Esperar un turno"),
        ftxui::text("H                     Consultar ayuda en la partida"),
        ftxui::text("Q                     Salir de la partida"),

        ftxui::separator(),

        ftxui::text("INTERRUPTORES Y SALIDA")
            | ftxui::bold | ftxui::color(ftxui::Color::CyanLight),
        ftxui::text("Entrar en un interruptor lo activa permanentemente."),
        ftxui::text("La salida solo permite escapar con ambos activados."),

        ftxui::separator(),

        ftxui::text("VIGILANTE") | ftxui::bold | ftxui::color(ftxui::Color::CyanLight),
        ftxui::text("Persigue al jugador y avanza en los turnos pares."),
        ftxui::text("Si ocupa tu misma casilla, pierdes."),

        ftxui::separator(),

        ftxui::text("ENERGIA") | ftxui::bold | ftxui::color(ftxui::Color::CyanLight),
        ftxui::text("El movimiento consume energia segun el terreno."),
        ftxui::text("Esperar o intentar un movimiento invalido cuesta 1."),
        ftxui::text("Llegar a la salida con energia cero no permite ganar.")
    });

    return window(
        ftxui::text(" AYUDA GENERAL ") | ftxui::bold | ftxui::color(ftxui::Color::CyanLight),
        content
    );
}

/*---------------------------------------------------------------------
RenderGame() -> Renderiza la información reicibida de HelpReport

1. cosText es un lambda que guarda el costo 
2. activeA/B guardan el estado de los bottones y exitState el estado de la salida en string
3. row define como se componene las filas
4. return -> la ventana con todo la información necesaria
---------------------------------------------------------------------*/
ftxui::Element Help::RenderGame(const HelpReport& report) const {

    auto costText = [](const ObjectiveInfo& objective) -> std::string {
        if (objective.status == ObjectiveStatus::activated ||
            objective.status == ObjectiveStatus::locked) {
            return "—";
        }

        if (!objective.cost_from_current.has_value()) {
            return "Destino inaccesible";
        }

        return std::to_string(*objective.cost_from_current);
    };

    const bool activeA =
        report.switch_a.status == ObjectiveStatus::activated;

    const bool activeB =
        report.switch_b.status == ObjectiveStatus::activated;

    std::string exitState;

    if (report.exit_goal.status == ObjectiveStatus::enabled) { exitState = "Habilitada"; } 
    else if (!activeA && !activeB) { exitState = "Requiere A y B";} 
    else if (!activeA) { exitState = "Requiere A"; } 
    else if (!activeB) { exitState = "Requiere B"; } 
    else { exitState = "Bloqueada"; }

    auto row = [](const std::string& objective,
                const std::string& state,
                const std::string& cost) -> ftxui::Element {
        return ftxui::hbox({
            ftxui::text(objective)
                | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 10),

            ftxui::text(state)
                | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 20),

            ftxui::text(cost)
        });
    };

    return ftxui::window(
        ftxui::text(" AYUDA DE ENERGIA ")
            | ftxui::bold
            | ftxui::color(ftxui::Color::CyanLight),

        ftxui::vbox({
            row("Objetivo", "Estado", "Gasto desde aqui")
                | ftxui::bold,

            ftxui::separator(),

            row("A",
                activeA ? "Activado" : "Pendiente",
                costText(report.switch_a)),

            row("B",
                activeB ? "Activado" : "Pendiente",
                costText(report.switch_b)),

            row("Salida",
                exitState,
                costText(report.exit_goal))
        })
    );
}