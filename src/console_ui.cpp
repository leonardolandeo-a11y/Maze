#include "circuit_escape/console_ui.h"
#include <variant>
#include <type_traits>

/*   Constructor of the console_ui   */
ConsoleUI::ConsoleUI(RenderMode mode): mode_(mode){};

/*=========================*/

/*   Translate keys    */

std::optional<UICommand> ConsoleUI::KeyMapping(const ftxui::Event& event)const{
    if (event == ftxui::Event::Character('w') || event == ftxui::Event::Character('W')){
        return UICommand::up;
    }
    if (event == ftxui::Event::Character('s')|| event == ftxui::Event::Character('S')){
        return UICommand::down;
    }
    if (event == ftxui::Event::Character('d') || event == ftxui::Event::Character('D')){
        return UICommand::right;
    }
    if (event == ftxui::Event::Character('a')|| event == ftxui::Event::Character('a')){
        return UICommand::left;
    }
    if (event == ftxui::Event::Character('q')|| event == ftxui::Event::Character('Q')){
        return UICommand::quit;
    }
    if (event == ftxui::Event::Character('e')|| event == ftxui::Event::Character('E')){
        return UICommand::wait;
    }
    return std::nullopt;

}



/*=========================*/


/*          Render             */

/*  Render the environment  */
ftxui::Element ConsoleUI::render(
    const NavigationEnvironment<20, 30>& environment,
    std::span<const NavigationEvent> recentEvents
) const {
    constexpr int cellVisualWidth = 3;

    std::vector<ftxui::Element> rows;
    Observation observation = environment.state();

    for (std::size_t row = 0; row < 20; row++){
        std::vector<ftxui::Element> columns;

        for (std::size_t column = 0; column < 30; column++){
            Position position{row,column};

            const Cell& cell = environment.grid().at(position);

            bool isAgent = false;

            if (observation.agent == position){
                isAgent = true;
            }

            columns.push_back(
                RenderCell(cell, isAgent) |
                ftxui::size(ftxui::WIDTH, ftxui::EQUAL, cellVisualWidth) |
                ftxui::center
            );
        }

        rows.push_back(ftxui::hbox(std::move(columns)));
    }

    return ftxui::vbox(std::move(rows));
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


/*=========================*/
