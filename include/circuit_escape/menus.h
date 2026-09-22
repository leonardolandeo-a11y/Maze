#pragma once

#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

enum class MenuScreen {
    Main,
    Scenario,
    Difficulty,
    RenderMode,
    Help
};

class Menus {
public:
    [[nodiscard]] ftxui::Element Render() const;
    bool OnEvent(ftxui::Event event);

private:
    MenuScreen currentScreen_ = MenuScreen::Main;
    int selectedOption_ = 0;
};
