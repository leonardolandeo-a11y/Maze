#pragma once
#include <array>
#include <cstddef>
#include <string>

struct Project2MapSize {
    std::size_t rows;
    std::size_t columns;
    bool operator==(const Project2MapSize&) const = default;
};
inline constexpr std::array<Project2MapSize, 3> project2_map_sizes{{{10,15}, {20,30}, {40,60}}};
std::string map_size_label(Project2MapSize size);
void validate_project2_map_size(Project2MapSize size);
