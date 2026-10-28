#include "circuit_escape/project2_session.h"

#include "circuit_escape/map_generation/map_batch.h"

#include <cassert>
#include <stdexcept>
#include <utility>

void run_project2_session_tests() {
    MapBatchConfig config;
    const MapBatchResult batch = run_map_batch(config);
    const MapBatchEntry* winner = nullptr;
    for (const auto& entry : batch.entries) {
        if (entry.winning_game && entry.winning_game->outcome == GameOutcome::victory) {
            winner = &entry;
            break;
        }
    }
    assert(winner != nullptr);
    Project2Session session(make_saved_map(*winner, SavedDifficulty::easy));
    const GameState initial = session.state();
    assert(initial.turn == 0);
    assert(initial.energy == session.map().input.initial_energy);
    assert(initial.player == session.map().input.player_start);

    const HelpReport initial_help = session.help_report();
    assert(initial_help.current_energy == initial.energy);
    assert(initial_help.switch_a.status == ObjectiveStatus::pending);
    assert(initial_help.switch_b.status == ObjectiveStatus::pending);
    assert(initial_help.exit_goal.status == ObjectiveStatus::locked);
    assert(initial_help.total_cost.has_value());
    assert(!initial_help.complete_path.empty());
    assert(session.state().turn == 0);

    for (Action action : session.map().winning_actions) {
        assert(session.state().outcome == GameOutcome::in_progress);
        const GameState next = session.step(action);
        assert(next.turn > 0);
        if (next.switch_a_active || next.switch_b_active) {
            const HelpReport updated = session.help_report();
            assert(session.state().turn == next.turn);
            if (next.switch_a_active) {
                assert(updated.switch_a.status == ObjectiveStatus::activated);
                assert(!updated.switch_a.cost_from_current);
            }
            if (next.switch_b_active) {
                assert(updated.switch_b.status == ObjectiveStatus::activated);
                assert(!updated.switch_b.cost_from_current);
            }
        }
    }

    assert(session.state().outcome == GameOutcome::victory);
    assert(session.state().switch_a_active);
    assert(session.state().switch_b_active);
    const HelpReport final_help = session.help_report();
    assert(final_help.exit_goal.status == ObjectiveStatus::enabled);
    assert(final_help.total_cost == 0);
    assert(final_help.recommended_order.size() == 1);
    assert(final_help.recommended_order.front() == HelpTarget::exit);

    bool rejected = false;
    try {
        (void)session.step(Action::wait);
    } catch (const std::logic_error&) {
        rejected = true;
    }
    assert(rejected);

    session.reset();
    assert(session.state().turn == 0);
    assert(session.state().outcome == GameOutcome::in_progress);
    assert(!session.state().switch_a_active);
    assert(!session.state().switch_b_active);
    assert(session.help_report().total_cost == initial_help.total_cost);
}
