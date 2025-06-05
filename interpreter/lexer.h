#pragma once
#include <string>
#include <vector>
struct Token {
    enum Type {
        NUM, VAR, OP, LPAREN, RPAREN,
        LBRACE, RBRACE, SEMICOLON, ARROW,
        FUNCTION, WRITE, END, STRING, 
        IF, ELSE, EQUAL, NOT_EQUAL, LESS, 
        GREATER, LESS_EQUAL, GREATER_EQUAL,
        ASSIGN, COMMA, RETURN, ELIF, CONTINUE,
        FOR, WHILE, DO, PASS, BREAK, INCREMENT,
        DECREMENT
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
    Token peekToken();
    int getLineNumber(size_t position) const;
    void setLineNumber(int newLine);
private:
    std::string input;
    size_t pos;
    bool hasBufferedToken = false;
    Token bufferedToken;
    int lineNumber = 1;
};