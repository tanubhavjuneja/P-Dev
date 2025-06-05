#include "Debugger.h"
#include "lexer.h"
#include <iostream>
#include <sstream>
static bool debugEnabled = true;
void Debugger::enable(bool flag) {
    debugEnabled = flag;
}
void Debugger::log(const std::string& message, int lineNumber) {
    if (!debugEnabled) return;
    std::ostringstream oss;
    if (lineNumber >= 0) {
        oss << "Debug (line " << lineNumber << "): ";
    } else {
        oss << "Debug: ";
    }
    oss << message;
    std::cout << oss.str() << std::endl;
}
void Debugger::printContextTokens(Lexer& lexer, int contextSize) {
    int lexerPos = lexer.getPosition();
    lexer.setPosition(0);

    std::vector<std::pair<Token, int>> tokensWithPos;
    while (true) {
        int pos = lexer.getPosition();
        Token tok = lexer.nextToken();
        tokensWithPos.emplace_back(tok, pos);
        if (tok.type == Token::END) break;
    }

    int tokenIndex = 0;
    for (; tokenIndex < (int)tokensWithPos.size(); tokenIndex++) {
        if (tokensWithPos[tokenIndex].second >= lexerPos) break;
    }
    if (tokenIndex >= (int)tokensWithPos.size()) tokenIndex = (int)tokensWithPos.size() - 1;

    int start = std::max(0, tokenIndex - contextSize);
    int end = std::min((int)tokensWithPos.size() - 1, tokenIndex + contextSize);

    Debugger::log("---- Token Context ----");
    for (int i = start; i <= end; ++i) {
        const auto& [tok, pos] = tokensWithPos[i];
        std::string prefix = (i == tokenIndex - 1) ? ">> " : "   ";
        Debugger::log(prefix + "Token[" + std::to_string(i) + "] at pos " +
                      std::to_string(pos) + ": \"" + tok.text +
                      "\" (type=" + std::to_string(tok.type) + ")");
    }
    Debugger::log("------------------------");

    lexer.setPosition(lexerPos);
}
