#pragma once
#include <string>
#include <stdexcept>
class ErrorHandler {
public:
    static void throwError(const std::string& message, int lineNumber = -1);
};