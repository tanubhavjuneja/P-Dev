#include "interpreter.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <script-file>\n";
        return 1;
    }
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Failed to open file: " << argv[1] << "\n";
        return 1;
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    execStatements(lines);
    return 0;
}