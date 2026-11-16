#pragma once

#include <iosfwd>
#include <string>
#include <vector>

int run_map_generator(const std::vector<std::string>& arguments,
                      std::ostream& output, std::ostream& error);

int run_interactive_map_generator(const std::vector<std::string>& arguments,
                                  std::istream& input, std::ostream& output,
                                  std::ostream& error);
