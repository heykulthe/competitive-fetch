#include "headers/fetch.hpp"
#include <cstdlib>
#include <fstream>

static const std::string BASE =
    "https://raw.githubusercontent.com/heykulthe/cp-templates/main/";

std::vector<std::string> fetch_template_index() {
    system("curl -s https://api.github.com/repos/heykulthe/cp-templates/git/trees/main?recursive=1 > /tmp/cpf_index.json");

    std::ifstream in("/tmp/cpf_index.json");
    std::vector<std::string> files;
    std::string line;

    while (getline(in, line)) {
        if (line.find("\"path\":") != std::string::npos &&
            line.find(".cpp") != std::string::npos) {
            auto start = line.find(": \"") + 3;
            auto end = line.find("\"", start);
            files.push_back(line.substr(start, end - start));
            }
    }
    return files;
}

std::string fetch_template(const std::string& path) {
    std::string cmd = "curl -s " + BASE + path;
    FILE* pipe = popen(cmd.c_str(), "r");

    std::string content;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe))
        content += buffer;

    pclose(pipe);
    return content;
}
