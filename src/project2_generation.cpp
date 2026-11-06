#include "circuit_escape/project2_generation.h"
#include <algorithm>
#include <chrono>
#include <stdexcept>

MapBatchConfig project2_batch_config(Project2MapSize size) {
    validate_project2_map_size(size);
    MapBatchConfig config;
    config.rows = size.rows;
    config.columns = size.columns;
    config.base_seed = size == project2_map_sizes.back() ? 42 : 12345;
    config.initial_energy = config.maximum_energy = 1000;
    config.turn_limit = 2000;
    config.extra_openings = 15;
    config.maximum_search_states = 100;
    return config;
}

std::filesystem::path generate_project2_maps(Project2MapSize size, const std::filesystem::path& root) {
    const auto config = project2_batch_config(size); // Reject before creating files or workers.
    const auto batch = run_map_batch(config);
    if (!batch.selection)
        throw std::runtime_error("Generation failed: need three verified maps with distinct costs. Try again.");
    const auto directory = root / map_size_label(size);
    save_selected_maps(batch, directory);
    for (const auto* filename : {"mapa_facil.txt", "mapa_medio.txt", "mapa_dificil.txt"}) {
        const auto map = load_map(directory / filename);
        if (map.grid.rows() != size.rows || map.grid.columns() != size.columns)
            throw std::runtime_error("Generated map dimensions do not match the selected size.");
    }
    return directory;
}

void Project2Generation::start(Project2MapSize size, const std::filesystem::path& root) {
    if (loading()) throw std::logic_error("Map generation is already running");
    directory_.clear();
    error_.clear();
    future_ = std::async(std::launch::async, [size, root] { return generate_project2_maps(size, root); });
}

bool Project2Generation::poll() {
    if (!loading() || future_.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return false;
    try { directory_ = future_.get(); }
    catch (const std::exception& exception) { error_ = exception.what(); }
    catch (...) { error_ = "Map generation failed unexpectedly."; }
    return true;
}
