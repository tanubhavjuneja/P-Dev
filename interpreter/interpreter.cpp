#include "interpreter.h"
#include "parser.h"
#include <iostream>
#include <sstream>
void interpretLine(const std::string& line) {
    try {
        Parser parser(line);
        parser.statement();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
void execStatements(const std::vector<std::string>& lines) {
    std::string fullInput;
    for (const auto& line : lines) {
        fullInput += line + "\n";
    }
    try {
        Parser parser(fullInput);
        parser.parse();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}