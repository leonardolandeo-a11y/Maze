#include "circuit_escape/project2_generation.h"
#include "circuit_escape/project2_session.h"
#include <cassert>
#include <chrono>
#include <fstream>
#include <thread>

void run_project2_generation_tests() {
    const auto root = std::filesystem::temp_directory_path() /
        ("maze-runtime-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    assert((project2_map_sizes == std::array<Project2MapSize,3>{{{10,15},{20,30},{40,60}}}));
    for (const auto size : {Project2MapSize{0,0}, Project2MapSize{15,10}, Project2MapSize{100,150}}) {
        bool rejected = false;
        try { generate_project2_maps(size, root); }
        catch (const std::invalid_argument& e) {
            rejected = std::string(e.what()).find("Unsupported map size") != std::string::npos;
        }
        assert(rejected);
        assert(!std::filesystem::exists(root));
    }
    Project2Generation generation;
    for (const auto size : project2_map_sizes) {
        const auto config = project2_batch_config(size);
        assert(config.rows == size.rows && config.columns == size.columns);
        generation.start(size, root);
        assert(generation.loading());
        assert(generation.directory().empty()); // No playable directory during loading.
        bool duplicate_rejected = false;
        try { generation.start(size, root); }
        catch (const std::logic_error&) { duplicate_rejected = true; }
        assert(duplicate_rejected);
        while (!generation.poll()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
        assert(!generation.loading());
        assert(generation.error().empty());
        assert(generation.directory() == root / map_size_label(size));
        int previous_cost = -1;
        int level = 0;
        for (const auto* filename : {"mapa_facil.txt", "mapa_medio.txt", "mapa_dificil.txt"}) {
            auto map = load_map(generation.directory() / filename);
            assert(map.grid.rows() == size.rows && map.grid.columns() == size.columns);
            assert(static_cast<int>(map.difficulty) == level++);
            assert(map.minimum_energy_cost > previous_cost);
            previous_cost = map.minimum_energy_cost;
            Project2Session session(map);
            for (const auto action : map.winning_actions) (void)session.step(action);
            assert(session.state().outcome == GameOutcome::victory);
        }
    }
    // Changing size never overwrites another size's persisted maps.
    for (const auto size : project2_map_sizes) {
        const auto map = load_map(root / map_size_label(size) / "mapa_facil.txt");
        assert(map.grid.rows() == size.rows && map.grid.columns() == size.columns);
    }
    // Filesystem failures are published as an error, never as a stale ready directory.
    const auto blocked = root / "file";
    std::ofstream(blocked) << "not a directory";
    generation.start(project2_map_sizes.front(), blocked);
    while (!generation.poll()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    assert(!generation.error().empty() && generation.directory().empty());
    generation.start({100,150}, root);
    while (!generation.poll()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    assert(generation.error().find("Unsupported map size") != std::string::npos);
    std::filesystem::remove_all(root);
}
