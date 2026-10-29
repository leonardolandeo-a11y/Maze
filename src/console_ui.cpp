#include "circuit_escape/console_ui.h"
#include "circuit_escape/help.h"
#include <variant>
#include <type_traits>
#include <array>
#include <stdexcept>


ConsoleUI::ConsoleUI(RenderMode mode): mode_(mode){};

/*---------------------------------------------------------------------
KeyMapping() -> Traduce eventos de teclado de FTXUI a comandos independientes de la UI.

Las teclas no reconocidas producen std::nullopt y no ejecutan acciones
sobre el entorno.
---------------------------------------------------------------------*/
std::optional<UICommand> ConsoleUI::KeyMapping(const ftxui::Event& event)const{
    if (event == ftxui::Event::Character('w') || event == ftxui::Event::Character('W')||
        event == ftxui::Event::ArrowUp){
        return UICommand::up;
    }
    if (event == ftxui::Event::Character('s')|| event == ftxui::Event::Character('S')||
        event == ftxui::Event::ArrowDown){
        return UICommand::down;
    }
    if (event == ftxui::Event::Character('d') || event == ftxui::Event::Character('D')||
        event == ftxui::Event::ArrowRight){
        return UICommand::right;
    }
    if (event == ftxui::Event::Character('a')|| event == ftxui::Event::Character('A')||
        event == ftxui::Event::ArrowLeft){
        return UICommand::left;
    }
    if (event == ftxui::Event::Character('h')|| event == ftxui::Event::Character('H')){
        return UICommand::help;
    }
    if (event == ftxui::Event::Character('q')|| event == ftxui::Event::Character('Q')){
        return UICommand::quit;
    }
    if (event == ftxui::Event::Character('e')|| event == ftxui::Event::Character('E')){ 
        return UICommand::wait;
    }
    return std::nullopt;

}

/*---------------------------------------------------------------------
help() -> Construye la pantalla de ayuda con los controles disponibles.

Mientras help_ esté activo, render() muestra esta vista en lugar
del tablero principal.
---------------------------------------------------------------------*/
ftxui::Element ConsoleUI::help() const {
    // Datos temporales para comprobar la presentación.
    HelpReport report;

    report.switch_a = {ObjectiveStatus::pending, 8};
    report.switch_b = {ObjectiveStatus::pending, 2};
    report.exit_goal = {ObjectiveStatus::locked, std::nullopt};

    report.current_energy = 12;
    report.total_cost = 10;

    report.recommended_order = {
        HelpTarget::switch_b,
        HelpTarget::switch_a,
        HelpTarget::exit
    };

    report.segment_costs = {2, 5, 3};

    Help helpView;

    return ftxui::vbox({
        ftxui::text("VISTA DE PRUEBA: DATOS DE EJEMPLO")
            | ftxui::bold
            | ftxui::color(ftxui::Color::Yellow),

        ftxui::separatorEmpty(),

        helpView.RenderGame(report),

        ftxui::separatorEmpty(),

        ftxui::text("H - Volver a la partida")
    });
}
/*---------------------------------------------------------------------
RenderSupBar() -> Construye la barra de estado con turno, energía, puntaje y recursos.

---------------------------------------------------------------------*/
ftxui::Element ConsoleUI::RenderSupBar(const Observation& observation) const {

    return ftxui::text(
        "Turn " + std::to_string(observation.turn) + "/" + std::to_string(observation.turnLimit) +
        " | Energy " + std::to_string(observation.energy) + "/" + std::to_string(observation.maximumEnergy) +
        " | Score " + std::to_string(observation.score) +
        " | Resources " +std::to_string(observation.collectedResources)
    );
}

ftxui::Element ConsoleUI::CoordinateCell(const std::string& label) const {

    return ftxui::text(label) | ftxui::size(ftxui::WIDTH,ftxui::EQUAL, 2);
}

/*---------------------------------------------------------------------
RenderBoard() -> Construye visualmente el tablero 20x30.

Cada posición se transforma en un elemento FTXUI mediante RenderCell().
También se agregan las coordenadas de filas y columnas y se mantiene
un ancho fijo por celda para conservar la forma de la cuadrícula.
---------------------------------------------------------------------*/
ftxui::Element ConsoleUI::RenderBoard(
    const NavigationEnvironment<20, 30>& environment, 
    const Observation& observation) const {

    constexpr int cellVisualWidth = 2;

    const std::array<std::string, 10> emojiDigits{
        "0️⃣", "1️⃣", "2️⃣", "3️⃣", "4️⃣",
        "5️⃣", "6️⃣", "7️⃣", "8️⃣", "9️⃣"
    };

    std::vector<ftxui::Element> rows;

    // -------------------------
    // Coordenadas horizontales
    // -------------------------
    std::vector<ftxui::Element> horizontalLabels;

    for (std::size_t column = 0; column < 30; ++column) {
        std::string label;

        if (mode_ == RenderMode::emoji) {label = emojiDigits[column % 10];}
        else {label = std::to_string(column % 10);}

        horizontalLabels.push_back(CoordinateCell(label));
    }


    const std::string corner = mode_ == RenderMode::emoji ? "⬜" : ".";

    rows.push_back(
        ftxui::hbox({
            CoordinateCell(corner),
            ftxui::hbox(std::move(horizontalLabels))
        })
    );


    // -------------------------
    // Filas del tablero
    // -------------------------
    for (std::size_t row = 0; row < 20; ++row) {

        std::vector<ftxui::Element> columns;

        // Coordenada vertical
        std::string rowLabel;
        if (mode_ == RenderMode::emoji) {rowLabel = emojiDigits[row % 10];}
        else {rowLabel = std::to_string(row % 10);}

        columns.push_back(CoordinateCell(rowLabel));

        // Celdas
        for (std::size_t column = 0; column < 30; ++column) {

            Position position{ row, column };

            const Cell& cell = environment.grid().at(position);
            const bool isAgent = observation.agent == position;

            columns.push_back(
                RenderCell(cell, isAgent)
                | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, cellVisualWidth)
                | ftxui::center
            );
        }

        rows.push_back(
            ftxui::hbox(std::move(columns))
        );
    }

    return ftxui::vbox(
        std::move(rows)
    );
}
/*---------------------------------------------------------------------
RenderInfBar() -> Construye el footer de la partida.

Un mensaje temporal de entrada, como "Unknown command", tiene prioridad
sobre el último NavigationEvent. Al final siempre se muestran los
controles básicos.
---------------------------------------------------------------------*/
ftxui::Element ConsoleUI::RenderInfBar(std::span<const NavigationEvent> recentEvents) const {
    std::string message;
    if (!inputMessage_.empty()) {
        message = inputMessage_ + " | ";
    }else if (!recentEvents.empty()) {
        message =EventMessage(recentEvents.back()) + " | ";
    }
    message += "WASD move | E wait | H help | Q exit";
    return ftxui::text(message);
}

/*---------------------------------------------------------------------
render() -> Construye la vista principal de la partida.

Obtiene el estado actual del entorno y ensambla la barra de estado,
el tablero y el footer de eventos/controles.

Si la ayuda está activa, muestra help() en lugar de la vista principal.
---------------------------------------------------------------------*/
ftxui::Element ConsoleUI::render(
    const NavigationEnvironment<20, 30>& environment,
    std::span<const NavigationEvent> recentEvents
) const {
    using namespace ftxui;

    if (help_) {
        return help();
    }

    Observation observation = environment.state();

    auto SupWindow = window(
        text(" STATUS ")
            | bold
            | color(Color::CyanLight),
        hbox({
            text("  "),
            RenderSupBar(observation),
            text("  ")
        })
    );
    auto boardWindow = window(
        text(" MAZE ")
            | bold
            | color(Color::CyanLight),
        RenderBoard(
            environment,
            observation
        )
    );


    auto InfWindow = window(
        text(" EVENTS / CONTROLS ")
            | bold
            | color(Color::CyanLight),
        hbox({
            text("  "),
            RenderInfBar(recentEvents),
            text("  ")
        })
    );


    return vbox({
        SupWindow,
        separatorEmpty(),
        boardWindow,
        separatorEmpty(),
        InfWindow
    })
    | center;
}

/*---------------------------------------------------------------------
symbol_for() -> Dependendiendo del objeto que reciba, escoge como se
debería renderizar (ya ase en modo ascii o emoji)
---------------------------------------------------------------------*/
std::string ConsoleUI::symbol_for(GameSymbol symbol) const {
    const bool IsAscii = mode_ == RenderMode::ascii;

    switch (symbol) {
        case GameSymbol::player:
            return IsAscii ? "@" : "🤖";

        case GameSymbol::empty:
            return IsAscii ? "." : "⬜";

        case GameSymbol::wall:
            return IsAscii ? "#" : "⬛";

        case GameSymbol::rough_terrain:
            return IsAscii ? "~" : "🟫";

        case GameSymbol::resource:
            return IsAscii ? "R" : "💎";

        case GameSymbol::battery:
            return IsAscii ? "+" : "⚡";

        case GameSymbol::trap:
            return IsAscii ? "T" : "💥";

        case GameSymbol::exit:
            return IsAscii ? "S" : "🏁";

        case GameSymbol::switch_a_pending:
            return IsAscii ? "A" : "🔴";

        case GameSymbol::switch_a_active:
            return IsAscii ? "a" : "🟢";

        case GameSymbol::switch_b_pending:
            return IsAscii ? "B" : "🟥";

        case GameSymbol::switch_b_active:
            return IsAscii ? "b" : "🟩";

        case GameSymbol::guard:
            return IsAscii ? "V" : "👮"; 
    }

    throw std::invalid_argument("Simbolo de juego desconocido");
}


/*---------------------------------------------------------------------
RenderCell() -> Convierte una Cell en su representación ASCII o emoji.

El agente tiene prioridad visual sobre la celda que ocupa.
Para el resto de casos, std::visit identifica el tipo almacenado
en Cell y selecciona su símbolo usando la funcion aux symbol_for
---------------------------------------------------------------------*/
ftxui::Element ConsoleUI::RenderCell( const Cell& cell, bool IsAgent) const {

    // El jugador tiene prioridad sobre la casilla que ocupa.
    if (IsAgent) {
        return ftxui::text(symbol_for(GameSymbol::player));
    }

    return std::visit(
        [this](const auto& currentCell) -> ftxui::Element {
            using CellType = std::decay_t<decltype(currentCell)>;

            GameSymbol symbol = GameSymbol::empty;

            if constexpr (std::is_same_v<CellType, Empty>) {
                symbol = GameSymbol::empty;

            } else if constexpr (std::is_same_v<CellType, Wall>) {
                symbol = GameSymbol::wall;

            } else if constexpr (
                std::is_same_v<CellType, RoughTerrain>
            ) {
                symbol = GameSymbol::rough_terrain;

            } else if constexpr (
                std::is_same_v<CellType, ResourceCell<int>>
            ) {
                symbol = currentCell.collected
                    ? GameSymbol::empty
                    : GameSymbol::resource;

            } else if constexpr (std::is_same_v<CellType, Battery>) {
                symbol = currentCell.consumed
                    ? GameSymbol::empty
                    : GameSymbol::battery;

            } else if constexpr (std::is_same_v<CellType, Trap>) {
                symbol = GameSymbol::trap;

            } else if constexpr (std::is_same_v<CellType, Exit>) {
                symbol = GameSymbol::exit;

            } else if constexpr (std::is_same_v<CellType, Switch>) {
                if (currentCell.id == SwitchId::a) {
                    symbol = currentCell.pressed
                        ? GameSymbol::switch_a_active
                        : GameSymbol::switch_a_pending;
                } else {
                    symbol = currentCell.pressed
                        ? GameSymbol::switch_b_active
                        : GameSymbol::switch_b_pending;
                }
            }

            return ftxui::text(symbol_for(symbol));
        },
        cell
    );
}

/*---------------------------------------------------------------------
EventMessage() -> Convierte un NavigationEvent en un mensaje legible para el footer.

std::visit selecciona el texto correspondiente al tipo concreto
de evento almacenado en el variant.
---------------------------------------------------------------------*/
std::string ConsoleUI::EventMessage(
    const NavigationEvent& event
) const {

    return std::visit(
        [](const auto& currentEvent) -> std::string {
            using EventType = std::decay_t<decltype(currentEvent)>;
            if constexpr (std::is_same_v<EventType, MovedEvent>) {
                return
                    "Moved to (" + 
                        std::to_string(currentEvent.to.row) + "," + std::to_string(currentEvent.to.column) + 
                    ")";
            }
            else if constexpr (std::is_same_v<EventType,MovementRejectedEvent>) {
                return "Movement rejected";
            }
            else if constexpr (
                std::is_same_v<EventType, ResourceCollectedEvent>) {
                return
                    "Resource +" + std::to_string(currentEvent.points) + " at (" + 
                        std::to_string(currentEvent.at.row) + "," + std::to_string(currentEvent.at.column) +
                    ")";
            }
            else if constexpr (
                std::is_same_v< EventType, EnergyChangedEvent>) {
                return
                    "Energy " + std::to_string(currentEvent.previous) +
                        " -> " + std::to_string(currentEvent.current);
            }


            else if constexpr (
                std::is_same_v< EventType, TrapTriggeredEvent>) {
                return
                    "Trap triggered at (" +
                        std::to_string(currentEvent.at.row) + "," + std::to_string(currentEvent.at.column) +
                    ")";
            }


            else if constexpr (
                std::is_same_v< EventType, GoalReachedEvent>) {
                return "Exit reached";
            }


            return "";
        },
        event
    );
}

