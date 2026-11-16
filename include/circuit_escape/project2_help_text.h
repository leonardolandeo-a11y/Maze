#pragma once

#include "circuit_escape/project2_contracts.h"
#include "circuit_escape/help_controls.h"

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
    const auto status_line = [&](const char* label, const ObjectiveInfo& info) -> std::string {
        const std::string prefix = std::string(label) + ": ";
        if (info.status == ObjectiveStatus::activated) return prefix + "activated (-)";
        if (info.status == ObjectiveStatus::locked) {
            std::string missing;
            if (report.switch_a.status != ObjectiveStatus::activated) missing = "A";
            if (report.switch_b.status != ObjectiveStatus::activated)
                missing += missing.empty() ? "B" : "+B";
            return prefix + "requires " + missing + " (-)";
        }
        const std::string state = info.status == ObjectiveStatus::enabled ? "enabled " : "pending ";
        if (!info.cost_from_current) return prefix + state + "unreachable";
        return prefix + state + std::to_string(*info.cost_from_current) + " energy";
    };

    std::string route = "Route: unavailable";
    std::string cost = "Cost: unreachable";
    if (report.total_cost) {
        route = "Route: CURRENT";
        for (std::size_t i = 0; i < report.recommended_order.size(); ++i) {
            // These are consecutive leg costs, not independent costs from CURRENT.
            route += i < report.segment_costs.size()
                ? " -[" + std::to_string(report.segment_costs[i]) + "]-> " : " -> ";
            route += target_label(report.recommended_order[i]);
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
        "Estimate ignores guard, batteries, trap penalties and waiting",
        "Highlight: 1=A 2=B 3=route 0=off | H/Esc back",
        MazeUI::zoom_help
    };
}
