#include <cassert>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

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

    // Celdas que funcionan como los interruptores
    grid.at({0, 7}) = Switch{SwitchId::a, false};
    grid.at({0, 8}) = Switch{SwitchId::a, true};

    grid.at({0, 9}) = Switch{SwitchId::b, false};
    grid.at({0, 10}) = Switch{SwitchId::b, true};

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
    assert(output.find('+') != std::string::npos);
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

void test_switches_render() {
    auto environment = createRenderTestEnvironment();

    for (RenderMode mode : {RenderMode::ascii, RenderMode::emoji}) {
        ConsoleUI ui(mode);

        std::string output = renderToString(ui, environment);

        // Quitamos los espacios de separación entre las celdas.
        output.erase(
            std::remove(output.begin(), output.end(), ' '),
            output.end()
        );

        const std::string expected =
            mode == RenderMode::ascii
                ? "AaBb"
                : "🔴🟢🟥🟩";

        if (output.find(expected) == std::string::npos) {
            std::cerr << "\nEsperado: [" << expected << "]\n";
            std::cerr << "Renderizado:\n" << output << "\n";
        }

        assert(output.find(expected) != std::string::npos);
    }
}

void test_hud_renders_switches_and_guard_status() {
    auto environment = createRenderTestEnvironment();
    ConsoleUI ui(RenderMode::ascii);

    // Antes de recibir información del motor.
    std::string output = renderToString(ui, environment);
    assert(
        output.find("datos pendientes de integrar")
        != std::string::npos
    );

    // Primer reporte: A activo, B pendiente y vigilante por avanzar.
    HudReport report;
    report.switch_a_activated = true;
    report.switch_b_activated = false;
    report.guard_moves_next_turn = true;

    ui.setHudReport(report);

    output = renderToString(ui, environment);

    assert(
        output.find("A: Activado | B: Pendiente")
        != std::string::npos
    );

    assert(
        output.find("Vigilante: avanzara")
        != std::string::npos
    );

    assert(
        output.find("datos pendientes de integrar")
        == std::string::npos
    );

    // Segundo reporte: cambian los interruptores y el aviso.
    report.switch_a_activated = false;
    report.switch_b_activated = true;
    report.guard_moves_next_turn = false;

    ui.setHudReport(report);

    output = renderToString(ui, environment);

    assert(
        output.find("A: Pendiente | B: Activado")
        != std::string::npos
    );

    assert(
        output.find("Vigilante: permanecera quieto")
        != std::string::npos
    );
}

void test_guard_rendering() {
    auto environment = createRenderTestEnvironment();

    for (RenderMode mode : {RenderMode::ascii, RenderMode::emoji}) {
        ConsoleUI ui(mode);

        const std::string original =
            renderToString(ui, environment);

        // Colocamos al vigilante sobre el interruptor A pendiente.
        ui.setGuardPosition(Position{0, 7});

        std::string output = renderToString(ui, environment);

        output.erase(
            std::remove(output.begin(), output.end(), ' '),
            output.end()
        );

        // El vigilante debe cubrir solamente el primer interruptor.
        const std::string expected =
            mode == RenderMode::ascii
                ? "VaBb"
                : "👮🟢🟥🟩";

        assert(output.find(expected) != std::string::npos);

        // Al retirarlo, debe reaparecer el contenido original.
        ui.clearGuardPosition();

        output = renderToString(ui, environment);

        assert(output == original);

        // Si coincide con el jugador, se mantiene su prioridad visual.
        ui.setGuardPosition(environment.state().agent);

        output = renderToString(ui, environment);

        assert(output == original);
    }
}

void test_path_highlighting() {
    auto environment = createRenderTestEnvironment();
    ConsoleUI ui(RenderMode::ascii);

    auto renderScreen = [&]() {
        auto screen = ftxui::Screen::Create(
            ftxui::Dimension::Fixed(120),
            ftxui::Dimension::Fixed(50)
        );

        ftxui::Render(screen, ui.render(environment, {}));

        return screen;
    };

    const auto original = renderScreen();

    // Camino de ejemplo sobre celdas libres hasta el jugador.
    ui.setHighlightedPath({
        Position{19, 27},
        Position{19, 28},
        Position{19, 29}
    });

    const auto highlighted = renderScreen();

    bool foundBlueBackground = false;
    bool foundHighlightedPlayer = false;

    for (int y = 0; y < highlighted.dimy(); ++y) {
        for (int x = 0; x < highlighted.dimx(); ++x) {
            const auto& cell = highlighted.CellAt(x, y);
            const auto& previous = original.CellAt(x, y);

            // Resaltar no debe cambiar ningún símbolo.
            assert(cell.character == previous.character);

            if (cell.background_color == ftxui::Color::Blue) {
                foundBlueBackground = true;

                if (cell.character == "@") {
                    foundHighlightedPlayer = true;
                }
            }
        }
    }

    assert(foundBlueBackground);
    assert(foundHighlightedPlayer);

    // Limpiar debe restaurar los colores originales.
    ui.clearHighlightedPath();

    const auto cleared = renderScreen();

    for (int y = 0; y < cleared.dimy(); ++y) {
        for (int x = 0; x < cleared.dimx(); ++x) {
            const auto& cell = cleared.CellAt(x, y);
            const auto& previous = original.CellAt(x, y);

            assert(cell.character == previous.character);
            assert(cell.background_color == previous.background_color);
            assert(cell.foreground_color == previous.foreground_color);
        }
    }
}

void test_help_path_selection() {
    auto environment = createRenderTestEnvironment();
    ConsoleUI ui(RenderMode::ascii);

    HelpReport report;

    // Posiciones simuladas para probar únicamente la presentación.
    report.path_to_a = {{0, 7}};
    report.path_to_b = {{0, 9}};
    report.complete_path = {
        {0, 7}, {0, 8}, {0, 9}, {0, 10}
    };

    // Devuelve los símbolos de interruptor que tienen fondo azul.
    auto highlightedSwitches = [&]() -> std::string {
        auto screen = ftxui::Screen::Create(
            ftxui::Dimension::Fixed(120),
            ftxui::Dimension::Fixed(50)
        );

        ftxui::Render(screen, ui.render(environment, {}));

        std::string result;

        for (int y = 0; y < screen.dimy(); ++y) {
            for (int x = 0; x < screen.dimx(); ++x) {
                const auto& cell = screen.CellAt(x, y);

                const bool isSwitch =
                    cell.character == "A" ||
                    cell.character == "a" ||
                    cell.character == "B" ||
                    cell.character == "b";

                if (isSwitch &&
                    cell.background_color == ftxui::Color::Blue) {
                    result += cell.character;
                }
            }
        }

        return result;
    };

    ui.highlightHelpPath(report, HelpPathSelection::to_a);
    assert(highlightedSwitches() == "A");

    ui.highlightHelpPath(report, HelpPathSelection::to_b);
    assert(highlightedSwitches() == "B");

    ui.highlightHelpPath(report, HelpPathSelection::complete);
    assert(highlightedSwitches() == "AaBb");

    ui.highlightHelpPath(report, HelpPathSelection::none);
    assert(highlightedSwitches().empty());

    // Elegir una ruta vacía debe eliminar el resaltado anterior.
    ui.highlightHelpPath(report, HelpPathSelection::to_a);

    report.path_to_b.clear();

    ui.highlightHelpPath(report, HelpPathSelection::to_b);
    assert(highlightedSwitches().empty());
}


void test_help_report_integration(
    RenderMode mode,
    bool showPreview = false
) {
    auto environment = createRenderTestEnvironment();
    ConsoleUI ui(mode);
    // Abrimos la ayuda sin haber recibido un reporte.
    ui.setHelp();

    std::string output = renderToString(ui, environment);

    assert(output.find("AYUDA GENERAL") != std::string::npos);
    assert(
        output.find("La ayuda de energia aun no esta disponible.")
        != std::string::npos
    );

    // Reporte de ejemplo: ambos interruptores están activados.
    HelpReport report;

    report.switch_a = {
        ObjectiveStatus::activated, std::nullopt
    };

    report.switch_b = {
        ObjectiveStatus::activated, std::nullopt
    };

    report.exit_goal = {ObjectiveStatus::enabled, 3};

    report.current_energy = 12;
    report.total_cost = 3;
    report.recommended_order = {HelpTarget::exit};
    report.segment_costs = {3};

    
    report.complete_path = {
        {0, 7}, {0, 8}, {0, 9}, {0, 10}
    };
    
    ui.setHelpReport(report);
    
    ui.highlightHelpPath(
        report,
        HelpPathSelection::complete
    );

    if (showPreview) {
        auto preview = ftxui::Screen::Create(
            ftxui::Dimension::Fixed(120),
            ftxui::Dimension::Fixed(60)
        );

        ftxui::Render(preview, ui.render(environment, {}));

        preview.Print();
    }

    output = renderToString(ui, environment);

    assert(output.find("AYUDA DE ENERGIA") != std::string::npos);
    assert(output.find("MAPA Y RUTA") != std::string::npos);
    assert(output.find("Recorrido: Actual -> Salida") != std::string::npos);
    assert(output.find("Energia actual: 12") != std::string::npos);
    assert(output.find("Saldo estimado: 9") != std::string::npos);

    // Un nuevo reporte debe actualizar la vista.
    report.current_energy = 3;
    ui.setHelpReport(report);

    output = renderToString(ui, environment);

    assert(output.find("Energia actual: 3") != std::string::npos);
    assert(output.find("Saldo estimado: 0") != std::string::npos);
    assert(output.find("no permite ganar") != std::string::npos);
    assert(output.find("Energia actual: 12") == std::string::npos);

    // Sin reporte volvemos a la ayuda general.
    ui.clearHelpReport();

    output = renderToString(ui, environment);

    assert(output.find("AYUDA GENERAL") != std::string::npos);
    assert(output.find("AYUDA DE ENERGIA") == std::string::npos);

    // Cerramos la ayuda y vuelve a mostrarse el tablero.
    ui.setHelp();

    output = renderToString(ui, environment);

    assert(output.find("MAZE") != std::string::npos);
    assert(output.find("AYUDA GENERAL") == std::string::npos);
}

// Punto de entrada para integrarlo con tests/test_main.cpp.
void run_console_render_tests() {
    test_ascii_renders_all_cell_types();
    test_ascii_board_has_20_rows_and_30_cells();

    test_emoji_renders_all_cell_types();
    test_emoji_board_has_20_rows_and_30_cells();
    test_switches_render();
    test_hud_renders_switches_and_guard_status();
    test_guard_rendering();
    test_path_highlighting();
    test_help_path_selection();
    test_help_report_integration(RenderMode::ascii);
    test_help_report_integration(RenderMode::emoji);
}
