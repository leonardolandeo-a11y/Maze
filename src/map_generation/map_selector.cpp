#include "circuit_escape/map_generation/map_selector.h"


std::optional<MapSelection> select_maps(const std::vector<MapCandidate>& candidates) {
    if (candidates.empty()) {
        return std::nullopt;
    }

    std::vector<MapCandidate> verified_candidates;

    for (const auto& candidate : candidates) {
        if (candidate.verified_win) {
            verified_candidates.push_back(candidate);
        }
    }

    if (verified_candidates.empty()) {
        return std::nullopt;
    }

    // Easy / medium / hard selection will be implemented
    // in the next task.
    return std::nullopt;
}