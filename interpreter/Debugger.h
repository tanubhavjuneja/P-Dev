#pragma once
#include <string>
#include "lexer.h"
class Debugger {
public:
    static void enable(bool flag);
    static void log(const std::string& message, int lineNumber = -1);
    static void printContextTokens(Lexer& lexer, int contextSize = 5);
};