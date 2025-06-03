#pragma once
#include <string>
#include <vector>
struct Token {
    enum Type {
        NUM, VAR, OP, LPAREN, RPAREN,
        LBRACE, RBRACE, SEMICOLON, ARROW,
        FUNCTION, WRITE, END,STRING
    } type;
    std::string text;
};
class Lexer {
public:
    Lexer(const std::string& input);
    Token nextToken();
    char peek();
    char get();
    size_t getPosition() const;
    void setPosition(size_t pos);

private:
    std::string input;
    size_t pos;
};