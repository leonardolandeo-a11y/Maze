#include "circuit_escape/console_ui.h"
#include <variant>
#include <type_traits>
#include <array>


/*   Constructor of the console_ui   */
ConsoleUI::ConsoleUI(RenderMode mode): mode_(mode){};

/*=========================*/

/*   Translate keys    */

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

//Renderizacion de Help
ftxui::Element ConsoleUI::help() const {
    using namespace ftxui;

    const std::array<std::string, 6> controlsTitle = {
        " ██████╗ ██████╗ ███╗   ██╗████████╗██████╗  ██████╗ ██╗     ███████╗",
        "██╔════╝██╔═══██╗████╗  ██║╚══██╔══╝██╔══██╗██╔═══██╗██║     ██╔════╝",
        "██║     ██║   ██║██╔██╗ ██║   ██║   ██████╔╝██║   ██║██║     ███████╗",
        "██║     ██║   ██║██║╚██╗██║   ██║   ██╔══██╗██║   ██║██║     ╚════██║",
        "╚██████╗╚██████╔╝██║ ╚████║   ██║   ██║  ██║╚██████╔╝███████╗███████║",
        " ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚══════╝"
    };

    Elements titleLines;

    for (const auto& line : controlsTitle){
        titleLines.push_back(
            text(line)
            | color(Color::CyanLight)
            | bold
            | center
        );
    }

    auto movement = vbox({
        hbox({
            text("W / ↑") | bold | color(Color::CyanLight),
            text("    MOVE UP")
        }),
        hbox({
            text("S / ↓") | bold | color(Color::CyanLight),
            text("    MOVE DOWN")
        }),
        hbox({
            text("A / ←") | bold | color(Color::CyanLight),
            text("    MOVE LEFT")
        }),
        hbox({
            text("D / →") | bold | color(Color::CyanLight),
            text("    MOVE RIGHT")
        })
    });

    auto actions = vbox({
        hbox({
            text("E") | bold | color(Color::CyanLight),
            text("        WAIT ONE TURN")
        }),
        hbox({
            text("H") | bold | color(Color::CyanLight),
            text("        CLOSE HELP")
        }),
        hbox({
            text("Q") | bold | color(Color::CyanLight),
            text("        EXIT")
        })
    });

    auto content = vbox({
        text("MOVEMENT")
        | bold
        | color(Color::CyanLight),

        separatorEmpty(),
        movement,
        separatorEmpty(),

        separator(),

        text("ACTIONS")
        | bold
        | color(Color::CyanLight),

        separatorEmpty(),
        actions,
        separatorEmpty()
    });

    auto controlsWindow = window(
        text(" COMMAND REFERENCE ")
        | bold
        | color(Color::CyanLight),

        hbox({
            text("  "),
            content,
            text("  ")
        })
    );

    return vbox({
        filler(),

        vbox(titleLines),

        text("INPUT PROTOCOL // COMMAND REFERENCE")
        | color(Color::White)
        | bold
        | center,

        separatorEmpty(),
        separatorEmpty(),

        controlsWindow
        | size(WIDTH, EQUAL, 52)
        | center,

        separatorEmpty(),

        text("H - RETURN TO GAME")
        | color(Color::GrayLight)
        | center,

        filler()
    })
    | bgcolor(Color::Black);
}
//Barra Superior
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

//Tablero
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
//Barra Inferior
ftxui::Element ConsoleUI::RenderInfBar(std::span<const NavigationEvent> recentEvents) const {
    std::string message;

    if (!recentEvents.empty()) {
        message =EventMessage(recentEvents.back()) + " | ";
    }
    message += "WASD move | E wait | H help | Q exit";
    return ftxui::text(message);
}


/*=========================*/


/*          Render             */


/*  Render the environment  */
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


/*    Render the cells    */
ftxui::Element ConsoleUI::RenderCell(const Cell& cell, bool IsAgent) const{
    if (IsAgent){
        if (mode_ == RenderMode::emoji){
            return ftxui::text("🤖");
        }else{
            return ftxui::text("@");
        }
    }

    return std::visit(
        [this] (const auto& CurrentCell ) ->ftxui::Element{
            using CellType = std::remove_const_t<std::remove_reference_t<decltype(CurrentCell)>>;
            
            if constexpr(std::is_same_v<CellType, Empty>){
                if (mode_ == RenderMode::emoji){
                    return ftxui::text("⬜");
                }else{
                    return ftxui::text(".");
                }
            }
            if constexpr(std::is_same_v<CellType,Wall>){
                if (mode_ == RenderMode::emoji){
                    return ftxui::text("⬛");
                }else{
                    return ftxui::text("#");
                }
            }
            if constexpr(std::is_same_v<CellType,RoughTerrain>){
                if (mode_ == RenderMode::emoji){
                    return ftxui::text("🟫");
                }else{
                    return ftxui::text("~");
                }
            }
            if constexpr(std::is_same_v<CellType,ResourceCell<int>>){
                if (mode_ == RenderMode::emoji){
                    return ftxui::text("💎");
                }else{
                    return ftxui::text("R");
                }
            }
            if constexpr(std::is_same_v<CellType, Battery>){
                if (mode_ == RenderMode::emoji){
                    return ftxui::text("⚡");
                }else{
                    return ftxui::text("B");
                }
            }
            if constexpr(std::is_same_v<CellType,Trap>){
                if (mode_ == RenderMode::emoji){
                    return ftxui::text("💥");
                }else{
                    return ftxui::text("T");
                }
            }
            if constexpr(std::is_same_v<CellType,Exit>){
                if (mode_ == RenderMode::emoji){
                    return ftxui::text("🏁");
                }else{
                    return ftxui::text("S");
                }
            }   

        }
        
        ,cell);
}
//Evento
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

/*=========================*/




