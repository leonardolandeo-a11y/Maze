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
void run_map_selector_tests();
void run_kruskal_tests();
void run_dijkstra_tests();
void run_map_validator_tests();
void run_game_verifier_tests();
void run_map_batch_tests();
void run_map_storage_tests();
void run_map_generator_cli_tests();
void run_project2_integration_tests();

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
    run_map_selector_tests();
    run_kruskal_tests();
    run_dijkstra_tests();
    run_map_validator_tests();
    run_game_verifier_tests();
    run_map_batch_tests();
    run_map_storage_tests();
    run_map_generator_cli_tests();
    run_project2_integration_tests();
    return 0;
}