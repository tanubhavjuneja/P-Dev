#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <string>
#include <vector>
void execStatements(const std::vector<std::string>& lines);
void interpretLine(const std::string& line);
#endif