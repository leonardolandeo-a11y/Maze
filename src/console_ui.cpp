#include "circuit_escape/console_ui.h"

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

ftxui::Element ConsoleUI::render(const NavigationEnvironment<20, 30>& environment,std::span<const NavigationEvent> recentEvents) const{

    return ftxui::vbox({
        ftxui::text("Game")
    });
}

/*=========================*/
