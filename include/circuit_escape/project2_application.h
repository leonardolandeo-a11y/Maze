#pragma once

#include <filesystem>
#include <optional>

#include "circuit_escape/map_generation/map_storage.h"

class Project2Application {
public:
    explicit Project2Application(std::filesystem::path maps_directory);
    int run(std::optional<SavedDifficulty> difficulty = std::nullopt);

private:
    std::filesystem::path maps_directory_;
};
