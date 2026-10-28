#pragma once

#include "circuit_escape/map_generation/map_batch.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>

enum class SavedDifficulty {
    easy,
    medium,
    hard
};

struct SavedMap {
    std::size_t id{};
    std::uint32_t seed{};
    SavedDifficulty difficulty{SavedDifficulty::easy};
    int minimum_energy_cost{};
    Grid<Cell> grid;
    MapValidationInput input;
    GameEffects effects;
    std::vector<Action> winning_actions;

    SavedMap(std::size_t rows, std::size_t columns);
};

SavedMap make_saved_map(const MapBatchEntry& entry, SavedDifficulty difficulty);
void save_map(const SavedMap& map, const std::filesystem::path& path);
SavedMap load_map(const std::filesystem::path& path);
void save_selected_maps(const MapBatchResult& batch, const std::filesystem::path& directory);
