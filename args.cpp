#include "headers/args.hpp"
#include <stdexcept>

Args parse_args(const int argc, char** argv) {
    Args args;

    for (int i=1; i<argc; ++i) {
        std::string cur = argv[i];

        if (cur == "-t" || cur == "--templates") {
            i++;
            while (i<argc && argv[i][0] != '-') {
                args.templates.emplace_back(argv[i]);
                i++;
            }
            i--;
        } else if (cur == "-f" || cur == "--file") {
            args.output_file = argv[++i];
        } else if (cur == "-r" || cur == "--refresh") {
            args.force_refresh = true;
        }
    }

    if (args.output_file.empty()) {
        throw std::runtime_error("Expected output file (-f)");
    }

    return args;
}