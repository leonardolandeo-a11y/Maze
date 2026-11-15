#include "circuit_escape/map_generation/map_generator_cli.h"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    std::vector<std::string> arguments;
    arguments.reserve(static_cast<std::size_t>(argc - 1));
    for (int index = 1; index < argc; ++index) {
        arguments.emplace_back(argv[index]);
    }
    if (!arguments.empty() && arguments.front() == "--interactive") {
        arguments.erase(arguments.begin());
        return run_interactive_map_generator(arguments, std::cin, std::cout, std::cerr);
    }
    return run_map_generator(arguments, std::cout, std::cerr);
}
