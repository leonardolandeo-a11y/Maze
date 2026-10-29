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

    return ftxui::window(
        ftxui::text(" AYUDA GENERAL ") | ftxui::bold | ftxui::color(ftxui::Color::CyanLight),
        content
    );
}

/*---------------------------------------------------------------------
RenderGame() -> Renderiza la información reicibida de HelpReport

1. cosText es un lambda que convierte y guarda el costo/estado como string 
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

    auto targetName = [](HelpTarget target) -> std::string {
        switch (target) {
            case HelpTarget::switch_a:
                return "A";

            case HelpTarget::switch_b:
                return "B";

            case HelpTarget::exit:
                return "Salida";
        }

        return "Desconocido";
    };

    ftxui::Elements routeDetails;

    if (!report.total_cost.has_value()) {
        routeDetails.push_back( ftxui::text("No hay un recorrido completo.") );
    
    } else if ( report.recommended_order.empty() || report.recommended_order.size() != report.segment_costs.size()) {
        routeDetails.push_back( ftxui::text("Datos del recorrido incompletos.") );

    } else {
        std::string route = "Recorrido: Actual";

        for (HelpTarget target : report.recommended_order) {
            route += " -> " + targetName(target);
        }

        routeDetails.push_back(ftxui::text(route));
        routeDetails.push_back(ftxui::separatorEmpty());

        std::string previous = "Actual";

        for (std::size_t i = 0; i < report.recommended_order.size(); ++i) {
            const std::string destination =
                targetName(report.recommended_order[i]);

            routeDetails.push_back(
                ftxui::text(
                    previous + " -> " + destination +
                    ": " + std::to_string(report.segment_costs[i]) +
                    " de energia"
                )
            );

            previous = destination;
        }
    }

    ftxui::Elements energyDetails;

    energyDetails.push_back(
        ftxui::text( "Energia actual: " + std::to_string(report.current_energy))
    );

    if (report.total_cost.has_value()) {
        const int totalCost = *report.total_cost;

        const long long remainingEnergy =
            static_cast<long long>(report.current_energy) - totalCost;

        energyDetails.push_back(
            ftxui::text("Costo total: " + std::to_string(totalCost))
        );

        energyDetails.push_back(
            ftxui::text("Saldo estimado: " + std::to_string(remainingEnergy))
        );

        if (report.current_energy > totalCost) {
            energyDetails.push_back(
                ftxui::text("Energia suficiente para este recorrido sin recargas.")
                    | ftxui::color(ftxui::Color::Green)
            );

        } else if (report.current_energy == totalCost) {
            energyDetails.push_back(
                ftxui::text("Llegarias con energia cero: no permite ganar.")
                    | ftxui::color(ftxui::Color::Yellow)
            );

        } else {
            energyDetails.push_back(
                ftxui::text("Energia insuficiente para este recorrido sin recargas.")
                    | ftxui::color(ftxui::Color::Red)
            );
        }

    } else {
        energyDetails.push_back(
            ftxui::text("Destino inaccesible: no se puede estimar el saldo.")
                | ftxui::color(ftxui::Color::Red)
        );
    }

    energyDetails.push_back(ftxui::separatorEmpty());

    energyDetails.push_back(
        ftxui::text("El calculo no anticipa al vigilante ni las recargas.")
    );

    energyDetails.push_back(
        ftxui::text("El recorrido no garantiza una victoria.")
    );

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
                costText(report.exit_goal)),

            ftxui::separator(),
            ftxui::vbox(routeDetails),

            ftxui::separator(),
            ftxui::vbox(energyDetails)
        })
    );
}