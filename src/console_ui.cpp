#include "circuit_escape/console_ui.h"
#include "circuit_escape/project2_session.h"
#include "circuit_escape/help.h"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <type_traits>
#include <variant>
#include <ftxui/screen/terminal.hpp>
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
    Help helpView;

    if (!helpReport_.has_value()) {
        return ftxui::vbox({
            helpView.RenderMenu(),

            ftxui::separatorEmpty(),

            ftxui::text("La ayuda de energia aun no esta disponible.")
                | ftxui::color(ftxui::Color::Yellow),

            ftxui::separatorEmpty(),

            ftxui::text("H - Volver a la partida")
        });
    }

    return ftxui::vbox({
        helpView.RenderGame(*helpReport_),

        ftxui::separatorEmpty(),

        ftxui::text("H - Volver a la partida")
    });
}
/*---------------------------------------------------------------------
RenderSupBar() -> Construye la barra de estado con turno, energía, puntaje y recursos.

---------------------------------------------------------------------*/
ftxui::Element ConsoleUI::RenderSupBar( const Observation& observation) const {

    HudReport report = hudReport_.value_or(HudReport{});

    // datos ya disponibles en el estado real del juego
    report.turn = observation.turn;
    report.turn_limit = observation.turnLimit;
    report.energy = observation.energy;
    report.maximum_energy = observation.maximumEnergy;

    const auto lines = format_status(report);

    ftxui::Elements content;

    content.push_back(
        ftxui::text(
            lines[0] + " | " +
            lines[1] +
            " | Score: " + std::to_string(observation.score) +
            " | Resources: " +
            std::to_string(observation.collectedResources)
        )
    );

    if (hudReport_.has_value()) {
        content.push_back(
            ftxui::text(lines[2] + " | " + lines[3])
        );

        content.push_back(
            ftxui::text(lines[4])
        );
    } else {
        content.push_back(
            ftxui::text("A/B y vigilante: datos pendientes de integrar")
        );
    }

    return ftxui::vbox(content);
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
    return render_board_area(20, 30, observation.agent, [&](Position position) {
        const bool is_player = observation.agent == position;
        const bool is_guard = guardPosition_.has_value() && *guardPosition_ == position;
        auto cell = RenderCell(environment.grid().at(position), is_player, is_guard);
        if (std::find(highlightedPath_.begin(), highlightedPath_.end(), position)
            != highlightedPath_.end()) {
            cell = cell | ftxui::bgcolor(ftxui::Color::Blue)
                | ftxui::color(ftxui::Color::White);
        }
        return cell;
    });
}

ftxui::Element ConsoleUI::render_board_area(
    std::size_t rows_count,
    std::size_t columns_count,
    Position player,
    const std::function<ftxui::Element(Position)>& cell_renderer,
    bool numeric_coordinates,
    bool show_coordinates) const {
    using namespace ftxui;

    constexpr std::size_t viewport_rows = 20;
    constexpr std::size_t viewport_columns = 30;
    constexpr int cell_visual_width = 2;

    const std::size_t visible_rows = std::min(rows_count, viewport_rows);
    const std::size_t visible_columns = std::min(columns_count, viewport_columns);
    const std::size_t first_row = std::min(
        player.row > visible_rows / 2 ? player.row - visible_rows / 2 : std::size_t{0},
        rows_count - visible_rows);
    const std::size_t first_column = std::min(
        player.column > visible_columns / 2 ? player.column - visible_columns / 2 : std::size_t{0},
        columns_count - visible_columns);

    const std::size_t top_padding = (viewport_rows - visible_rows) / 2;
    const std::size_t left_padding = (viewport_columns - visible_columns) / 2;

    const std::array<std::string, 10> emoji_digits{
        "0️⃣", "1️⃣", "2️⃣", "3️⃣", "4️⃣", "5️⃣", "6️⃣", "7️⃣", "8️⃣", "9️⃣"
    };
    auto coordinate_label = [&](std::size_t value) {
        return !numeric_coordinates && mode_ == RenderMode::emoji
            ? emoji_digits[value % emoji_digits.size()]
            : std::to_string(value % 10);
    };
    auto empty_space = [] {
        return text("  ") | size(WIDTH, EQUAL, cell_visual_width);
    };

    Elements rows;
    Elements horizontal_labels;
    for (std::size_t view_column = 0; view_column < viewport_columns; ++view_column) {
        if (view_column >= left_padding &&
            view_column < left_padding + visible_columns) {
            horizontal_labels.push_back(CoordinateCell(coordinate_label(
                first_column + view_column - left_padding)));
        } else {
            horizontal_labels.push_back(empty_space());
        }
    }
    if (show_coordinates) {
        rows.push_back(hbox({
            CoordinateCell(mode_ == RenderMode::emoji ? "⬜" : "."),
            hbox(std::move(horizontal_labels))
        }));
    }

    for (std::size_t view_row = 0; view_row < viewport_rows; ++view_row) {
        Elements columns;
        const bool row_in_map = view_row >= top_padding &&
            view_row < top_padding + visible_rows;
        const std::size_t map_row = row_in_map
            ? first_row + view_row - top_padding : 0;
        if (show_coordinates) {
            columns.push_back(row_in_map
                ? CoordinateCell(coordinate_label(map_row)) : empty_space());
        }
        for (std::size_t view_column = 0; view_column < viewport_columns; ++view_column) {
            if (row_in_map && view_column >= left_padding &&
                view_column < left_padding + visible_columns) {
                const Position position{map_row,
                    first_column + view_column - left_padding};
                columns.push_back(cell_renderer(position)
                    | size(WIDTH, EQUAL, cell_visual_width) | center);
            } else {
                columns.push_back(empty_space());
            }
        }
        rows.push_back(hbox(std::move(columns)));
    }
    return vbox(std::move(rows));
}

ftxui::Element ConsoleUI::render_project2_cell(
    const Project2Session& session,
    Position position,
    std::span<const Position> highlighted) const {
    using namespace ftxui;
    const auto& state = session.state();
    const auto& map = session.map();
    const bool emoji = mode_ == RenderMode::emoji;
    Element result;

    if (position == state.player) {
        result = RenderCell(map.grid.at(position), true);
    } else if (position == state.guard) {
        result = text(emoji ? "👀" : "V") | color(Color::RedLight);
    } else if (position == map.input.switch_a) {
        result = text(state.switch_a_active ? "a" : "A")
            | color(Color::YellowLight) | bold;
    } else if (position == map.input.switch_b) {
        result = text(state.switch_b_active ? "b" : "B")
            | color(Color::YellowLight) | bold;
    } else if (session.battery_consumed(position) || session.resource_collected(position)) {
        result = RenderCell(::Cell{Empty{}}, false);
    } else {
        result = RenderCell(map.grid.at(position), false);
    }

    if (std::find(highlighted.begin(), highlighted.end(), position) != highlighted.end()) {
        result = result | bgcolor(Color::Blue);
    }
    return result;
}

ftxui::Element ConsoleUI::render_project2_overview(
    const Project2Session& session,
    std::span<const Position> highlighted) const {
    using namespace ftxui;
    const auto& map = session.map();
    const auto& state = session.state();
    const std::size_t rows = map.grid.rows();
    const std::size_t columns = map.grid.columns();

    auto color_at = [&](Position position) -> Color {
        if (position == state.player) return Color::RGB(44, 191, 255);
        if (position == state.guard) return Color::RGB(155, 62, 174);
        if (position == map.input.switch_a) return Color::RGB(154, 115, 12);
        if (position == map.input.switch_b) return Color::RGB(170, 89, 20);
        if (position == map.input.exit) return Color::RGB(25, 120, 69);
        if (std::find(highlighted.begin(), highlighted.end(), position) != highlighted.end()) {
            return Color::RGB(33, 83, 160);
        }
        if (session.resource_collected(position) || session.battery_consumed(position)) {
            return Color::RGB(214, 218, 223);
        }
        const ::Cell& cell = map.grid.at(position);
        return std::visit([](const auto& value) -> Color {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, Wall>) return Color::RGB(42, 47, 55);
            else if constexpr (std::is_same_v<T, RoughTerrain>) return Color::RGB(133, 82, 65);
            else if constexpr (std::is_same_v<T, ResourceCell<int>>) return Color::RGB(14, 119, 145);
            else if constexpr (std::is_same_v<T, Battery>) return Color::RGB(167, 118, 11);
            else if constexpr (std::is_same_v<T, Trap>) return Color::RGB(170, 41, 57);
            else if constexpr (std::is_same_v<T, Exit>) return Color::RGB(25, 120, 69);
            else return Color::RGB(214, 218, 223);
        }, cell);
    };

    auto marker_at = [&](Position position) -> char {
        if (position == state.player) return '@';
        if (position == state.guard) return 'V';
        if (position == map.input.switch_a) return state.switch_a_active ? 'a' : 'A';
        if (position == map.input.switch_b) return state.switch_b_active ? 'b' : 'B';
        if (position == map.input.exit) return 'S';
        return '\0';
    };

    // Two map rows share one terminal row, without numerical rulers.
    Elements lines;
    for (std::size_t row = 0; row < rows; row += 2) {
        Elements cells;

        for (std::size_t column = 0; column < columns; ++column) {
            const Position upper{row, column};
            const Position lower{std::min(row + 1, rows - 1), column};
            const Color upper_color = color_at(upper);
            const Color lower_color = row + 1 < rows
                ? color_at(lower) : Color::RGB(20, 24, 30);
            const char top_marker = marker_at(upper);
            const char bottom_marker = row + 1 < rows ? marker_at(lower) : '\0';
            if (upper == state.player || (row + 1 < rows && lower == state.player)) {
                const bool in_upper_half = upper == state.player;
                cells.push_back(text("@")
                    | color(in_upper_half
                        ? Color::RGB(255, 255, 255) : Color::RGB(29, 215, 252))
                    | bgcolor(in_upper_half
                        ? Color::RGB(15, 110, 157) : Color::RGB(23, 35, 54))
                    | size(WIDTH, EQUAL, 1));
            } else if (top_marker != '\0' || bottom_marker != '\0') {
                const char marker = top_marker != '\0' ? top_marker : bottom_marker;
                cells.push_back(text(std::string(1, marker))
                    | color(top_marker != '\0' ? upper_color : lower_color)
                    | bgcolor(top_marker != '\0' ? lower_color : upper_color)
                    | size(WIDTH, EQUAL, 1));
            } else {
                cells.push_back(text("▀") | color(upper_color)
                    | bgcolor(lower_color) | size(WIDTH, EQUAL, 1));
            }
        }
        lines.push_back(hbox(std::move(cells)));
    }
    return vbox(std::move(lines));
}

// Scale the complete map to the available terminal area. Use one terminal
// row per map row when there is room; otherwise retain both map rows using
// the upper/lower half-block representation.
ftxui::Element ConsoleUI::render_project2_scaled_overview(
    const Project2Session& session,
    std::span<const Position> highlighted,
    int board_width,
    int board_height) const {

    using namespace ftxui;

    const auto& map = session.map();
    const auto& state = session.state();

    const int rows =
        static_cast<int>(map.grid.rows());

    const int columns =
        static_cast<int>(map.grid.columns());

    const int available_width =
        std::max(1, board_width);

    const int available_height =
        std::max(1, board_height);

    // Use the available horizontal space, but don't stretch cells too much.
    const bool portrait_map =
        rows > columns;

    const int max_cell_width =
        portrait_map ? 3 : 2;

    const int cell_width =
        std::clamp(
            available_width / std::max(1, columns),
            1,
            max_cell_width
        );

    auto color_at = [&](Position position) -> Color {
        if (position == state.player)
            return Color::RGB(15, 110, 157);

        if (position == state.guard)
            return Color::RGB(155, 62, 174);

        if (position == map.input.switch_a)
            return Color::RGB(154, 115, 12);

        if (position == map.input.switch_b)
            return Color::RGB(170, 89, 20);

        if (position == map.input.exit)
            return Color::RGB(25, 120, 69);

        if (std::find(
                highlighted.begin(),
                highlighted.end(),
                position
            ) != highlighted.end()) {

            return Color::RGB(33, 83, 160);
        }

        if (session.resource_collected(position) ||
            session.battery_consumed(position)) {

            return Color::RGB(214, 218, 223);
        }

        return std::visit(
            [](const auto& value) -> Color {
                using T =
                    std::decay_t<decltype(value)>;

                if constexpr (std::is_same_v<T, Wall>)
                    return Color::RGB(42, 47, 55);

                else if constexpr (
                    std::is_same_v<T, RoughTerrain>)
                    return Color::RGB(133, 82, 65);

                else if constexpr (
                    std::is_same_v<T, ResourceCell<int>>)
                    return Color::RGB(14, 119, 145);

                else if constexpr (
                    std::is_same_v<T, Battery>)
                    return Color::RGB(167, 118, 11);

                else if constexpr (
                    std::is_same_v<T, Trap>)
                    return Color::RGB(170, 41, 57);

                else if constexpr (
                    std::is_same_v<T, Exit>)
                    return Color::RGB(25, 120, 69);

                else
                    return Color::RGB(214, 218, 223);
            },
            map.grid.at(position)
        );
    };

    auto marker_at = [&](Position position) -> char {
        if (position == state.player)
            return '@';

        if (position == state.guard)
            return 'V';

        if (position == map.input.switch_a)
            return state.switch_a_active ? 'a' : 'A';

        if (position == map.input.switch_b)
            return state.switch_b_active ? 'b' : 'B';

        if (position == map.input.exit)
            return 'S';

        return '\0';
    };

    auto marker_cell =
        [&](char marker,
            Color foreground,
            Color background) -> Element {

        std::string glyph(
            static_cast<std::size_t>(cell_width),
            ' '
        );

        glyph[
            static_cast<std::size_t>(
                (cell_width - 1) / 2
            )
        ] = marker;

        return text(std::move(glyph))
            | color(foreground)
            | bgcolor(background)
            | bold;
    };

    Elements output;

    // =========================================================
    // CASE 1:
    // Every real map row fits.
    // Do NOT compress vertically.
    // =========================================================
    if (rows <= available_height) {

        for (int row = 0; row < rows; ++row) {

            Elements cells;

            for (int column = 0;
                 column < columns;
                 ++column) {

                const Position position{
                    static_cast<std::size_t>(row),
                    static_cast<std::size_t>(column)
                };

                const Color background =
                    color_at(position);

                const char marker =
                    marker_at(position);

                if (marker != '\0') {

                    cells.push_back(
                        marker_cell(
                            marker,
                            Color::White,
                            background
                        )
                    );

                } else {

                    cells.push_back(
                        text(
                            std::string(
                                static_cast<std::size_t>(
                                    cell_width
                                ),
                                ' '
                            )
                        )
                        | bgcolor(background)
                    );
                }
            }

            output.push_back(
                hbox(std::move(cells))
            );
        }

        return vbox(std::move(output));
    }

    // =========================================================
    // CASE 2:
    // Two real map rows fit inside one terminal row.
    //
    // Example:
    //
    // 70 real rows -> 35 terminal rows.
    //
    // No map information is discarded.
    // =========================================================
    if ((rows + 1) / 2 <= available_height) {

        for (int row = 0; row < rows; row += 2) {

            Elements cells;

            for (int column = 0;
                 column < columns;
                 ++column) {

                const Position upper{
                    static_cast<std::size_t>(row),
                    static_cast<std::size_t>(column)
                };

                const bool has_lower =
                    row + 1 < rows;

                const Position lower{
                    static_cast<std::size_t>(
                        has_lower ? row + 1 : row
                    ),
                    static_cast<std::size_t>(column)
                };

                const Color upper_color =
                    color_at(upper);

                const Color lower_color =
                    has_lower
                        ? color_at(lower)
                        : Color::RGB(23, 35, 54);

                const char upper_marker =
                    marker_at(upper);

                const char lower_marker =
                    has_lower
                        ? marker_at(lower)
                        : '\0';

                // Upper marker has priority.
                if (upper_marker != '\0') {

                    cells.push_back(
                        marker_cell(
                            upper_marker,
                            Color::White,
                            lower_color
                        )
                    );

                } else if (lower_marker != '\0') {

                    cells.push_back(
                        marker_cell(
                            lower_marker,
                            lower_color,
                            upper_color
                        )
                    );

                } else {

                    std::string blocks;

                    for (int i = 0;
                         i < cell_width;
                         ++i) {

                        blocks += "▀";
                    }

                    cells.push_back(
                        text(std::move(blocks))
                        | color(upper_color)
                        | bgcolor(lower_color)
                    );
                }
            }

            output.push_back(
                hbox(std::move(cells))
            );
        }

        return vbox(std::move(output));
    }

    // =========================================================
    // CASE 3:
    // The map is physically too tall even when using 2 rows
    // per terminal row.
    //
    // Only here do we perform real sampling/compression.
    // =========================================================

    const int rendered_rows =
        available_height;

    for (int visual_row = 0;
         visual_row < rendered_rows;
         ++visual_row) {

        Elements cells;

        const int source_row =
            visual_row * rows /
            rendered_rows;

        for (int column = 0;
             column < columns;
             ++column) {

            const Position position{
                static_cast<std::size_t>(source_row),
                static_cast<std::size_t>(column)
            };

            const Color background =
                color_at(position);

            const char marker =
                marker_at(position);

            if (marker != '\0') {

                cells.push_back(
                    marker_cell(
                        marker,
                        Color::White,
                        background
                    )
                );

            } else {

                cells.push_back(
                    text(
                        std::string(
                            static_cast<std::size_t>(
                                cell_width
                            ),
                            ' '
                        )
                    )
                    | bgcolor(background)
                );
            }
        }

        output.push_back(
            hbox(std::move(cells))
        );
    }

    return vbox(std::move(output));
}

// Display every tile of small maps while retaining the original symbols.
// Unlike the compact 40x60 overview, this never replaces items with color dots.
ftxui::Element ConsoleUI::render_project2_fitted_map(
    const Project2Session& session,
    std::span<const Position> highlighted,
    int board_width, int board_height) const {
    using namespace ftxui;
    const auto& grid = session.map().grid;
    const int rows = static_cast<int>(grid.rows());
    const int columns = static_cast<int>(grid.columns());
    const int cell_width = std::clamp(board_width / columns, 2, 8);
    const int cell_height = std::clamp(board_height / rows, 1, 4);

    Elements board_rows;
    for (int row = 0; row < rows; ++row) {
        Elements board_columns;
        for (int column = 0; column < columns; ++column) {
            const Position position{static_cast<std::size_t>(row),
                                    static_cast<std::size_t>(column)};
            board_columns.push_back(
                render_project2_cell(session, position, highlighted)
                | center
                | size(WIDTH, EQUAL, cell_width)
                | size(HEIGHT, EQUAL, cell_height));
        }
        board_rows.push_back(hbox(std::move(board_columns)));
    }
    return vbox(std::move(board_rows));
}

// Player-centered zoom for small maps. Movement changes only the viewport;
// it never changes the map, player state, or turn count.
ftxui::Element ConsoleUI::render_project2_zoom_detail(
    const Project2Session& session,
    std::span<const Position> highlighted,
    int board_width, int board_height) const {
    using namespace ftxui;
    const auto& grid = session.map().grid;
    const Position player = session.state().player;
    const int rows = static_cast<int>(grid.rows());
    const int columns = static_cast<int>(grid.columns());
    const int cell_width = std::min(board_width, board_width >= 90 ? 8 : 6);
    const int cell_height = std::min(board_height, board_height >= 32 ? 4 : 3);
    const int visible_columns = std::clamp(board_width / cell_width, 1, columns);
    const int visible_rows = std::clamp(board_height / cell_height, 1, rows);
    const int first_row = std::clamp(
        static_cast<int>(player.row) - visible_rows / 2, 0, rows - visible_rows);
    const int first_column = std::clamp(
        static_cast<int>(player.column) - visible_columns / 2, 0, columns - visible_columns);

    // Fill the entire enlarged tile, not just its centered emoji. Otherwise
    // neighboring passages look disconnected when zoomed to multiple columns
    // and rows. The original emoji remains visible in the tile's center.
    const auto tile_background = [&](Position position) -> Color {
        if (std::find(highlighted.begin(), highlighted.end(), position) != highlighted.end()) {
            return Color::RGB(30, 83, 141);
        }
        if (position == session.state().player) return Color::RGB(23, 96, 117);
        if (position == session.state().guard) return Color::RGB(88, 45, 110);
        if (session.resource_collected(position) || session.battery_consumed(position)) {
            return Color::RGB(73, 80, 91);
        }
        return std::visit([](const auto& cell) -> Color {
            using CellType = std::decay_t<decltype(cell)>;
            if constexpr (std::is_same_v<CellType, Wall>) return Color::RGB(14, 17, 24);
            else if constexpr (std::is_same_v<CellType, RoughTerrain>) return Color::RGB(109, 70, 57);
            else if constexpr (std::is_same_v<CellType, ResourceCell<int>>) return Color::RGB(35, 79, 98);
            else if constexpr (std::is_same_v<CellType, Battery>) return Color::RGB(106, 82, 35);
            else if constexpr (std::is_same_v<CellType, Trap>) return Color::RGB(106, 44, 56);
            else if constexpr (std::is_same_v<CellType, Exit>) return Color::RGB(37, 88, 59);
            else if constexpr (std::is_same_v<CellType, Switch>) return Color::RGB(104, 77, 38);
            else return Color::RGB(73, 80, 91);
        }, grid.at(position));
    };

    Elements board_rows;
    for (int row = 0; row < visible_rows; ++row) {
        Elements board_columns;
        for (int column = 0; column < visible_columns; ++column) {
            const Position position{static_cast<std::size_t>(first_row + row),
                                    static_cast<std::size_t>(first_column + column)};
            board_columns.push_back(
                render_project2_cell(session, position, highlighted)
                | center
                | size(WIDTH, EQUAL, cell_width)
                | size(HEIGHT, EQUAL, cell_height)
                | bgcolor(tile_background(position)));
        }
        board_rows.push_back(hbox(std::move(board_columns)));
    }
    return vbox(std::move(board_rows));
}
ftxui::Element ConsoleUI::render_project2_board(
    const Project2Session& session,
    std::span<const Position> highlighted,
    Project2MapView view,
    int board_width,
    int board_height) const {

    const auto& map = session.map();

    const int rows =
        static_cast<int>(map.grid.rows());

    const int columns =
        static_cast<int>(map.grid.columns());

    // DETAIL is ALWAYS a real player-centered zoom.
    if (view == Project2MapView::detail &&
        board_width > 0 &&
        board_height > 0) {

        return render_project2_zoom_detail(
            session,
            highlighted,
            board_width,
            board_height
        );
    }

    if (view == Project2MapView::overview &&
        board_width > 0 &&
        board_height > 0) {

        const bool normal_fits =
            columns * 2 <= board_width &&
            rows <= board_height;

        if (normal_fits) {
            return render_project2_fitted_map(
                session,
                highlighted,
                board_width,
                board_height
            );
        }

        // Generic adaptive FULL MAP.
        return render_project2_scaled_overview(
            session,
            highlighted,
            board_width,
            board_height
        );
    }

    // Preserve the old direct-call behavior used by some tests.
    if (view == Project2MapView::overview &&
        (map.grid.rows() > 20 ||
         map.grid.columns() > 30) &&
        map.grid.rows() <= 40 &&
        map.grid.columns() <= 60) {

        return render_project2_overview(
            session,
            highlighted
        );
    }

    return render_board_area(
        map.grid.rows(),
        map.grid.columns(),
        session.state().player,
        [&](Position position) {
            return render_project2_cell(
                session,
                position,
                highlighted
            );
        },
        true,
        false
    );
}
ftxui::Element ConsoleUI::render_project2(
    const Project2Session& session,
    Project2MapView view,
    std::span<const Position> highlighted,
    std::span<const std::string> help_lines,
    std::optional<ftxui::Dimensions> terminal_size) const {
    using namespace ftxui;
    const auto& state = session.state();
    const auto& map = session.map();
    std::string difficulty;
    switch (map.difficulty) {
        case SavedDifficulty::easy: difficulty = "EASY"; break;
        case SavedDifficulty::medium: difficulty = "MEDIUM"; break;
        case SavedDifficulty::hard: difficulty = "HARD"; break;
    }

    const std::string turns = "Turn " + std::to_string(state.turn) + "/" +
        std::to_string(map.input.turn_limit);
    const std::string energy = "Energy " + std::to_string(state.energy) + "/" +
        std::to_string(map.input.maximum_energy);
    const std::string score = "Score " + std::to_string(state.score);
    const std::string resources = "Resources " + std::to_string(state.resource_count) +
        "/" + std::to_string(map.effects.resources.size());
    const std::string switches = std::string("Switch A ") +
        (state.switch_a_active ? "ON" : "OFF") + " | B " +
        (state.switch_b_active ? "ON" : "OFF");
    const std::string player = "Player (" + std::to_string(state.player.row) + "," +
        std::to_string(state.player.column) + ")";
    const std::string guard = "Guard (" + std::to_string(state.guard.row) + "," +
        std::to_string(state.guard.column) + ")";
    const std::string next_guard_turn = "Next move: turn " +
        std::to_string(state.turn % 2 == 0 ? state.turn + 2 : state.turn + 1);

    const ftxui::Dimensions terminal = terminal_size.value_or(ftxui::Terminal::Size());
    // Keep the interface proportional to the actual map instead of
    // stretching empty panel borders to the entire terminal width.
    const bool spacious =
        terminal.dimx >= 105 &&
        terminal.dimy >= 43;

    const int map_rows =
        static_cast<int>(map.grid.rows());

    const int map_columns =
        static_cast<int>(map.grid.columns());

    const int target_cell_width =
        map_columns >= 55 ? 2 :
        (map_columns >= 25 ? 3 : 4);

    const int desired_panel_width =
        std::max(
            76,
            map_columns *
                target_cell_width + 8
        );

    const int available_panel_width =
        std::max(
            20,
            terminal.dimx - 8
        );

    const int panel_width =
        spacious
            ? std::min(
                available_panel_width,
                desired_panel_width
            )
            : std::min(
                76,
                available_panel_width
            );

    const int board_width =
        std::max(
            1,
            panel_width - 6
        );

    const bool normal_width_fits =
        board_width >=
        map_columns * 2;

    int ideal_board_height;

    if (map_rows <= 10 &&
        normal_width_fits) {

        ideal_board_height = 34;

    } else if (
        map_rows <= 20 &&
        normal_width_fits) {

        ideal_board_height = 44;

    } else {

        ideal_board_height =
            std::clamp(
                map_rows + 4,
                12,
                44
            );
    }

    const int board_height =
        spacious
            ? std::min(
                terminal.dimy - 18,
                ideal_board_height
            )
            : 22;
    constexpr int footer_height = 7;
    const bool can_render_full_tiles =
        board_width >= map_columns * 2 &&
        board_height >= map_rows;

    const bool compact =
        view == Project2MapView::overview &&
        !can_render_full_tiles;

    auto sup_window = window(
        text(" STATUS ") | bold | color(Color::CyanLight),
        hbox({
            text(" "),
            vbox({
                text(turns + " | " + energy + " | " + score + " | " + resources),
                text(difficulty + " | " + std::to_string(map.grid.rows()) + "x" +
                     std::to_string(map.grid.columns()) + " | " + switches),
                text(player + " | " + guard + " | " + next_guard_turn)
            }),
            text(" ")
        })
    ) | size(WIDTH, EQUAL, panel_width);

    // All map views share one fixed-size board slot. Zoom and help update
    // its contents instead of moving the surrounding game panels.
    auto map_content = render_project2_board(
        session,
        highlighted,
        view,
        panel_width - 6,
        board_height
    );
    // Center the contents in the same slot, independently of Z and H.
    auto board = vbox({
        filler(),
        hbox({filler(), std::move(map_content), filler()}),
        filler()
    }) | size(WIDTH, EQUAL, panel_width - 4) | size(HEIGHT, EQUAL, board_height);
    auto board_window = window(
        text(" MAZE ") | bold | color(Color::CyanLight),
        hbox({text(" "), std::move(board), filler()})
    ) | size(WIDTH, EQUAL, panel_width);

    Elements footer_lines;
    if (!help_lines.empty()) {
        for (const auto& line : help_lines) {
            footer_lines.push_back(text(" " + line));
        }
    } else {
        footer_lines.push_back(text(" Move: WASD/arrows (S = down) | Wait: E | Zoom: Z"));
        footer_lines.push_back(text(std::string(" View: ") +
            (view == Project2MapView::overview ? "FULL MAP" : "DETAIL") +
            " | Help: H | Quit: Q"));
        const bool ascii_legend = mode_ == RenderMode::ascii;
        footer_lines.push_back(compact
            ? text(" @ player | V guard | A/B switches | S = goal tile")
            : ascii_legend
                ? text(" A/B switches | V guard | @ player | S exit")
                : text(" A/B switches | Guard: eyes | S = down; goal is on the map"));
        footer_lines.push_back(compact
            ? text(" Colors: teal resource | amber battery | burgundy trap | green goal")
            : ascii_legend
                ? text(" Resources: R | Batteries: + | Traps: T | Walls: #")
                : text(" Resources: diamonds | Batteries: lightning | Traps: explosions"));
    }
    while (footer_lines.size() < 5) {
        footer_lines.push_back(text(" "));
    }
    auto inf_window = window(
        text(help_lines.empty() ? " EVENTS / CONTROLS " : " DIJKSTRA ENERGY HELP ")
            | bold | color(Color::CyanLight),
        vbox(std::move(footer_lines))
    ) | size(WIDTH, EQUAL, panel_width) | size(HEIGHT, EQUAL, footer_height);

    return vbox({
        std::move(sup_window),
        separatorEmpty(),
        std::move(board_window),
        separatorEmpty(),
        std::move(inf_window)
    });
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
        // Mientras no haya reporte, mostramos la ayuda general.
        if (!helpReport_.has_value()) {
            return help();
        }

        const Observation observation = environment.state();

        auto boardWindow = ftxui::window(
            ftxui::text(" MAPA Y RUTA ")
                | ftxui::bold
                | ftxui::color(ftxui::Color::CyanLight),

            RenderBoard(environment, observation)
        );

        return ftxui::vbox({
            help(),

            ftxui::separatorEmpty(),

            boardWindow
        }) | ftxui::center;
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
ftxui::Element ConsoleUI::RenderCell( const Cell& cell, bool IsAgent, bool IsGuardian) const {

    // El jugador tiene prioridad sobre la casilla que ocupa.
    //El vigilante igual
    if (IsAgent) {
        return ftxui::text(symbol_for(GameSymbol::player));
    }

    if (IsGuardian) {
        return ftxui::text(symbol_for(GameSymbol::guard));
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

/*---------------------------------------------------------------------
format_Status() -> Establece el formato con el que se renderizara
el reporte de estaod:

1. Turno
2. Energia
3. Estado de los switches A y B
4. Movimientos del vigilante
---------------------------------------------------------------------*/
std::vector<std::string> ConsoleUI::format_status(
    const HudReport& report
) const {
    return {
        "Turno: " +
            std::to_string(report.turn) + "/" +
            std::to_string(report.turn_limit),

        "Energia: " +
            std::to_string(report.energy) + "/" +
            std::to_string(report.maximum_energy),

        std::string("A: ") +
            (report.switch_a_activated ? "Activado" : "Pendiente"),

        std::string("B: ") +
            (report.switch_b_activated ? "Activado" : "Pendiente"),

        report.guard_moves_next_turn
            ? "Vigilante: avanzara despues de tu siguiente accion."
            : "Vigilante: permanecera quieto despues de tu siguiente accion."
    };
}

void ConsoleUI::highlightHelpPath(
    const HelpReport& report,
    HelpPathSelection selection
) {
    switch (selection) {
        case HelpPathSelection::none:
            clearHighlightedPath();
            break;

        case HelpPathSelection::to_a:
            setHighlightedPath(report.path_to_a);
            break;

        case HelpPathSelection::to_b:
            setHighlightedPath(report.path_to_b);
            break;

        case HelpPathSelection::complete:
            setHighlightedPath(report.complete_path);
            break;
    }
}