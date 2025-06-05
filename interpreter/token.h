#ifndef TOKEN_H
#define TOKEN_H
#include <string>
struct Token {
    enum Type {
        NUM, VAR, OP, LPAREN, RPAREN,
        LBRACE, RBRACE, SEMICOLON, ARROW,
        FUNCTION, WRITE, END, STRING, 
        IF, ELSE, EQUAL, NOT_EQUAL, LESS, 
        GREATER, LESS_EQUAL, GREATER_EQUAL,
        ASSIGN, UNKNOWN, COMMA, RETURN,  ELIF,
        CONTINUE, FOR, WHILE, DO, PASS, RETURN,
        BREAK, INCREMENT, DECREMENT
    } type;
    std::string text;
};
#endif 