#include "circuit_escape/map_generation/map_selector.h"


std::optional<MapSelection> select_maps(const std::vector<MapCandidate>& candidates){
    if (candidates.empty()){
        return std::nullopt;
    }

    bool has_verified_map = false;
    for (const auto& candidate : candidates){
        if (candidate.verified_win){
            has_verified_map=true;
            break;
        }
    }
    if (!has_verified_map){
        return std::nullopt;
    }
    return std::nullopt;
}