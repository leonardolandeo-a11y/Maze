#pragma once

#include "circuit_escape/environment.h"
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>
#include <functional>
#include <optional>
#include <span>
#include <string>

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
enum class Project2MapView { overview, detail };

class Project2Session;

class ConsoleUI {
private:
    RenderMode mode_;
    bool help_{false};
    std::string inputMessage_;

    ftxui::Element RenderCell(const Cell& cell, bool IsAgent) const;
    //metodos aux agregados para mantener limpio el codigo
    ftxui::Element RenderSupBar(const Observation& observation) const;
    ftxui::Element RenderBoard(const NavigationEnvironment<20, 30>& environment, 
        const Observation& observation) const;
    ftxui::Element render_board_area(
        std::size_t rows,
        std::size_t columns,
        Position player,
        const std::function<ftxui::Element(Position)>& cell_renderer,
        bool numeric_coordinates = false,
        bool show_coordinates = true) const;
    ftxui::Element render_project2_overview(
        const Project2Session& session,
        std::span<const Position> highlighted) const;
    ftxui::Element render_project2_scaled_overview(
        const Project2Session& session,
        std::span<const Position> highlighted,
        int board_width, int board_height) const;
    ftxui::Element render_project2_cell(
        const Project2Session& session,
        Position position,
        std::span<const Position> highlighted) const;
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
    
    ftxui::Element render_project2(
        const Project2Session& session,
        Project2MapView view = Project2MapView::overview,
        std::span<const Position> highlighted = {},
        std::span<const std::string> help_lines = {},
        std::optional<ftxui::Dimensions> terminal_size = std::nullopt) const;

    ftxui::Element render_project2_board(
        const Project2Session& session,
        std::span<const Position> highlighted = {},
        Project2MapView view = Project2MapView::overview,
        int board_width = 0, int board_height = 0) const;

    std::optional<UICommand> KeyMapping(const ftxui::Event& event) const;

    ftxui::Element help() const;

    void setHelp() noexcept {
        help_ = !help_;
    }
    [[nodiscard]]bool isHelpVisible() const noexcept {
        return help_;
    }
    void setInputMessage(const std::string& message) {
        inputMessage_ = message;
    }

    void clearInputMessage() {
        inputMessage_.clear();
    }
};