#pragma once
#include <ftxui/dom/elements.hpp>

namespace UILayout{
    /*      Center Game View configuration       */

    inline ftxui::Element CenterGameView(ftxui::Element content) {

        constexpr int gameViewWidth = 90;
        constexpr int gameViewHeight = 20;

        ftxui::Element fixedView =content |
            ftxui::size(ftxui::WIDTH, ftxui::EQUAL, gameViewWidth) |
                ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, gameViewHeight);
        

        return ftxui::vbox({
            ftxui::filler(),
            ftxui::hbox({
                ftxui::filler(),
                fixedView,
                ftxui::filler()
            }),
            ftxui::filler()
        }) | ftxui::bgcolor(ftxui::Color::Black);
    }

    /*=========================*/
}