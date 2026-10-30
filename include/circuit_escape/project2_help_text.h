#pragma once

#include "circuit_escape/project2_contracts.h"

#include <string>
#include <vector>

// Format the Dijkstra help shown by the active Project 2 game screen.
// Keep presentation text separate from FTXUI so it can be regression-tested.
[[nodiscard]] inline std::vector<std::string> format_project2_help_lines(const HelpReport& report) {
    const auto target_label = [](HelpTarget target) -> const char* {
        switch (target) {
            case HelpTarget::switch_a: return "A";
            case HelpTarget::switch_b: return "B";
            case HelpTarget::exit: return "EXIT";
        }
        return "EXIT";
    };
    const auto status_line = [](const char* label, const ObjectiveInfo& info) -> std::string {
        switch (info.status) {
            case ObjectiveStatus::activated: return std::string(label) + ": activated";
            case ObjectiveStatus::locked: return std::string(label) + ": locked";
            case ObjectiveStatus::enabled: break;
            case ObjectiveStatus::pending: break;
        }
        if (!info.cost_from_current) return std::string(label) + ": unreachable";
        return std::string(label) + ": " + std::to_string(*info.cost_from_current) + " energy";
    };

    std::string route = "Route: unavailable";
    std::string cost = "Cost: unreachable";
    if (report.total_cost) {
        route = "Route: CURRENT";
        for (HelpTarget target : report.recommended_order) {
            route += " -> ";
            route += target_label(target);
        }
        const long long remaining = static_cast<long long>(report.current_energy) -
            static_cast<long long>(*report.total_cost);
        const char* energy_status = remaining > 0 ? "ENOUGH" :
            (remaining == 0 ? "ZERO AT EXIT - RECHARGE" : "INSUFFICIENT - RECHARGE");
        cost = "Cost " + std::to_string(*report.total_cost) +
            " | Remaining " + std::to_string(remaining) +
            " | " + energy_status;
    }
    return {
        status_line("A", report.switch_a) + " | " +
            status_line("B", report.switch_b) + " | " + status_line("Exit", report.exit_goal),
        route,
        cost,
        "Estimate ignores guard movements, batteries and waiting",
        "Highlight: 1=A 2=B 3=route 0=off | H/Esc back | Z zoom"
    };
}
