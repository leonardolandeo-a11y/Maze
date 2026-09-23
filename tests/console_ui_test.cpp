#include <cassert>

#include "circuit_escape/console_ui.h"


void test_key_mapping_lowercase() {
    ConsoleUI ui;

    assert(ui.KeyMapping(ftxui::Event::Character('w'))== UICommand::up);
    assert(ui.KeyMapping(ftxui::Event::Character('s'))== UICommand::down);
    assert(ui.KeyMapping(ftxui::Event::Character('a'))== UICommand::left);
    assert(ui.KeyMapping(ftxui::Event::Character('d'))== UICommand::right);
    assert(ui.KeyMapping(ftxui::Event::Character('e'))== UICommand::wait);
    assert(ui.KeyMapping(ftxui::Event::Character('h'))== UICommand::help);
    assert(ui.KeyMapping(ftxui::Event::Character('q'))== UICommand::quit);
}


void test_key_mapping_uppercase() {
    ConsoleUI ui;

    assert(ui.KeyMapping(ftxui::Event::Character('W'))== UICommand::up);
    assert(ui.KeyMapping(ftxui::Event::Character('S'))== UICommand::down);
    assert(ui.KeyMapping(ftxui::Event::Character('A'))== UICommand::left);
    assert(ui.KeyMapping(ftxui::Event::Character('D'))== UICommand::right);
    assert(ui.KeyMapping(ftxui::Event::Character('E'))== UICommand::wait);
    assert(ui.KeyMapping(ftxui::Event::Character('H'))== UICommand::help);
    assert(ui.KeyMapping(ftxui::Event::Character('Q'))== UICommand::quit);
}


void test_unknown_key_returns_nullopt() {
    ConsoleUI ui;

    const auto command =
        ui.KeyMapping(
            ftxui::Event::Character('x')
        );

    assert(!command.has_value());
}


void run_console_ui_tests() {
    test_key_mapping_lowercase();
    test_key_mapping_uppercase();
    test_unknown_key_returns_nullopt();
}