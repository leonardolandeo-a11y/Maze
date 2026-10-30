#pragma once

#include "circuit_escape/map_generation/map_validator.h"
#include "circuit_escape/types.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

enum class GameOutcome {
    in_progress,
    victory,
    captured,
    energy_depleted,
    turn_limit
};

struct GameEffects {
    std::vector<Position> batteries;
    std::vector<Position> traps;
    int battery_recharge{3};
    int trap_energy_penalty{2};
    int wait_energy_cost{1};
    std::vector<std::pair<Position, int>> resources;
    std::vector<std::pair<Position, int>> trap_score_penalties;
};

struct GameState {
    Position player{};
    Position guard{};
    int energy{};
    std::size_t turn{};
    bool switch_a_active{};
    bool switch_b_active{};
    std::vector<std::uint64_t> consumed_batteries;
    std::vector<std::uint64_t> collected_resources;
    int score{};
    std::size_t resource_count{};
    GameOutcome outcome{GameOutcome::in_progress};
};

struct GameVerification {
    GameOutcome outcome{GameOutcome::in_progress};
    std::vector<Action> actions;
    std::vector<GameState> history;
};

class GameVerifier {
public:
    GameVerifier(MapValidationInput input, GameEffects effects = {});

    [[nodiscard]] GameState initial_state() const;
    // Reuse the same BFS pursuit and tie-breaking in the interactive game.
    [[nodiscard]] Position guard_next_step(Position guard, Position player) const;
    [[nodiscard]] GameState step(const GameState& state, Action action) const;
    [[nodiscard]] GameVerification replay(const std::vector<Action>& actions) const;
    [[nodiscard]] std::optional<GameVerification> find_winning_game(
        std::size_t maximum_states = 100'000
    ) const;

private:
    MapValidationInput input_;
    GameEffects effects_;
    MapRouteValidation route_validation_;
    std::vector<int> battery_indices_;
    std::vector<bool> trap_cells_;
    std::vector<int> trap_score_values_;
    std::vector<int> resource_indices_;
    std::vector<std::vector<std::size_t>> reversed_neighbors_;

    [[nodiscard]] std::size_t index(Position position) const;
    [[nodiscard]] Position position(std::size_t index) const;
    [[nodiscard]] Position next_guard_position(Position guard, Position player) const;
    [[nodiscard]] std::vector<std::size_t> distances_to(Position goal) const;
    [[nodiscard]] std::size_t minimum_remaining_turns(
        const GameState& state,
        const std::vector<std::size_t>& to_a,
        const std::vector<std::size_t>& to_b,
        const std::vector<std::size_t>& to_exit,
        std::size_t a_to_b,
        std::size_t b_to_a,
        std::size_t a_to_exit,
        std::size_t b_to_exit
    ) const;
};
