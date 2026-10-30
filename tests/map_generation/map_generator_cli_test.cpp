#include "circuit_escape/map_generation/map_generator_cli.h"
#include "circuit_escape/map_generation/map_storage.h"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

namespace {

void usage_and_validation_test() {
    std::ostringstream output;
    std::ostringstream error;
    assert(run_map_generator({"--help"}, output, error) == 0);
    assert(output.str().find("--seed") != std::string::npos);
    assert(error.str().empty());
    for (const auto& arguments : {
            std::vector<std::string>{"--unknown", "1"},
            {"--rows"},
            {"--rows", "-2"},
            {"--seed", "4294967296"},
            {"--rows", "abc"},
            {"--workers", "0"},
            {"--candidates", "11"},
            {"--rows", "3", "--columns", "3"},
            {"--rows", "10", "--rows", "20"},
            {"--rough-percent", "101"},
            {"--help", "--seed", "4"}}) {
        output.str("");
        output.clear();
        error.str("");
        error.clear();
        assert(run_map_generator(arguments, output, error) == 1);
        assert(!error.str().empty());
    }
}

void missing_three_levels_test(const std::filesystem::path& directory) {
    const auto output_directory = directory / "incomplete";
    std::ostringstream output;
    std::ostringstream error;
    const int result = run_map_generator(
        {"--candidates", "1", "--seed", "2026", "--search-states", "200",
         "--output", output_directory.string()}, output, error);
    assert(result == 2);
    assert(error.str().find("three distinct") != std::string::npos);
    assert(!std::filesystem::exists(output_directory));
}

void generated_files_test(const std::filesystem::path& directory) {
    const auto output_directory = directory / "maps";
    std::ostringstream output;
    std::ostringstream error;
    const int result = run_map_generator(
        {"--rows", "10", "--columns", "15", "--seed", "12345",
         "--workers", "4", "--candidates", "10",
         "--resources", "3", "--resource-points", "12",
         "--batteries", "2", "--battery-recharge", "5",
         "--traps", "2", "--trap-penalty", "3", "--trap-score-penalty", "2",
         "--output", output_directory.string()},
        output, error);
    assert(result == 0);
    assert(error.str().empty());
    assert(output.str().find("Verified candidates:") != std::string::npos);
    const auto easy = load_map(output_directory / "mapa_facil.txt");
    const auto medium = load_map(output_directory / "mapa_medio.txt");
    const auto hard = load_map(output_directory / "mapa_dificil.txt");
    assert(easy.difficulty == SavedDifficulty::easy);
    assert(medium.difficulty == SavedDifficulty::medium);
    assert(hard.difficulty == SavedDifficulty::hard);
    assert(easy.minimum_energy_cost < medium.minimum_energy_cost);
    assert(medium.minimum_energy_cost < hard.minimum_energy_cost);
    assert(easy.grid.rows() == 10 && easy.grid.columns() == 15);
    assert(medium.grid.rows() == 10 && hard.grid.columns() == 15);
    for (const auto* map : {&easy, &medium, &hard}) {
        assert(map->effects.resources.size() == 3);
        assert(map->effects.batteries.size() == 2);
        assert(map->effects.traps.size() == 2);
        assert(map->effects.resources.front().second == 12);
        assert(map->effects.trap_score_penalties.front().second == 2);
        assert(GameVerifier(map->input, map->effects).replay(map->winning_actions).outcome == GameOutcome::victory);
    }
}

}

void run_map_generator_cli_tests() {
    usage_and_validation_test();
    const auto folder = std::filesystem::temp_directory_path() /
        ("circuit_escape_generator_cli_" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(folder);
    try {
        missing_three_levels_test(folder);
        generated_files_test(folder);
    } catch (...) {
        std::filesystem::remove_all(folder);
        throw;
    }
    std::filesystem::remove_all(folder);
}
