#include "headers/args.hpp"
#include "headers/fzf.hpp"
#include "headers/fetch.hpp"
#include "headers/generator.hpp"

#include <fstream>

int main(int argc, char** argv) {
    auto args = parse_args(argc, argv);
    auto index = fetch_template_index();

    std::vector<std::string> contents;

    for (auto& q : args.templates) {
        auto match = fzf_select(index, q);
        contents.push_back(fetch_template(match));
    }

    generate_main(args.output_file, contents);
}
