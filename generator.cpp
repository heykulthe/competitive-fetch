#include "headers/generator.hpp"
#include <fstream>
#include <filesystem>

void generate_main(const std::string& file,
                   const std::vector<std::string>& templates) {
    std::ofstream out(file);

    out << "#include <bits/stdc++.h>\n";
    out << "using namespace std;\n\n";
    out << "// ===== Templates =====\n\n";

    for (auto& t : templates)
        out << t << "\n\n";

    out << "int main() {\n";
    out << "    freopen(\"input.txt\", \"r\", stdin);\n";
    out << "    freopen(\"output.txt\", \"w\", stdout);\n\n";
    out << "    ios::sync_with_stdio(0);\n";
    out << "    cin.tie(0);\n";
    out << "    return 0;\n";
    out << "}\n";

    std::ofstream("input.txt");
    std::ofstream("output.txt");
}
