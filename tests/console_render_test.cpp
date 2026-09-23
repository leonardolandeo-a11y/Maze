#include <cassert>
#include <sstream>
#include <string>
#include <vector>

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

#include "circuit_escape/console_ui.h"


// Crea un entorno 20x30 controlado que contiene todas las clases de celda.
NavigationEnvironment<20, 30> createRenderTestEnvironment() {
    Grid<Cell, 20, 30> grid;

    // Primera fila: todas las clases de celda.
    grid.at({0, 0}) = Empty{};
    grid.at({0, 1}) = Wall{};
    grid.at({0, 2}) = RoughTerrain{};
    grid.at({0, 3}) = ResourceCell<int>{10, false};
    grid.at({0, 4}) = Battery{3, false};
    grid.at({0, 5}) = Trap{};
    grid.at({0, 6}) = Exit{};

    GameRules rules = rulesFor(Difficulty::standard);

    // Se coloca al agente lejos de la primera fila para no cubrir
    // ninguna de las celdas que queremos comprobar.
    Agent agent(
        {19, 29},
        rules.initialEnergy,
        rules.maximumEnergy
    );

    return NavigationEnvironment<20, 30>(
        grid,
        agent,
        rules
    );
}


// Crea un entorno especialmente diseñado para comprobar las dimensiones
// del tablero renderizado.
//
// Cada fila contiene exactamente:
// - 29 Wall
// - 1 celda transitable
//
// Por tanto, una fila válida debe mostrar exactamente 29 símbolos de muro.
// Hay 20 filas, así que deben encontrarse exactamente 20 líneas de ese tipo.
NavigationEnvironment<20, 30> createDimensionTestEnvironment() {
    Grid<Cell, 20, 30> grid;

    for (std::size_t row = 0; row < 20; ++row) {
        for (std::size_t column = 0; column < 30; ++column) {
            grid.at({row, column}) = Wall{};
        }

        // Una celda no-muro por fila.
        grid.at({row, 0}) = Empty{};
    }

    // El entorno exige exactamente una salida.
    grid.at({0, 0}) = Exit{};

    GameRules rules = rulesFor(Difficulty::standard);

    // La posición inicial debe ser transitable.
    Agent agent(
        {19, 0},
        rules.initialEnergy,
        rules.maximumEnergy
    );

    return NavigationEnvironment<20, 30>(
        grid,
        agent,
        rules
    );
}


// Convierte el resultado de render() de FTXUI a std::string.
std::string renderToString(
    const ConsoleUI& ui,
    const NavigationEnvironment<20, 30>& environment
) {
    const std::vector<NavigationEvent> events;

    auto element = ui.render(
        environment,
        std::span<const NavigationEvent>(events)
    );

    // Usamos un tamaño fijo suficientemente grande para evitar que
    // el layout centrado de FTXUI comprima o recorte el tablero durante el test.
    auto screen = ftxui::Screen::Create(
        ftxui::Dimension::Fixed(120),
        ftxui::Dimension::Fixed(50)
    );

    ftxui::Render(screen, element);

    return screen.ToString();
}


// Cuenta cuántas veces aparece un token dentro de un string.
std::size_t countOccurrences(
    const std::string& text,
    const std::string& token
) {
    std::size_t count = 0;
    std::size_t position = 0;

    while (
        (position = text.find(token, position))
        != std::string::npos
    ) {
        ++count;
        position += token.size();
    }

    return count;
}


// Verifica que todas las clases de celda tengan representación ASCII.
void test_ascii_renders_all_cell_types() {
    auto environment = createRenderTestEnvironment();

    ConsoleUI ui(RenderMode::ascii);

    const std::string output =
        renderToString(ui, environment);

    assert(output.find('.') != std::string::npos);
    assert(output.find('#') != std::string::npos);
    assert(output.find('~') != std::string::npos);
    assert(output.find('R') != std::string::npos);
    assert(output.find('B') != std::string::npos);
    assert(output.find('T') != std::string::npos);
    assert(output.find('S') != std::string::npos);
}


// Verifica que el tablero ASCII conserve 20 filas y 30 celdas por fila.
//
// El entorno de prueba tiene 29 muros por fila y una celda no-muro.
// Por ello deben existir exactamente 20 líneas con 29 caracteres '#'.
void test_ascii_board_has_20_rows_and_30_cells() {
    auto environment = createDimensionTestEnvironment();

    ConsoleUI ui(RenderMode::ascii);

    const std::string output =
        renderToString(ui, environment);

    std::istringstream stream(output);

    std::string line;
    std::size_t boardRows = 0;

    while (std::getline(stream, line)) {
        const std::size_t wallCount =
            countOccurrences(line, "#");

        if (wallCount == 29) {
            ++boardRows;
        }
    }

    assert(boardRows == 20);
}


// Verifica que todas las clases de celda tengan representación emoji.
void test_emoji_renders_all_cell_types() {
    auto environment = createRenderTestEnvironment();

    ConsoleUI ui(RenderMode::emoji);

    const std::string output =
        renderToString(ui, environment);

    assert(output.find("⬜") != std::string::npos);
    assert(output.find("⬛") != std::string::npos);
    assert(output.find("🟫") != std::string::npos);
    assert(output.find("💎") != std::string::npos);
    assert(output.find("⚡") != std::string::npos);
    assert(output.find("💥") != std::string::npos);
    assert(output.find("🏁") != std::string::npos);
}


// Verifica que el tablero emoji conserve 20 filas y 30 celdas por fila.
//
// El entorno de prueba tiene 29 muros por fila y una celda no-muro.
// Por ello deben existir exactamente 20 líneas con 29 símbolos "⬛".
void test_emoji_board_has_20_rows_and_30_cells() {
    auto environment = createDimensionTestEnvironment();

    ConsoleUI ui(RenderMode::emoji);

    const std::string output =
        renderToString(ui, environment);

    std::istringstream stream(output);

    std::string line;
    std::size_t boardRows = 0;

    while (std::getline(stream, line)) {
        const std::size_t wallCount =
            countOccurrences(line, "⬛");

        if (wallCount == 29) {
            ++boardRows;
        }
    }

    assert(boardRows == 20);
}


// Punto de entrada para integrarlo con tests/test_main.cpp.
void run_console_render_tests() {
    test_ascii_renders_all_cell_types();
    test_ascii_board_has_20_rows_and_30_cells();

    test_emoji_renders_all_cell_types();
    test_emoji_board_has_20_rows_and_30_cells();
}
