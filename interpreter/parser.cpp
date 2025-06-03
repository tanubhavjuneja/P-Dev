#include "parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(Lexer lexer) : lexer(lexer) {
    currentToken = this->lexer.nextToken();
}

void Parser::consume(Token::Type expected) {
    if (currentToken.type == expected)
        currentToken = lexer.nextToken();
    else
        throw std::runtime_error("Unexpected token: " + currentToken.text);
}

int Parser::expr() {
    int result = term();
    while (currentToken.type == Token::OP && (currentToken.text == "+" || currentToken.text == "-")) {
        std::string op = currentToken.text;
        consume(Token::OP);
        int rhs = term();
        result = (op == "+") ? result + rhs : result - rhs;
    }
    return result;
}

int Parser::term() {
    int result = factor();
    while (currentToken.type == Token::OP && (currentToken.text == "*" || currentToken.text == "/")) {
        std::string op = currentToken.text;
        consume(Token::OP);
        int rhs = factor();
        if (op == "*") result *= rhs;
        else {
            if (rhs == 0) throw std::runtime_error("Division by zero");
            result /= rhs;
        }
    }
    return result;
}

int Parser::factor() {
    if (currentToken.type == Token::NUM) {
        int val = std::stoi(currentToken.text);
        consume(Token::NUM);
        return val;
    } else if (currentToken.type == Token::LPAREN) {
        consume(Token::LPAREN);
        int val = expr();
        consume(Token::RPAREN);
        return val;
    } else if (currentToken.type == Token::VAR) {
        std::string varName = currentToken.text;
        consume(Token::VAR);
        if (variables.find(varName) != variables.end()) {
            if (std::holds_alternative<int>(variables[varName]))
                return std::get<int>(variables[varName]);
            else
                throw std::runtime_error("Variable is not an integer: " + varName);
        } else {
            throw std::runtime_error("Undefined variable: " + varName);
        }
    } else {
        throw std::runtime_error("Unexpected token in factor: " + currentToken.text);
    }
}

void Parser::statement() {
    /*if (currentToken.type == Token::RBRACE) {
        std::cout << "Debug: Skipping stray RBRACE token" << std::endl;
        currentToken = lexer.nextToken();
        return;
    }*/
    std::cout << "Debug: Entering statement(), token: " << currentToken.text << std::endl;
    if (currentToken.type == Token::VAR) {
        std::string name = currentToken.text;
        consume(Token::VAR);
        if (currentToken.type == Token::ARROW) {
            std::cout << "Debug: Detected variable assignment to " << name << std::endl;
            consume(Token::ARROW);
            if (currentToken.type == Token::STRING) {
                variables[name] = currentToken.text;
                consume(Token::STRING);
            } else {
                int value = expr();
                variables[name] = value;
            }
            consume(Token::SEMICOLON);
        } else if (currentToken.type == Token::LPAREN) {
            std::cout << "Debug: Detected function call to " << name << std::endl;
            Value argVal;
            consume(Token::LPAREN);
            std::cout << "Debug: Consumed LPAREN\n";

            if (currentToken.type == Token::STRING) {
                argVal = currentToken.text;
                consume(Token::STRING);
                std::cout << "Debug: Consumed STRING argument\n";
            } else if (currentToken.type == Token::NUM) {
                argVal = std::stoi(currentToken.text);
                consume(Token::NUM);
                std::cout << "Debug: Consumed NUM argument\n";
            } else if (currentToken.type == Token::VAR) {
                std::string varName = currentToken.text;
                consume(Token::VAR);
                std::cout << "Debug: Consumed VAR argument\n";
                if (variables.find(varName) != variables.end()) {
                    argVal = variables[varName];
                } else {
                    throw std::runtime_error("Undefined variable: " + varName);
                }
            } else {
                throw std::runtime_error("Invalid argument in function call");
            }

            consume(Token::RPAREN);
            std::cout << "Debug: Consumed RPAREN\n";
            consume(Token::SEMICOLON);
            std::cout << "Debug: Consumed SEMICOLON\n";

            if (functions.find(name) != functions.end()) {
                auto& func = functions[name];
                // Save current lexer position before jumping to function body
                returnStates.push({lexer.getPosition(), currentToken});
                lexer.setPosition(func.position);
                currentToken = lexer.nextToken();

                // Set parameter variable if function takes one argument
                if (!func.param.empty()) {
                    variables[func.param] = argVal;
                }

                int braceCount = 1;
                while (braceCount > 0 && currentToken.type != Token::END) {
                    if (currentToken.type == Token::LBRACE) {
                        braceCount++;
                        consume(Token::LBRACE);
                    } else if (currentToken.type == Token::RBRACE) {
                        braceCount--;
                        consume(Token::RBRACE);
                        // If braceCount is now zero, don't continue parsing statements inside function
                        if (braceCount == 0) break;
                    } else {
                        statement();  // safe to call only when not on braces
                    }
                }
/*
                if (currentToken.type == Token::RBRACE) {
                    consume(Token::RBRACE);  // consume the closing brace
                } else {
                    throw std::runtime_error("Expected closing brace at end of function");
                }*/

                // Restore lexer position to after the function call
                auto [pos, savedToken] = returnStates.top();
                returnStates.pop();
                lexer.setPosition(pos);
                currentToken = savedToken;
                std::cout << "Debug: Returned to position after function call, current token: " << currentToken.text << std::endl;
            } else {
                throw std::runtime_error("Undefined function: " + name);
            }
        } else {
            throw std::runtime_error("Expected '->' or '(' after variable");
        }
    } else if (currentToken.type == Token::WRITE) {
        std::cout << "Debug: Processing write statement" << std::endl;
        consume(Token::WRITE);
        consume(Token::LPAREN);
        if (currentToken.type == Token::VAR) {
            std::string varName = currentToken.text;
            consume(Token::VAR);
            if (variables.find(varName) != variables.end()) {
                if (std::holds_alternative<int>(variables[varName]))
                    std::cout << std::get<int>(variables[varName]) << std::endl;
                else
                    std::cout << std::get<std::string>(variables[varName]) << std::endl;
            } else {
                std::cout << "Undefined variable: " << varName << std::endl;
            }
        } else if (currentToken.type == Token::STRING) {
            std::cout << currentToken.text << std::endl;
            consume(Token::STRING);
        } else {
            throw std::runtime_error("Invalid argument to write()");
        }
        consume(Token::RPAREN);
        consume(Token::SEMICOLON);
    } else if (currentToken.type == Token::FUNCTION) {
        std::cout << "Debug: Parsing function definition" << std::endl;
        consume(Token::FUNCTION);
        if (currentToken.type != Token::VAR)
            throw std::runtime_error("Expected function name");
        std::string funcName = currentToken.text;
        consume(Token::VAR);
        consume(Token::LPAREN);

        std::string param;
        if (currentToken.type == Token::VAR) {
            param = currentToken.text;
            consume(Token::VAR);
        }

        consume(Token::RPAREN);
        functions[funcName] = { lexer.getPosition(), param };
        consume(Token::LBRACE);
        int braceCount = 1;
        while (braceCount > 0) {
            Token tok = lexer.nextToken();
            if (tok.type == Token::LBRACE) braceCount++;
            else if (tok.type == Token::RBRACE) braceCount--;
        }
        currentToken = lexer.nextToken();
    } else {
        throw std::runtime_error("Unknown statement starting with token: " + currentToken.text);
    }
}
int Parser::parseExpression() {
    int val = expr();
    if (currentToken.type != Token::END) {
        throw std::runtime_error("Unexpected tokens after expression");
    }
    return val;
}

void Parser::parse() {
    while (currentToken.type != Token::END) {
        if (currentToken.type == Token::RBRACE) {
            std::cout << "Debug: Skipping RBRACE at top level\n";
            currentToken = lexer.nextToken();
            continue;
        }

        // Skip over function definitions — already handled when seen
        if (currentToken.type == Token::FUNCTION) {
            std::cout << "Debug: Skipping over function definition at top level\n";
            statement();  // still call statement() to record function info
            continue;
        }

        statement();
    }
}

