#include "lexer.h"
#include <cctype>
#include <stdexcept>
Lexer::Lexer(const std::string& input) : input(input), pos(0) {}
char Lexer::peek() {
    return pos < input.size() ? input[pos] : '\0';
}
char Lexer::get() {
    return pos < input.size() ? input[pos++] : '\0';
}
size_t Lexer::getPosition() const {
    return pos;
}
void Lexer::setPosition(size_t p) {
    pos = p;
}
Token Lexer::nextToken() {
    while (isspace(peek())) get();
    if (pos >= input.size()) {
        return {Token::END, ""};
    }
    if (isdigit(peek())) {
        std::string num;
        while (isdigit(peek())) num.push_back(get());
        return {Token::NUM, num};
    }
    if (isalpha(peek()) || peek() == '_') {
        std::string var;
        while (isalnum(peek()) || peek() == '_') var.push_back(get());

        if (var == "function") return {Token::FUNCTION, var};
        if (var == "write") return {Token::WRITE, var};

        return {Token::VAR, var};
    }
    if (peek() == '-') {
        get();
        if (peek() == '>') {
            get();
            return {Token::ARROW, "->"};
        } else {
            return {Token::OP, "-"};
        }
    }
    if (peek() == '"') {
        get();  // skip opening quote
        std::string str;
        while (peek() != '"' && peek() != '\0') {
            str.push_back(get());
        }
        if (peek() == '"') get();  // skip closing quote
        else throw std::runtime_error("Unterminated string literal");
        return {Token::STRING, str};
    }
    char c = get();
    switch (c) {
        case '+': case '*': case '/':
            return {Token::OP, std::string(1, c)};
        case '(': return {Token::LPAREN, "("};
        case ')': return {Token::RPAREN, ")"};
        case '{': return {Token::LBRACE, "{"};
        case '}': return {Token::RBRACE, "}"};
        case ';': return {Token::SEMICOLON, ";"};
        case '\0': return {Token::END, ""};
        default: return {Token::END, ""};
    }
}