#pragma once

#include "circuit_escape/map_generation/game_verifier.h"
#include "circuit_escape/map_generation/map_storage.h"
#include "circuit_escape/project2_contracts.h"

#include <vector>

class Project2Session {
public:
    explicit Project2Session(SavedMap map);

    [[nodiscard]] const SavedMap& map() const noexcept;
    [[nodiscard]] const GameState& state() const noexcept;
    [[nodiscard]] GameState step(Action action);
    void reset();
    [[nodiscard]] HelpReport help_report() const;
    [[nodiscard]] bool battery_consumed(Position position) const;
    [[nodiscard]] bool resource_collected(Position position) const;

private:
    SavedMap map_;
    GameVerifier verifier_;
    GameState state_;
};
