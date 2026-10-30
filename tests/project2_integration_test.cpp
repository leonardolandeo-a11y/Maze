#include "circuit_escape/console_ui.h"
#include "circuit_escape/project2_help_text.h"
#include "circuit_escape/project2_session.h"
#include "circuit_escape/menus.h"

#include <cassert>
#include <filesystem>
#include <limits>
#include <type_traits>
#include <utility>
#include <string>
#include <vector>

#include <ftxui/screen/screen.hpp>

namespace {

SavedMap make_fixture(std::size_t rows, std::size_t columns) {
    SavedMap map(rows, columns);
    map.id = 1;
    map.seed = 12345;
    map.difficulty = SavedDifficulty::easy;
    map.minimum_energy_cost = 3;
    map.grid.at({1, 4}) = Exit{};
    map.input.graph.rows = rows;
    map.input.graph.columns = columns;
    const std::size_t count = rows * columns;
    map.input.graph.neighbors.resize(count);
    map.input.graph.blocked.resize(count, false);
    map.input.graph.entry_costs.resize(count, 1);
    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t column = 0; column < columns; ++column) {
            auto& neighbors = map.input.graph.neighbors[row * columns + column];
            if (row > 0) neighbors.push_back((row - 1) * columns + column);
            if (column > 0) neighbors.push_back(row * columns + column - 1);
            if (column + 1 < columns) neighbors.push_back(row * columns + column + 1);
            if (row + 1 < rows) neighbors.push_back((row + 1) * columns + column);
        }
    }
    map.input.player_start = {1, 1};
    map.input.guard_start = {rows - 2, columns - 2};
    map.input.switch_a = {1, 2};
    map.input.switch_b = {1, 3};
    map.input.exit = {1, 4};
    map.input.initial_energy = 100;
    map.input.maximum_energy = 100;
    map.input.turn_limit = 150;
    map.winning_actions = {Action::right, Action::right, Action::right};
    return map;
}

std::string render_to_string(const ftxui::Element& element) {
    auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(140), ftxui::Dimension::Fixed(55));
    ftxui::Render(screen, element);
    return screen.ToString();
}

ftxui::Cell rendered_player(const ftxui::Element& element) {
    auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(80), ftxui::Dimension::Fixed(30));
    ftxui::Render(screen, element);
    for (int row = 0; row < screen.dimy(); ++row) {
        for (int column = 0; column < screen.dimx(); ++column) {
            const auto& cell = screen.PixelAt(column, row);
            if (cell.character == "@") return cell;
        }
    }
    assert(false && "Player marker missing");
    return {};
}

}

void project2_items_visual_test() {
    SavedMap map = make_fixture(10, 15);
    map.grid.at({1, 0}) = ResourceCell<int>{10, false};
    map.grid.at({2, 0}) = Battery{5, false};
    map.grid.at({2, 1}) = Trap{2, 1};
    map.effects.resources = {{{1, 0}, 10}};
    map.effects.batteries = {{2, 0}};
    map.effects.traps = {{2, 1}};
    map.effects.trap_score_penalties = {{{2, 1}, 1}};
    map.effects.battery_recharge = 5;
    map.effects.trap_energy_penalty = 2;
    Project2Session session(std::move(map));
    ConsoleUI ui(RenderMode::emoji);
    const std::string original = render_to_string(ui.render_project2(session));
    assert(original.find("💎") != std::string::npos);
    assert(original.find("⚡") != std::string::npos);
    assert(original.find("💥") != std::string::npos);
    assert(original.find("Resources 0/1") != std::string::npos);
    auto state = session.step(Action::left);
    assert(state.score == 10 && state.resource_count == 1);
    state = session.step(Action::down);
    assert(state.energy == 100);
    assert(session.resource_collected({1, 0}));
    assert(session.battery_consumed({2, 0}));
    const std::string consumed = render_to_string(ui.render_project2(session));
    assert(consumed.find("Resources 1/1") != std::string::npos);
    assert(consumed.find("Score 10") != std::string::npos);
    assert(consumed.find("💎") == std::string::npos);
    assert(consumed.find("⚡") == std::string::npos);
    state = session.step(Action::right);
    assert(state.score == 9);
    state = session.step(Action::left);
    state = session.step(Action::right);
    assert(state.score == 8 && state.resource_count == 1);
    session.reset();
    assert(session.state().score == 0 && session.state().resource_count == 0);
    assert(!session.resource_collected({1, 0}));
    assert(!session.battery_consumed({2, 0}));
}

void project2_viewport_test() {
    ConsoleUI ui(RenderMode::emoji);
    for (const auto& [rows, columns] : {
            std::pair<std::size_t, std::size_t>{10,15},
            {20,30}, {40,60}}) {
        Project2Session session(make_fixture(rows, columns));
        auto board = ui.render_project2_board(session, {}, Project2MapView::detail);
        board->ComputeRequirement();
        assert(board->requirement().min_x == 60);
        assert(board->requirement().min_y == 20);
    }

    SavedMap distant = make_fixture(40, 60);
    distant.input.player_start = {36, 56};
    distant.input.guard_start = {1, 1};
    distant.grid.at({36, 55}) = ResourceCell<int>{10, false};
    distant.grid.at({1, 10}) = Battery{5, false};
    distant.effects.resources = {{{36, 55}, 10}};
    distant.effects.batteries = {{1, 10}};
    Project2Session session(std::move(distant));
    const auto camera_view = render_to_string(
        ui.render_project2_board(session, {}, Project2MapView::detail));
    assert(camera_view.find("🤖") != std::string::npos);
    assert(camera_view.find("💎") != std::string::npos);
    assert(camera_view.find("⚡") == std::string::npos);
}

void project2_full_map_test() {
    SavedMap map = make_fixture(40, 60);
    map.input.player_start = {36, 56};
    map.input.guard_start = {1, 1};
    map.input.switch_a = {35, 52};
    map.input.switch_b = {2, 53};
    map.input.exit = {1, 58};
    map.grid.at({1, 4}) = Empty{};
    map.grid.at({1, 58}) = Exit{};
    Project2Session session(std::move(map));
    ConsoleUI ui(RenderMode::emoji);
    auto full = ui.render_project2_board(session);
    auto detail = ui.render_project2_board(session, {}, Project2MapView::detail);
    full->ComputeRequirement();
    detail->ComputeRequirement();
    assert(full->requirement().min_x == 60);
    assert(full->requirement().min_y == 20);
    assert(detail->requirement().min_x == 60);
    assert(detail->requirement().min_y == 20);
    const auto full_text = render_to_string(full);
    const auto detail_text = render_to_string(detail);
    assert(full_text.find("@") != std::string::npos);
    assert(full_text.find("00   05") == std::string::npos);
    assert(full_text.find("V") != std::string::npos);
    assert(full_text.find("A") != std::string::npos);
    assert(full_text.find("B") != std::string::npos);
    assert(full_text.find("S") != std::string::npos);
    assert(detail_text.find("👀") == std::string::npos);
    const auto before = session.state();
    ui.render_project2(session, Project2MapView::detail);
    ui.render_project2(session, Project2MapView::overview);
    assert(session.state().turn == before.turn);
    assert(session.state().energy == before.energy);
}

void project2_compact_player_row_test() {
    SavedMap map = make_fixture(40, 60);
    map.input.player_start = {22, 25};
    map.input.guard_start = {1, 1};
    Project2Session session(std::move(map));
    ConsoleUI ui(RenderMode::emoji);
    const ftxui::Dimensions compact_terminal{
        90,
        35
    };

    const auto render_compact_screen = [&]() {
        return render_to_string(
            ui.render_project2(
                session,
                Project2MapView::overview,
                {},
                {},
                compact_terminal
            )
        );
    };
    const auto upper = render_to_string(ui.render_project2_board(session));
    assert(upper.find("@") != std::string::npos);
    const auto upper_player = rendered_player(ui.render_project2_board(session));
    assert(upper_player.foreground_color == ftxui::Color::RGB(255, 255, 255));
    assert(upper_player.background_color == ftxui::Color::RGB(15, 110, 157));
    assert(
        render_compact_screen().find("Player (22,25)")
        != std::string::npos
    );

    assert(
        render_compact_screen().find("S = goal")
        != std::string::npos
    );


    const auto after_first = session.step(Action::down);
    assert(after_first.player == (Position{23, 25}));
    const auto lower = render_to_string(ui.render_project2_board(session));
    assert(lower.find("@") != std::string::npos);
    const auto lower_player = rendered_player(ui.render_project2_board(session));
    assert(lower_player.foreground_color == ftxui::Color::RGB(29, 215, 252));
    assert(lower_player.background_color == ftxui::Color::RGB(23, 35, 54));
    assert(upper_player.background_color != lower_player.background_color);
    assert(
        render_compact_screen().find("Player (23,25)")
        != std::string::npos
    );

    const auto after_second = session.step(Action::down);
    assert(after_second.player == (Position{24, 25}));
    const auto next_upper = render_to_string(ui.render_project2_board(session));
    assert(next_upper.find("@") != std::string::npos);
    const auto next_player = rendered_player(ui.render_project2_board(session));
    assert(next_player.background_color == upper_player.background_color);
    assert(
        render_compact_screen().find("Player (24,25)")
        != std::string::npos
    );
}


void project2_overview_palette_test() {
    SavedMap map = make_fixture(40, 60);
    map.grid.at({4, 10}) = ResourceCell<int>{10, false};
    map.grid.at({6, 11}) = Battery{5, false};
    map.grid.at({8, 12}) = Trap{2, 1};
    map.grid.at({1, 4}) = Empty{};
    map.grid.at({10, 13}) = Exit{};
    map.input.exit = {10, 13};
    Project2Session session(std::move(map));
    ConsoleUI ui(RenderMode::emoji);
    auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(80), ftxui::Dimension::Fixed(28));
    ftxui::Render(screen, ui.render_project2_board(session));
    assert(screen.PixelAt(10, 2).foreground_color == ftxui::Color::RGB(14, 119, 145));
    assert(screen.PixelAt(11, 3).foreground_color == ftxui::Color::RGB(167, 118, 11));
    assert(screen.PixelAt(12, 4).foreground_color == ftxui::Color::RGB(170, 41, 57));
    assert(screen.PixelAt(13, 5).foreground_color == ftxui::Color::RGB(25, 120, 69));
}
void project2_stable_layout_test() {
    Project2Session session(make_fixture(40, 60));
    ConsoleUI ui(RenderMode::emoji);

    // Fixed terminal dimensions make this test deterministic.
    const ftxui::Dimensions terminal_size{
        80,
        40
    };

    auto overview = ui.render_project2(
        session,
        Project2MapView::overview,
        std::span<const Position>{},
        std::span<const std::string>{},
        terminal_size
    );

    auto detail = ui.render_project2(
        session,
        Project2MapView::detail,
        std::span<const Position>{},
        std::span<const std::string>{},
        terminal_size
    );

    const std::vector<std::string> help_lines{
        "A 18 | B 48 | Exit locked",
        "Route: CURRENT -> A -> B -> EXIT",
        "Cost 103 | Remaining 297 | ENOUGH",
        "Estimate ignores guard movements, batteries and waiting",
        "Highlight: 1=A 2=B 3=route 0=off | H/Esc back | Z zoom"
    };

    auto help_overview = ui.render_project2(
        session,
        Project2MapView::overview,
        std::span<const Position>{},
        help_lines,
        terminal_size
    );

    auto help_detail = ui.render_project2(
        session,
        Project2MapView::detail,
        std::span<const Position>{},
        help_lines,
        terminal_size
    );

    overview->ComputeRequirement();
    detail->ComputeRequirement();
    help_overview->ComputeRequirement();
    help_detail->ComputeRequirement();

    const int stable_width =
        overview->requirement().min_x;

    const int stable_height =
        overview->requirement().min_y;

    // Everything must fit inside the simulated terminal.
    assert(stable_width <= terminal_size.dimx);
    assert(stable_height <= terminal_size.dimy);

    // Changing FULL MAP / DETAIL must not resize the UI.
    assert(
        detail->requirement().min_x ==
        stable_width
    );

    assert(
        detail->requirement().min_y ==
        stable_height
    );

    // Opening help must not resize the layout either.
    assert(
        help_overview->requirement().min_x ==
        stable_width
    );

    assert(
        help_overview->requirement().min_y ==
        stable_height
    );

    assert(
        help_detail->requirement().min_x ==
        stable_width
    );

    assert(
        help_detail->requirement().min_y ==
        stable_height
    );

    const auto help_render =
        render_to_string(help_overview);

    assert(
        help_render.find("DIJKSTRA ENERGY HELP")
        != std::string::npos
    );

    assert(
        help_render.find("Route: CURRENT")
        != std::string::npos
    );

    assert(
        help_render.find(" MAZE ")
        != std::string::npos
    );

    assert(
        help_render.find(" STATUS ")
        != std::string::npos
    );

    // Rendering must not modify the game.
    const auto state = session.state();

    assert(
        state.turn == 0 &&
        state.energy == 100
    );
}

void project2_small_map_zoom_test() {
    ConsoleUI ui(RenderMode::emoji);
    for (const auto& [rows, columns] : {
            std::pair<std::size_t, std::size_t>{10, 15}, {20, 30}}) {
        SavedMap map = make_fixture(rows, columns);
        map.grid.at({0, 0}) = ResourceCell<int>{10, false};
        Project2Session session(std::move(map));
        auto overview = ui.render_project2_board(session, {},
            Project2MapView::overview, 70, 22);
        auto detail = ui.render_project2_board(session, {},
            Project2MapView::detail, 70, 22);
        overview->ComputeRequirement();
        detail->ComputeRequirement();
        assert(overview->requirement().min_x == 60);
        assert(overview->requirement().min_y == 20);
        assert(detail->requirement().min_x == 66);
        assert(detail->requirement().min_y == 21);
        // The full map retains the original item symbol rather than a color-only dot.
        assert(render_to_string(overview).find("💎") != std::string::npos);
        assert(render_to_string(detail).find("🤖") != std::string::npos);

        const auto before = session.state();
        auto full_screen = ui.render_project2(session, Project2MapView::overview,
            {}, {}, ftxui::Dimensions{90, 35});
        auto detail_screen = ui.render_project2(session, Project2MapView::detail,
            {}, {}, ftxui::Dimensions{90, 35});
        full_screen->ComputeRequirement();
        detail_screen->ComputeRequirement();
        assert(full_screen->requirement().min_x == detail_screen->requirement().min_x);
        assert(full_screen->requirement().min_y == detail_screen->requirement().min_y);
        assert(session.state().turn == before.turn);
        assert(session.state().energy == before.energy);
    }

    // The previously working 40x60 overview and detail are unchanged.
    Project2Session large(make_fixture(40, 60));
    auto overview = ui.render_project2_board(large, {}, Project2MapView::overview);
    auto detail = ui.render_project2_board(large, {}, Project2MapView::detail);
    overview->ComputeRequirement();
    detail->ComputeRequirement();
    assert(overview->requirement().min_x == 60);
    assert(overview->requirement().min_y == 20);
    assert(detail->requirement().min_x == 60);
    assert(detail->requirement().min_y == 20);
}
void project2_responsive_layout_test() {
    ConsoleUI ui(RenderMode::emoji);

    const ftxui::Dimensions terminal_size{
        160,
        65
    };

    for (const auto& [rows, columns] : {
            std::pair<std::size_t, std::size_t>{10, 15},
            {20, 30},
            {30, 40},
            {40, 30},
            {20, 70},
            {70, 20},
            {40, 40},
            {50, 50},
            {40, 60}
        }) {

        Project2Session session(
            make_fixture(rows, columns)
        );

        const GameState before =
            session.state();

        auto overview = ui.render_project2(
            session,
            Project2MapView::overview,
            {},
            {},
            terminal_size
        );

        auto detail = ui.render_project2(
            session,
            Project2MapView::detail,
            {},
            {},
            terminal_size
        );

        overview->ComputeRequirement();
        detail->ComputeRequirement();

        // The complete UI must fit inside the simulated terminal.
        assert(
            overview->requirement().min_x <=
            terminal_size.dimx
        );

        assert(
            detail->requirement().min_x <=
            terminal_size.dimx
        );

        assert(
            overview->requirement().min_y <=
            terminal_size.dimy
        );

        assert(
            detail->requirement().min_y <=
            terminal_size.dimy
        );

        const std::string overview_text =
            render_to_string(overview);

        const std::string detail_text =
            render_to_string(detail);

        // FULL MAP may use either:
        //
        // 🤖 -> normal tile renderer
        // @  -> compact/scaled renderer
        //
        // Both are valid depending on the available terminal space.
        assert(
            overview_text.find("🤖") !=
                std::string::npos ||
            overview_text.find("@") !=
                std::string::npos
        );

        // DETAIL must always use the real zoomed tile renderer.
        assert(
            detail_text.find("🤖") !=
            std::string::npos
        );

        // Rendering must NEVER modify the game state.
        assert(
            session.state().turn ==
            before.turn
        );

        assert(
            session.state().energy ==
            before.energy
        );

        assert(
            session.state().player ==
            before.player
        );

        assert(
            session.state().guard ==
            before.guard
        );

        assert(
            session.state().switch_a_active ==
            before.switch_a_active
        );

        assert(
            session.state().switch_b_active ==
            before.switch_b_active
        );
    }

    //
    // Also verify the board renderer directly.
    //
    // This simulates the actual map area inside a reasonably large
    // terminal and confirms that all three geometries work:
    //
    // landscape: rows < columns
    // portrait:  rows > columns
    // square:    rows == columns
    //

    for (const auto& [rows, columns] : {
            std::pair<std::size_t, std::size_t>{20, 70},
            {70, 20},
            {40, 40}
        }) {

        Project2Session session(
            make_fixture(rows, columns)
        );

        const GameState before =
            session.state();

        constexpr int board_width = 146;
        constexpr int board_height = 44;

        auto overview_board =
            ui.render_project2_board(
                session,
                {},
                Project2MapView::overview,
                board_width,
                board_height
            );

        auto detail_board =
            ui.render_project2_board(
                session,
                {},
                Project2MapView::detail,
                board_width,
                board_height
            );

        overview_board->ComputeRequirement();
        detail_board->ComputeRequirement();

        assert(
            overview_board->requirement().min_x <=
            board_width
        );

        assert(
            overview_board->requirement().min_y <=
            board_height
        );

        assert(
            detail_board->requirement().min_x <=
            board_width
        );

        assert(
            detail_board->requirement().min_y <=
            board_height
        );

        const std::string overview_text =
            render_to_string(overview_board);

        const std::string detail_text =
            render_to_string(detail_board);

        // Player must never disappear from FULL MAP.
        assert(
            overview_text.find("🤖") !=
                std::string::npos ||
            overview_text.find("@") !=
                std::string::npos
        );

        // DETAIL always has the real player tile.
        assert(
            detail_text.find("🤖") !=
            std::string::npos
        );

        // Rendering cannot consume a turn or energy.
        assert(
            session.state().turn ==
            before.turn
        );

        assert(
            session.state().energy ==
            before.energy
        );

        assert(
            session.state().player ==
            before.player
        );

        assert(
            session.state().guard ==
            before.guard
        );
    }
}


// The interactive session must reproduce the verifier's game rules while
// processing movement and cell interactions through NavigationEnvironment.
void project2_navigation_engine_parity_test() {
    const auto compare = [](SavedMap map, const std::vector<Action>& actions,
                            GameOutcome expected) {
        GameVerifier verifier(map.input, map.effects);
        Project2Session session(std::move(map));
        GameState simulated = verifier.initial_state();
        const auto verify_state = [](const GameState& actual, const GameState& reference) {
            assert(actual.player == reference.player);
            assert(actual.guard == reference.guard);
            assert(actual.energy == reference.energy);
            assert(actual.turn == reference.turn);
            assert(actual.score == reference.score);
            assert(actual.resource_count == reference.resource_count);
            assert(actual.switch_a_active == reference.switch_a_active);
            assert(actual.switch_b_active == reference.switch_b_active);
            assert(actual.consumed_batteries == reference.consumed_batteries);
            assert(actual.collected_resources == reference.collected_resources);
            assert(actual.outcome == reference.outcome);
        };
        verify_state(session.state(), simulated);
        for (Action action : actions) {
            if (simulated.outcome != GameOutcome::in_progress) break;
            simulated = verifier.step(simulated, action);
            const GameState played = session.step(action);
            verify_state(played, simulated);
        }
        assert(simulated.outcome == expected);
        // Reset must restore both the environment's mutable cell flags and the game state.
        session.reset();
        verify_state(session.state(), verifier.initial_state());
        for (Action action : actions) {
            if (session.state().outcome != GameOutcome::in_progress) break;
            (void)session.step(action);
        }
        verify_state(session.state(), simulated);
        if (simulated.outcome != GameOutcome::in_progress) {
            bool rejected = false;
            try { (void)session.step(Action::wait); }
            catch (const std::logic_error&) { rejected = true; }
            assert(rejected);
        }
    };

    // Also exercise the exact map serialization path used by the installer.
    const auto saved_path = std::filesystem::temp_directory_path() /
        "maze_project2_navigation_audit_map.txt";
    const SavedMap serialized = make_fixture(20, 30);
    save_map(serialized, saved_path);
    const SavedMap loaded = load_map(saved_path);
    std::filesystem::remove(saved_path);
    compare(loaded, loaded.winning_actions, GameOutcome::victory);

    for (const auto& [rows, columns] : {
            std::pair<std::size_t, std::size_t>{10, 15}, {20, 30}, {40, 60}}) {
        auto ordinary = make_fixture(rows, columns);
        compare(ordinary, {Action::right, Action::right, Action::right}, GameOutcome::victory);

        auto reversed = make_fixture(rows, columns);
        compare(reversed, {Action::down, Action::right, Action::right,
            Action::up, Action::left, Action::right, Action::right}, GameOutcome::victory);

        auto items = make_fixture(rows, columns);
        items.grid.at({1, 0}) = ResourceCell<int>{10, false};
        items.grid.at({2, 0}) = Battery{5, false};
        items.grid.at({2, 1}) = Trap{2, 2};
        items.grid.at({1, 5}) = RoughTerrain{4};
        items.input.graph.entry_costs[columns + 5] = 4;
        items.effects.resources = {{{1, 0}, 10}};
        items.effects.batteries = {{2, 0}};
        items.effects.traps = {{2, 1}};
        items.effects.trap_score_penalties = {{{2, 1}, 2}};
        items.effects.battery_recharge = 5;
        items.effects.trap_energy_penalty = 2;
        compare(items, {Action::left, Action::down, Action::right, Action::left,
            Action::right, Action::up, Action::right, Action::right, Action::right},
            GameOutcome::victory);

        auto large_values = make_fixture(rows, columns);
        large_values.input.initial_energy = std::numeric_limits<int>::max();
        large_values.input.maximum_energy = std::numeric_limits<int>::max();
        large_values.grid.at({1, 0}) = ResourceCell<int>{std::numeric_limits<int>::max(), false};
        large_values.grid.at({2, 0}) = Battery{std::numeric_limits<int>::max(), false};
        large_values.grid.at({2, 1}) = Trap{2, 2};
        large_values.grid.at({1, 5}) = RoughTerrain{4};
        large_values.input.graph.entry_costs[1 * columns + 5] = 4;
        large_values.effects.resources = {{{1, 0}, std::numeric_limits<int>::max()}};
        large_values.effects.batteries = {{2, 0}};
        large_values.effects.traps = {{2, 1}};
        large_values.effects.trap_score_penalties = {{{2, 1}, 2}};
        large_values.effects.battery_recharge = std::numeric_limits<int>::max();
        large_values.effects.trap_energy_penalty = 2;
        compare(large_values, {Action::left, Action::down, Action::right, Action::left,
            Action::right, Action::up, Action::right, Action::right, Action::right},
            GameOutcome::victory);

        auto depleted = make_fixture(rows, columns);
        depleted.input.initial_energy = 1;
        depleted.input.maximum_energy = 1;
        compare(depleted, {Action::wait}, GameOutcome::energy_depleted);

        auto captured = make_fixture(rows, columns);
        captured.input.guard_start = {2, 1};
        compare(captured, {Action::down}, GameOutcome::captured);

        auto limited = make_fixture(rows, columns);
        limited.input.turn_limit = 2;
        compare(limited, {Action::right, Action::right}, GameOutcome::turn_limit);

        auto blocked = make_fixture(rows, columns);
        blocked.grid.at({2, 1}) = Wall{};
        blocked.input.graph.blocked[2 * columns + 1] = true;
        compare(blocked, {Action::up, Action::down, Action::wait,
            Action::right, Action::right, Action::right}, GameOutcome::victory);
    }
}

// The H report must recalculate Dijkstra costs as objectives change, without
// executing a turn or modifying the player's energy.
void project2_dijkstra_help_progression_test() {
    for (const auto& [rows, columns] : {
            std::pair<std::size_t, std::size_t>{10, 15},
            {20, 30},{30, 40}, {40, 60}
        }) {
        Project2Session session(make_fixture(rows, columns));
        const auto unchanged_by_help = [&](const HelpReport& report) {
            const GameState before = session.state();
            assert(report.current_energy == before.energy);
            assert(session.state().turn == before.turn);
            assert(session.state().energy == before.energy);
            assert(session.state().player == before.player);
            assert(session.state().guard == before.guard);
        };

        const HelpReport initial = session.help_report();
        unchanged_by_help(initial);
        assert(initial.switch_a.status == ObjectiveStatus::pending);
        assert(initial.switch_b.status == ObjectiveStatus::pending);
        assert(initial.exit_goal.status == ObjectiveStatus::locked);
        assert(initial.switch_a.cost_from_current == 1);
        assert(initial.switch_b.cost_from_current == 2);
        assert(!initial.exit_goal.cost_from_current.has_value());
        assert(initial.total_cost == 3);
        assert(initial.recommended_order == (std::vector<HelpTarget>{
            HelpTarget::switch_a, HelpTarget::switch_b, HelpTarget::exit}));
        assert(initial.segment_costs == (std::vector<int>{1, 1, 1}));
        assert(initial.complete_path == (std::vector<Position>{
            {1, 1}, {1, 2}, {1, 3}, {1, 4}}));

        const GameState at_a = session.step(Action::right);
        assert(at_a.switch_a_active && !at_a.switch_b_active);
        const HelpReport after_a = session.help_report();
        unchanged_by_help(after_a);
        assert(after_a.switch_a.status == ObjectiveStatus::activated);
        assert(!after_a.switch_a.cost_from_current.has_value());
        assert(after_a.switch_b.cost_from_current == 1);
        assert(after_a.exit_goal.status == ObjectiveStatus::locked);
        assert(after_a.total_cost == 2);
        assert(after_a.recommended_order == (std::vector<HelpTarget>{
            HelpTarget::switch_b, HelpTarget::exit}));
        assert(after_a.segment_costs == (std::vector<int>{1, 1}));
        assert(after_a.complete_path == (std::vector<Position>{
            {1, 2}, {1, 3}, {1, 4}}));

        const GameState at_b = session.step(Action::right);
        assert(at_b.switch_a_active && at_b.switch_b_active);
        const HelpReport after_both = session.help_report();
        unchanged_by_help(after_both);
        assert(after_both.switch_a.status == ObjectiveStatus::activated);
        assert(after_both.switch_b.status == ObjectiveStatus::activated);
        assert(after_both.exit_goal.status == ObjectiveStatus::enabled);
        assert(after_both.exit_goal.cost_from_current == 1);
        assert(after_both.total_cost == 1);
        assert(after_both.recommended_order ==
            (std::vector<HelpTarget>{HelpTarget::exit}));
        assert(after_both.segment_costs == (std::vector<int>{1}));
        assert(after_both.complete_path == (std::vector<Position>{{1, 3}, {1, 4}}));

        session.reset();
        const HelpReport after_reset = session.help_report();
        unchanged_by_help(after_reset);
        assert(after_reset.total_cost == initial.total_cost);
        assert(after_reset.recommended_order == initial.recommended_order);

        // Reverse the objective positions: the optimal order must now be B, A, exit.
        SavedMap reversed = make_fixture(rows, columns);
        reversed.input.switch_a = {1, 3};
        reversed.input.switch_b = {1, 2};
        Project2Session reversed_session(std::move(reversed));
        const HelpReport reverse_help = reversed_session.help_report();
        assert(reverse_help.total_cost == 3);
        assert(reverse_help.recommended_order == (std::vector<HelpTarget>{
            HelpTarget::switch_b, HelpTarget::switch_a, HelpTarget::exit}));
        assert(reverse_help.segment_costs == (std::vector<int>{1, 1, 1}));
        assert(reversed_session.state().turn == 0);
        assert(reversed_session.state().energy == 100);
        const GameState at_first_b = reversed_session.step(Action::right);
        assert(at_first_b.switch_b_active && !at_first_b.switch_a_active);
        const HelpReport reverse_after_b = reversed_session.help_report();
        assert(reverse_after_b.recommended_order == (std::vector<HelpTarget>{
            HelpTarget::switch_a, HelpTarget::exit}));
        assert(reverse_after_b.total_cost == 2);
        assert(reversed_session.state().turn == at_first_b.turn);
    }
}

void project2_observation_contract_test() {
    static_assert(std::is_same_v<
        decltype(std::declval<const ReadOnlyMapView&>().at(Position{})),
        const Cell&>);
    static_assert(!std::is_assignable_v<
        decltype(std::declval<const ReadOnlyMapView&>().at(Position{})), Cell>);

    for (const auto& [rows, columns] : {
            std::pair<std::size_t, std::size_t>{10, 15},
            {20, 30}, {40, 60}}) {
        Project2Session session(make_fixture(rows, columns));
        const Position initial_guard = session.state().guard;
        const Observation initial = session.observation();
        assert(initial.map.rows == rows && initial.map.columns == columns);
        assert(initial.map.cells.size() == rows * columns);
        assert(initial.map.contains({1, 4}));
        assert(std::holds_alternative<Exit>(initial.map.at({1, 4})));
        assert(!initial.map.contains({rows, columns}));
        bool rejected = false;
        try { (void)initial.map.at({rows, columns}); }
        catch (const std::out_of_range&) { rejected = true; }
        assert(rejected);
        assert(initial.agent == (Position{1, 1}));
        assert(initial.guardPosition == initial_guard);
        assert(!initial.switchAActive && !initial.switchBActive);
        assert(initial.turn == 0);
        assert(!initial.availableActions.empty());

        (void)session.step(Action::right);
        const Observation after_a = session.observation();
        assert(after_a.agent == (Position{1, 2}));
        assert(after_a.switchAActive && !after_a.switchBActive);
        assert(after_a.guardPosition == initial_guard);
        assert(after_a.turn == 1);

        (void)session.step(Action::right);
        const Observation after_b = session.observation();
        assert(after_b.switchAActive && after_b.switchBActive);
        assert(after_b.guardPosition == session.state().guard);
        assert(after_b.guardPosition != initial_guard);
        assert(after_b.turn == 2);

        session.reset();
        const Observation reset = session.observation();
        assert(!reset.switchAActive && !reset.switchBActive);
        assert(reset.guardPosition == initial_guard);
        assert(reset.agent == (Position{1, 1}));
        assert(reset.turn == 0);
        assert(reset.map.rows == rows && reset.map.columns == columns);
        assert(std::holds_alternative<Exit>(reset.map.at({1, 4})));
    }

    // Original fixed-size Grid also provides the read-only map view.
    Grid<Cell, 2, 3> original_grid;
    original_grid.at({1, 2}) = Exit{};
    NavigationEnvironment<2, 3> original(
        original_grid, Agent({0, 0}, 10, 10), GameRules{
            10, 10, 10, 1, 2, 1, 0, 3, 2, 0});
    const Observation old_style = original.state();
    assert(old_style.map.rows == 2 && old_style.map.columns == 3);
    assert(std::holds_alternative<Exit>(old_style.map.at({1, 2})));
    assert(!old_style.switchAActive && !old_style.switchBActive);
    assert(!old_style.guardPosition.has_value());
}

void project2_active_help_text_test() {
    HelpReport report;
    report.switch_a = {ObjectiveStatus::pending, 2};
    report.switch_b = {ObjectiveStatus::pending, 3};
    report.exit_goal = {ObjectiveStatus::locked, std::nullopt};
    report.total_cost = 10;
    report.recommended_order = {HelpTarget::switch_a, HelpTarget::switch_b, HelpTarget::exit};

    report.current_energy = 11;
    const auto positive = format_project2_help_lines(report);
    assert(positive.size() == 5);
    assert(positive[0].find("A: 2 energy") != std::string::npos);
    assert(positive[0].find("B: 3 energy") != std::string::npos);
    assert(positive[0].find("Exit: locked") != std::string::npos);
    assert(positive[1] == "Route: CURRENT -> A -> B -> EXIT");
    assert(positive[2] == "Cost 10 | Remaining 1 | ENOUGH");
    assert(positive[4].find("H/Esc back | Z zoom") != std::string::npos);

    report.current_energy = 10;
    const auto zero = format_project2_help_lines(report);
    assert(zero[2] == "Cost 10 | Remaining 0 | ZERO AT EXIT - RECHARGE");

    report.current_energy = 9;
    const auto negative = format_project2_help_lines(report);
    assert(negative[2] == "Cost 10 | Remaining -1 | INSUFFICIENT - RECHARGE");

    report.total_cost.reset();
    report.exit_goal = {ObjectiveStatus::enabled, std::nullopt};
    const auto unreachable = format_project2_help_lines(report);
    assert(unreachable[1] == "Route: unavailable");
    assert(unreachable[2] == "Cost: unreachable");
    assert(unreachable[0].find("Exit: unreachable") != std::string::npos);
}

void run_project2_integration_tests() {
    project2_active_help_text_test();
    project2_observation_contract_test();
    project2_dijkstra_help_progression_test();
    project2_navigation_engine_parity_test();
    project2_items_visual_test();
    project2_viewport_test();
    project2_full_map_test();
    project2_compact_player_row_test();
    project2_overview_palette_test();
    project2_stable_layout_test();
    project2_small_map_zoom_test();
    project2_responsive_layout_test();
    assert(Menus(true).selectedDifficulty() == Difficulty::easy);
    assert(Menus(false).selectedDifficulty() == Difficulty::standard);
    for (const auto& [rows, columns] : {
            std::pair<std::size_t, std::size_t>{10,15}, {20,30}, {40,60}}) {
        Project2Session session(make_fixture(rows, columns));
        ConsoleUI emoji(RenderMode::emoji);
        ConsoleUI ascii(RenderMode::ascii);
        const auto screen_emoji = render_to_string(emoji.render_project2(session));
        const auto screen_ascii = render_to_string(ascii.render_project2(session));
        assert(screen_emoji.find(" MAZE ") != std::string::npos);
        assert(screen_emoji.find(" STATUS ") != std::string::npos);
        assert(screen_emoji.find("Score 0") != std::string::npos);
        assert(screen_emoji.find("Resources 0/0") != std::string::npos);
        assert(
            screen_emoji.find("🤖") != std::string::npos ||
            screen_emoji.find("@") != std::string::npos
        );
        assert(screen_emoji.find("0️⃣") == std::string::npos);
        assert(screen_emoji.find("Next move: turn 2") != std::string::npos);
        assert(screen_ascii.find("@") != std::string::npos);
        assert(screen_ascii.find("A") != std::string::npos);
        assert(screen_ascii.find("B") != std::string::npos);
        if (rows <= 20 && columns <= 30) {
            // The active Project 2 legend must match the selected render mode.
            assert(screen_ascii.find("V guard | @ player | S exit") != std::string::npos);
            assert(screen_ascii.find("Resources: R | Batteries: + | Traps: T | Walls: #")
                != std::string::npos);
            assert(screen_ascii.find("Resources: diamonds") == std::string::npos);
            assert(screen_emoji.find("Resources: diamonds") != std::string::npos);
        }
        const auto before = session.state();
        const auto help = session.help_report();
        assert(help.total_cost.has_value());
        assert(session.state().turn == before.turn);
        assert(session.state().energy == before.energy);
        assert(session.state().player == before.player);
        const auto route_screen = render_to_string(emoji.render_project2_board(session, help.complete_path));
        assert(!route_screen.empty());
        assert(session.state().turn == before.turn);
        assert(session.state().energy == before.energy);
    }
}
