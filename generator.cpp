#include "headers/generator.hpp"
#include <fstream>

void generate_main(
    const std::string& output,
    const std::vector<std::string>& templates
) {
    std::ofstream out(output);

    out <<
R"(#include <bits/stdc++.h>
using namespace std;

freopen("input.txt", "r", stdin);
freopen("output.txt", "w", stdout);

// ===== Templates =====
)";

    for (auto& t : templates)
        out << t << "\n\n";

    out <<
R"(int main() {
    return 0;
}
)";
}
