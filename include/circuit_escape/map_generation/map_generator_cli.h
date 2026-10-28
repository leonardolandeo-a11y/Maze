#pragma once

#include <iosfwd>
#include <string>
#include <vector>

int run_map_generator(const std::vector<std::string>& arguments,
                      std::ostream& output, std::ostream& error);
