#include "lexer.h"
#include <cctype>
#include <stdexcept>
Lexer::Lexer(const std::string& input) : input(input), pos(0) {}
char Lexer::peek() {
    return pos < input.size() ? input[pos] : '\0';
}
char Lexer::get() {
    if (pos >= input.size()) return '\0';
    char c = input[pos++];
    if (c == '\n') {
        lineNumber++;
    }
    return c;
}
int Lexer::getLineNumber(size_t position) const {
    if (position > input.size())
        position = input.size();
    int line = 1;
    for (size_t i = 0; i < position; ++i) {
        if (input[i] == '\n')
            ++line;
    }
    return line;
}
void Lexer::setLineNumber(int newLine) {
    lineNumber = newLine;
}
size_t Lexer::getPosition() const {
    return pos;
}
void Lexer::setPosition(size_t p) {
    pos = p;
    hasBufferedToken=false;
}
Token Lexer::peekToken() {
    if (!hasBufferedToken) {
        bufferedToken = nextToken();
        hasBufferedToken = true;
    }
    return bufferedToken;
}
Token Lexer::nextToken() {
    if (hasBufferedToken) {
        hasBufferedToken = false;
        return bufferedToken;
    }
    while (true) {
        while (isspace(peek())) get();
        if (peek() == '/' && pos + 1 < input.size() && input[pos + 1] == '/') {
            get(); get();
            while (peek() != '\n' && peek() != '\0') get();
            continue;
        }
        if (peek() == '/' && pos + 1 < input.size() && input[pos + 1] == '*') {
            get(); get();
            while (!(peek() == '*' && pos + 1 < input.size() && input[pos + 1] == '/')) {
                if (peek() == '\0') {
                    throw std::runtime_error("Unterminated multi-line comment");
                }
                get();
            }
            get(); get(); 
            continue; 
        }
        break; 
    }
    if (pos >= input.size()) {
        return {Token::END, ""};
    }
    char current = peek();
    if (isdigit(current)) {
        std::string num;
        while (isdigit(peek())) num.push_back(get());
        return {Token::NUM, num};
    }
    if (isalpha(current) || current == '_') {
        std::string var;
        while (isalnum(peek()) || peek() == '_') var.push_back(get());
        if (var == "function") return {Token::FUNCTION, var};
        if (var == "write") return {Token::WRITE, var};
        if (var == "for") return {Token::FOR, var};
        if (var == "do") return {Token::DO, var};
        if (var == "while") return {Token::WHILE, var};
        if (var == "break") return {Token::BREAK, var};
        if (var == "continue") return {Token::CONTINUE, var};
        if (var == "pass") return {Token::PASS, var};
        if (var == "if") return {Token::IF, var};
        if (var == "elif") return {Token::ELIF, var};
        if (var == "else") return {Token::ELSE, var};
        if (var == "return") return {Token::RETURN, var};
        return {Token::VAR, var};
    }
    switch (current) {
        case '=':
            get();
            if (peek() == '=') {
                get();
                return {Token::EQUAL, "=="};
            }
            return {Token::ASSIGN, "="};

        case '!':
            get();
            if (peek() == '=') {
                get();
                return {Token::NOT_EQUAL, "!="};
            }
            throw std::runtime_error("Unexpected token '!'");
        
        case '<':
            get();
            if (peek() == '=') {
                get();
                return {Token::LESS_EQUAL, "<="};
            }
            return {Token::LESS, "<"};

        case '>':
            get();
            if (peek() == '=') {
                get();
                return {Token::GREATER_EQUAL, ">="};
            }
            return {Token::GREATER, ">"};
        case '"': {
            get();  
            std::string str;
            while (peek() != '"' && peek() != '\0') {
                str.push_back(get());
            }
            if (peek() == '"') get(); 
            else throw std::runtime_error("Unterminated string literal");
            return {Token::STRING, str};
        }
        case '-': {
            get(); 
            if (peek() == '>') {
                get();
                return {Token::ARROW, "->"};
            }
            if (peek() == '-') {
                get(); 
                return {Token::DECREMENT, "--"};
            }
            return {Token::OP, "-"};
        }
        case '+': {
            get(); 
            if (peek() == '+') {
                get(); 
                return {Token::INCREMENT, "++"};
            }
            return {Token::OP, "+"};
        }
        case '*': case '/': {
            char op = get();
            return {Token::OP, std::string(1, op)};
        }
        case '(': get(); return {Token::LPAREN, "("};
        case ')': get(); return {Token::RPAREN, ")"};
        case '{': get(); return {Token::LBRACE, "{"};
        case '}': get(); return {Token::RBRACE, "}"};
        case ',': get(); return {Token::COMMA, ","};
        case ';': get(); return {Token::SEMICOLON, ";"};
    }
    throw std::runtime_error(std::string("Unknown character: ") + get());
}