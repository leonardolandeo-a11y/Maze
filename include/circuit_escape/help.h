#pragma once

#include "circuit_escape/project2_contracts.h"
#include <ftxui/dom/elements.hpp>

class Help {
public:
    // Ayuda general: instrucciones, controles y objetivos del juego
    ftxui::Element RenderMenu() const;

    // Ayuda durante la partida: objetivos, costos y recorrido
    ftxui::Element RenderGame(const HelpReport& report) const;
};
