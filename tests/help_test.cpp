#include "circuit_escape/help.h"

#include <ftxui/screen/screen.hpp>

#include <cassert>
#include <string>

namespace {

std::string renderHelpToString(const HelpReport& report) {
    Help helpView;

    auto screen = ftxui::Screen::Create(
        ftxui::Dimension::Fixed(120),
        ftxui::Dimension::Fixed(40)
    );

    ftxui::Render(screen, helpView.RenderGame(report));

    return screen.ToString();
}

} // namespace

void run_help_tests() {
    HelpReport report;

    report.switch_a = {ObjectiveStatus::pending, 8};
    report.switch_b = {ObjectiveStatus::pending, 2};
    report.exit_goal = {ObjectiveStatus::locked, std::nullopt};

    report.total_cost = 10;

    report.recommended_order = {
        HelpTarget::switch_b,
        HelpTarget::switch_a,
        HelpTarget::exit
    };

    report.segment_costs = {2, 5, 3};

    // Caso 1: energía suficiente.
    report.current_energy = 12;

    std::string output = renderHelpToString(report);

    assert(output.find("Saldo estimado: 2") != std::string::npos);
    assert(output.find("Energia suficiente") != std::string::npos);

    // Caso 2: energía exactamente igual al costo.
    report.current_energy = 10;

    output = renderHelpToString(report);

    assert(output.find("Saldo estimado: 0") != std::string::npos);
    assert(output.find("no permite ganar") != std::string::npos);

    // Caso 3: energía insuficiente.
    report.current_energy = 8;

    output = renderHelpToString(report);

    assert(output.find("Saldo estimado: -2") != std::string::npos);
    assert(output.find("Energia insuficiente") != std::string::npos);

    // Caso 4: no existe un recorrido completo.
    report.total_cost = std::nullopt;
    report.switch_a.cost_from_current = std::nullopt;
    report.recommended_order.clear();
    report.segment_costs.clear();

    output = renderHelpToString(report);

    assert(output.find("Destino inaccesible") != std::string::npos);
    assert(output.find("Saldo estimado:") == std::string::npos);
}