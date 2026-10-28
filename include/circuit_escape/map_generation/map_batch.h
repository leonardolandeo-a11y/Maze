#pragma once

#include "circuit_escape/map_generation/game_verifier.h"
#include "circuit_escape/map_generation/kruskal.h"
#include "circuit_escape/map_generation/map_selector.h"
#include "circuit_escape/map_generation/map_validator.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct MapBatchConfig {
    std::size_t rows{10};
    std::size_t columns{15};
    std::size_t candidate_count{10};
    std::size_t worker_count{4};
    std::uint32_t base_seed{12345};
    std::size_t extra_openings{4};
    int initial_energy{100};
    int maximum_energy{100};
    std::size_t turn_limit{150};
    unsigned int rough_terrain_percent{15};
    int rough_terrain_cost{2};
    std::size_t battery_count{};
    std::size_t trap_count{};
    int battery_recharge{3};
    int trap_energy_penalty{2};
    std::size_t maximum_search_states{20'000};
};

struct PreparedMap {
    std::size_t id{};
    std::uint32_t seed{};
    MazeGenerationResult maze;
    MapValidationInput input;
    GameEffects effects;
};

struct MapBatchEntry {
    std::size_t id{};
    std::uint32_t seed{};
    std::optional<PreparedMap> map;
    std::optional<MapRouteValidation> route_validation;
    std::optional<GameVerification> winning_game;
    std::string rejection_reason;
};

struct MapBatchResult {
    std::vector<MapBatchEntry> entries;
    std::optional<MapSelection> selection;
    double construction_milliseconds{};
    double validation_milliseconds{};
};

PreparedMap build_map(const MapBatchConfig& config, std::size_t id, std::uint32_t seed);
void validate_map(MapBatchEntry& entry, std::size_t maximum_search_states);
MapBatchResult run_map_batch(const MapBatchConfig& config);
