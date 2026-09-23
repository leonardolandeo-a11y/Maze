#pragma once

#include "circuit_escape/environment.h"
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <optional>
#include <span>

enum class RenderMode {
    emoji,
    ascii
};

enum class UICommand{
    up,
    down,
    left,
    right,
    wait,
    help,
    quit
};
class ConsoleUI {
private:
    RenderMode mode_;
    bool help_{false};

    ftxui::Element RenderCell(const Cell& cell, bool IsAgent) const;
    //metodos aux agregados para mantener limpio el codigo
    ftxui::Element RenderSupBar(const Observation& observation) const;
    ftxui::Element RenderBoard(const NavigationEnvironment<20, 30>& environment, 
        const Observation& observation) const;
    ftxui::Element RenderInfBar(std::span<const NavigationEvent> recentEvents) const;
    /*
    CoordinteCell: Encapsula la configuración de coordenadas 
    EventMessage: Encapsula la logica para representar el ultimo evento en la esquina del tablero
    */
    ftxui::Element CoordinateCell(const std::string& label) const;
    std::string EventMessage(const NavigationEvent& event) const;
public:
    explicit ConsoleUI(RenderMode mode = RenderMode::emoji);

    ftxui::Element render(
        const NavigationEnvironment<20, 30>& environment,
        std::span<const NavigationEvent> recentEvents) const;
    
    std::optional<UICommand> KeyMapping(const ftxui::Event& event) const;
    //Se agrega Help y getter/setter
    ftxui::Element help() const;

    void setHelp() noexcept {
        help_ = !help_;
    }
    [[nodiscard]]bool isHelpVisible() const noexcept {
        return help_;
    }
};