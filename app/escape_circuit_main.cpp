#include "circuit_escape/project2_application.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

int main(int argc, char* argv[]) {
    std::filesystem::path maps_directory = "assets/maps";
    std::optional<SavedDifficulty> difficulty;
    for (int index = 1; index < argc; ++index) {
        const std::string_view argument = argv[index];
        if (argument == "--help") {
            std::cout << "Usage: escape_circuit [--maps DIR] [--difficulty easy|medium|hard]\n";
            return 0;
        }
        if (argument == "--maps" && index + 1 < argc) {
            maps_directory = argv[++index];
        } else if (argument == "--difficulty" && index + 1 < argc) {
            const std::string_view level = argv[++index];
            if (level == "easy") difficulty = SavedDifficulty::easy;
            else if (level == "medium") difficulty = SavedDifficulty::medium;
            else if (level == "hard") difficulty = SavedDifficulty::hard;
            else {
                std::cerr << "Unknown difficulty: " << level << '\n';
                return 1;
            }
        } else {
            std::cerr << "Unknown or incomplete option: " << argument << '\n';
            return 1;
        }
    }
    try {
        return Project2Application(std::move(maps_directory)).run(difficulty);
    } catch (const std::exception& exception) {
        std::cerr << "Game error: " << exception.what() << '\n';
        return 1;
    }
}
