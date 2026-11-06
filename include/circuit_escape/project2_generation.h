#pragma once

#include "circuit_escape/map_generation/map_storage.h"
#include "circuit_escape/map_generation/map_sizes.h"
#include <future>
#include <string>

MapBatchConfig project2_batch_config(Project2MapSize size);
std::filesystem::path generate_project2_maps(Project2MapSize size, const std::filesystem::path& root);

// The future publishes only a completed directory or an exception. Game state
// remains exclusively owned by the UI thread. Destruction joins outstanding work.
class Project2Generation {
public:
    void start(Project2MapSize size, const std::filesystem::path& root);
    bool poll();
    bool loading() const { return future_.valid(); }
    const std::string& error() const { return error_; }
    const std::filesystem::path& directory() const { return directory_; }
private:
    std::future<std::filesystem::path> future_;
    std::filesystem::path directory_;
    std::string error_;
};
