#ifndef TOKEN_H
#define TOKEN_H

#include <string>

struct Token {
    enum Type {
        NUM,
        OP,
        VAR,
        LPAREN,
        RPAREN,
        ARROW,
        SEMICOLON,
        WRITE,
        FUNCTION,
        LBRACE,
        RBRACE,
        END,
        UNKNOWN
    } type;

    std::string text;
};

#endif // TOKEN_H
