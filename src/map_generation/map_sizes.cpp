#include "circuit_escape/map_generation/map_sizes.h"
#include <charconv>
#include <stdexcept>

std::string map_size_label(Project2MapSize size) {
    return std::to_string(size.rows) + "x" + std::to_string(size.columns);
}

void validate_project2_map_size(Project2MapSize size) {
    if (size.rows < 5 || size.columns < 5)
        throw std::invalid_argument("Minimum size: 5 rows × 5 columns.");
    if (size.rows > 40 || size.columns > 60)
        throw std::invalid_argument("Map too large. Maximum: 40 rows × 60 columns. Maximum map area: 2400 cells.");
    if (size.rows * size.columns > 2400)
        throw std::invalid_argument("Maximum map area: 2400 cells.");
    if (size.columns < size.rows || size.columns > size.rows * 2)
        throw std::invalid_argument("Unsupported proportions. Columns must be between rows and 2×rows.");
}

Project2MapSize parse_project2_map_size(std::string_view rows, std::string_view columns) {
    const auto number = [](std::string_view input) {
        std::size_t value = 0;
        const auto [end, error] = std::from_chars(input.data(), input.data() + input.size(), value);
        if (input.empty() || error != std::errc{} || end != input.data() + input.size())
            throw std::invalid_argument("Enter whole numbers for rows and columns.");
        return value;
    };
    Project2MapSize size{number(rows), number(columns)};
    validate_project2_map_size(size);
    return size;
}
