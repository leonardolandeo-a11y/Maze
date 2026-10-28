#include "circuit_escape/map_generation/map_generator_cli.h"

#include "circuit_escape/map_generation/map_batch.h"
#include "circuit_escape/map_generation/map_storage.h"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_set>
#include <vector>

namespace {

template<typename Number>
Number read_number(const std::string& value, std::string_view option) {
    Number result{};
    const auto [end, status] = std::from_chars(value.data(), value.data() + value.size(), result);
    if (status != std::errc{} || end != value.data() + value.size() || value.empty()) {
        throw std::invalid_argument("Invalid value for " + std::string(option) + ": " + value);
    }
    return result;
}

void print_usage(std::ostream& output) {
    output << "Usage: map_generator [options]\n"
           << "  --rows N              Map height (default: 10)\n"
           << "  --columns N           Map width (default: 15)\n"
           << "  --seed N              Base random seed (default: 12345)\n"
           << "  --workers N           Parallel workers (default: 4)\n"
           << "  --candidates N        Candidates per batch, 1 to 10 (default: 10)\n"
           << "  --output DIR          Output directory (default: assets/maps)\n"
           << "  --initial-energy N    Initial energy (default: 100)\n"
           << "  --maximum-energy N    Maximum energy (default: 100)\n"
           << "  --turn-limit N        Maximum turns (default: 150)\n"
           << "  --extra-openings N    Additional maze openings (default: 4)\n"
           << "  --rough-percent N     Rough terrain percentage (default: 15)\n"
           << "  --rough-cost N        Rough terrain entry cost (default: 2)\n"
           << "  --batteries N         Battery count (default: 0)\n"
           << "  --traps N             Trap count (default: 0)\n"
           << "  --battery-recharge N  Energy restored per battery (default: 3)\n"
           << "  --trap-penalty N      Energy penalty per trap (default: 2)\n"
           << "  --search-states N     Verification search budget (default: 20000)\n"
           << "  --help                Show these options\n";
}

struct Options {
    MapBatchConfig config;
    std::filesystem::path output_directory{"assets/maps"};
};

Options parse_options(const std::vector<std::string>& arguments) {
    Options options;
    std::unordered_set<std::string> seen;
    for (std::size_t index = 0; index < arguments.size(); index += 2) {
        const std::string& key = arguments[index];
        if (key == "--help") {
            throw std::invalid_argument("--help must be used by itself");
        }
        if (!seen.insert(key).second) {
            throw std::invalid_argument("Duplicate option: " + key);
        }
        if (index + 1 >= arguments.size() || arguments[index + 1].empty()) {
            throw std::invalid_argument("Missing value for " + key);
        }
        const std::string& value = arguments[index + 1];
        if (key == "--rows") options.config.rows = read_number<std::size_t>(value, key);
        else if (key == "--columns") options.config.columns = read_number<std::size_t>(value, key);
        else if (key == "--seed") options.config.base_seed = read_number<std::uint32_t>(value, key);
        else if (key == "--workers") options.config.worker_count = read_number<std::size_t>(value, key);
        else if (key == "--candidates") options.config.candidate_count = read_number<std::size_t>(value, key);
        else if (key == "--output") options.output_directory = value;
        else if (key == "--initial-energy") options.config.initial_energy = read_number<int>(value, key);
        else if (key == "--maximum-energy") options.config.maximum_energy = read_number<int>(value, key);
        else if (key == "--turn-limit") options.config.turn_limit = read_number<std::size_t>(value, key);
        else if (key == "--extra-openings") options.config.extra_openings = read_number<std::size_t>(value, key);
        else if (key == "--rough-percent") options.config.rough_terrain_percent = read_number<unsigned int>(value, key);
        else if (key == "--rough-cost") options.config.rough_terrain_cost = read_number<int>(value, key);
        else if (key == "--batteries") options.config.battery_count = read_number<std::size_t>(value, key);
        else if (key == "--traps") options.config.trap_count = read_number<std::size_t>(value, key);
        else if (key == "--battery-recharge") options.config.battery_recharge = read_number<int>(value, key);
        else if (key == "--trap-penalty") options.config.trap_energy_penalty = read_number<int>(value, key);
        else if (key == "--search-states") options.config.maximum_search_states = read_number<std::size_t>(value, key);
        else throw std::invalid_argument("Unknown option: " + key);
    }
    return options;
}

int cost_for(const MapBatchResult& batch, std::size_t id) {
    for (const MapBatchEntry& entry : batch.entries) {
        if (entry.id == id && entry.route_validation && entry.route_validation->recommended) {
            return entry.route_validation->recommended->total_energy_cost;
        }
    }
    throw std::logic_error("Selected map cost is missing");
}

}

int run_map_generator(const std::vector<std::string>& arguments,
                      std::ostream& output, std::ostream& error) {
    if (arguments.size() == 1 && arguments.front() == "--help") {
        print_usage(output);
        return 0;
    }
    try {
        const Options options = parse_options(arguments);
        const MapBatchResult batch = run_map_batch(options.config);
        output << "Map generation: " << options.config.rows << 'x' << options.config.columns
               << ", seed " << options.config.base_seed
               << ", candidates " << options.config.candidate_count
               << ", workers " << options.config.worker_count << '\n';
        output << "Construction: " << batch.construction_milliseconds << " ms\n"
               << "Validation: " << batch.validation_milliseconds << " ms\n";
        std::size_t verified_count = 0;
        for (const MapBatchEntry& entry : batch.entries) {
            if (entry.winning_game) {
                ++verified_count;
                output << "Candidate " << entry.id << " (seed " << entry.seed
                       << "): verified, cost " << cost_for(batch, entry.id) << '\n';
            } else {
                output << "Candidate " << entry.id << " (seed " << entry.seed
                       << "): rejected - " << entry.rejection_reason << '\n';
            }
        }
        output << "Verified candidates: " << verified_count << '/' << batch.entries.size() << '\n';
        if (!batch.selection) {
            error << "No three distinct verified difficulty levels in this batch. "
                  << "Try another --seed or configuration.\n";
            return 2;
        }
        save_selected_maps(batch, options.output_directory);
        const MapSelection& levels = *batch.selection;
        output << "Easy: candidate " << levels.easy_id
               << ", cost " << cost_for(batch, levels.easy_id) << '\n'
               << "Medium: candidate " << levels.medium_id
               << ", cost " << cost_for(batch, levels.medium_id) << '\n'
               << "Hard: candidate " << levels.hard_id
               << ", cost " << cost_for(batch, levels.hard_id) << '\n'
               << "Saved to: " << options.output_directory.string() << '\n';
        return 0;
    } catch (const std::exception& exception) {
        error << "Map generator error: " << exception.what() << '\n';
        return 1;
    }
}
