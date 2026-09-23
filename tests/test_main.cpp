void run_grid_tests();
void run_environment_tests();
void run_interaction_tests();
void run_controller_tests();
void run_simulation_tests();
void run_tests_scenario_1();
void run_tests_scenario_2();
void run_generic_function_tests();
void run_console_ui_tests();
void run_event_tests();
void run_console_render_tests();

int main() {

    run_grid_tests();
    run_environment_tests();
    run_interaction_tests();
    run_controller_tests();
    run_simulation_tests();
    run_tests_scenario_1();
    run_tests_scenario_2();
    run_generic_function_tests();
    run_console_ui_tests();
    run_event_tests();
    run_console_render_tests();
    return 0;
}