void run_grid_tests();
void run_environment_tests();
void run_interaction_tests();
// void run_controller_tests();
// void run_simulation_tests();

int main() {

    run_grid_tests();
    run_environment_tests();
    run_interaction_tests();
    //run_controller_tests();
    //run_simulation_tests();

    return 0;
}