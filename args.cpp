#include "headers/args.hpp"
#include <stdexcept>

Args parse_args(const int argc, char** argv) {
    Args args;

    for (int i=1; i<argc; ++i) {
        std::string cur = argv[i];

        if (cur == "-t") {
            i++;
            while (i<argc && argv[i][0] != '-') {
                args.templates.emplace_back(argv[i]);
                i++;
            }
            i--;
        } else if (cur == "-f") {
            args.output_file = argv[++i];
        }
    }

    if (args.output_file.empty()) {
        throw std::runtime_error("Expected output file (-f)");
    }

    return args;
}
