#pragma once
#include "circuit_escape/project2_contracts.h"

#include <optional>
#include <vector>
#include <cmath>
#include <limits>
#include <set>
std::optional<MapSelection> select_maps(const std::vector<MapCandidate>& candidates);