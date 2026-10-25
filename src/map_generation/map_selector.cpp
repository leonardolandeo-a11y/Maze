#include "circuit_escape/map_generation/map_selector.h"


std::optional<MapSelection> select_maps(const std::vector<MapCandidate>& candidates) {
    if (candidates.empty()) {
        return std::nullopt;
    }

    std::set<std::size_t> seen_ids;
    std::vector<MapCandidate> verified_candidates;

    for (const auto& candidate : candidates) {

        // Every map must have a unique identifier.
        if (!seen_ids.insert(candidate.id).second) {
            return std::nullopt;
        }

        if (!candidate.verified_win) {
            continue;
        }

        if (candidate.minimum_energy_cost < 0) {
            return std::nullopt;
        }

        verified_candidates.push_back(candidate);
    }

    if (verified_candidates.empty()) {
        return std::nullopt;
    }

    std::set<int> distinct_costs;

    for (const auto& candidate: verified_candidates){
        distinct_costs.insert(candidate.minimum_energy_cost);
    }

    if (distinct_costs.size()< 3){
        return std::nullopt;
    }

    const MapCandidate* easy =nullptr;
    const MapCandidate* hard = nullptr;

    for (const auto& candidate: verified_candidates){

        // Easy
        if (easy== nullptr||candidate.minimum_energy_cost<easy->minimum_energy_cost||
            (candidate.minimum_energy_cost==easy->minimum_energy_cost && candidate.id < easy->id)){
                easy= &candidate;
            }

        // Hard
        if (hard ==nullptr|| candidate.minimum_energy_cost>hard->minimum_energy_cost||
            (candidate.minimum_energy_cost== hard->minimum_energy_cost&& candidate.id<hard->id)){
                hard = &candidate;
            }
    }

    long long total_cost = 0;

    for (const auto& candidate: verified_candidates){
        total_cost += candidate.minimum_energy_cost;
    }
    const double average = static_cast<double>(total_cost)/static_cast<double>(verified_candidates.size());


    const MapCandidate* medium = nullptr;
    double best_distance = std::numeric_limits<double>::max();

    for (const auto& candidate : verified_candidates){
        if (candidate.minimum_energy_cost==easy->minimum_energy_cost || candidate.minimum_energy_cost== hard->minimum_energy_cost){
            continue;
        }
        const double distance = std::abs(static_cast<double>(candidate.minimum_energy_cost)-average);
        
        if (medium==nullptr || distance < best_distance||(distance== best_distance&& candidate.id<medium->id)){
            medium=&candidate;
            best_distance=distance;
        }
    }
    if (medium == nullptr){
        return std::nullopt;
    }
    
    return MapSelection{
        easy->id,
        medium->id,
        hard->id
    };
}