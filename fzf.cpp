#include "headers/fzf.hpp"
#include <cstdio>
#include <sstream>
#include <vector>

std::string fzf_select(const std::vector<std::string> &options, const std::string &query) {
    FILE* fzf = popen (
    ("printf \"" +
     std::string([&]() {
         std::string s;
         for (auto& o : options) s += o + "\\n";
         return s;
     }()) +
     "\" | fzf -1 -q \"" + query + "\"").c_str(),
    "r"
    );

    char buffer[1024];
    fgets(buffer, sizeof(buffer), fzf);
    pclose(fzf);

    return std::string(buffer).substr(0, std::string(buffer).find('\n'));
}

