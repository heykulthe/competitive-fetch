#include "headers/args.hpp"
#include "headers/fetch.hpp"
#include "headers/generator.hpp"
#include <iostream>

int main(int argc, char** argv) {
    try {
        auto args = parse_args(argc, argv);
        auto index = load_or_fetch_index();

        std::vector<std::string> paths;
        for (auto& q : args.templates) {
            paths.push_back(resolve_template(q, &index));
        }

        auto contents = fetch_templates_batch(paths);

        generate_main(args.output_file, contents);
    } catch (const std::exception& e) {
        std::cerr << "cpf error: " << e.what() << "\n";
        return 1;
    }
}