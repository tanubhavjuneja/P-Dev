#include "ErrorHandler.h"
#include <sstream>
void ErrorHandler::throwError(const std::string& message, int lineNumber) {
    std::ostringstream oss;
    if (lineNumber >= 0) {
        oss << "Error at line " << lineNumber << ": ";
    }
    oss << message;
    throw std::runtime_error(oss.str());
}