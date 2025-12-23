#pragma once
#include <vector>
#include <string>

struct Args {
    std::vector<std::string> templates;
    std::string output_file;
};

Args parse_args(int argc, char** argv);