#include "circuit_escape/map_generation/map_sizes.h"
#include <algorithm>
#include <stdexcept>

std::string map_size_label(Project2MapSize size) {
    return std::to_string(size.rows) + "x" + std::to_string(size.columns);
}

void validate_project2_map_size(Project2MapSize size) {
    if (std::find(project2_map_sizes.begin(), project2_map_sizes.end(), size) == project2_map_sizes.end())
        throw std::invalid_argument("Unsupported map size. Choose 10x15, 20x30 or 40x60.");
}

