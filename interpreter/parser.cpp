#include "parser.h"
#include "Debugger.h"
#include "ErrorHandler.h"
#include <iostream>
Parser::Parser(Lexer lexer) : lexer(lexer) {
    currentToken = this->lexer.nextToken();
}
void Parser::consume(Token::Type expected) {
    if (currentToken.type == expected)
        currentToken = lexer.nextToken();
    else
        ErrorHandler::throwError("Expected token type " + std::to_string(expected) + " but found '" + currentToken.text + "'", lexer.getLineNumber(lexer.getPosition()));
}
void Parser::pushScope() {
    variableStack.emplace_back();
}
void Parser::popScope() {
    if (!variableStack.empty()) {
        variableStack.pop_back();
    } else {
        ErrorHandler::throwError("Variable scope stack underflow", lexer.getLineNumber(lexer.getPosition()));
    }
}
Value Parser::lookupVariableValue(const std::string& name) {
    for (auto it = variableStack.rbegin(); it != variableStack.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second;
        }
    }
    ErrorHandler::throwError("Undefined variable: " + name, lexer.getLineNumber(lexer.getPosition()));
    return Value(); 
}
void Parser::setVariableValue(const std::string& name, Value value) {
    if (variableStack.empty()) {
        ErrorHandler::throwError("No variable scope available", lexer.getLineNumber(lexer.getPosition()));
    }
    variableStack.back()[name] = value;
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
void Parser::statement() {
    Debugger::log("Entering statement() with token: " + currentToken.text + ", Type: " + std::to_string(currentToken.type));
    switch (currentToken.type) {
        case Token::END:
            Debugger::log("Reached END token in statement()");
            return;
            case Token::FOR:
            parseForStatement();
            break;
        case Token::DO:
            parseDoStatement();
            break;
        case Token::BREAK:
            Debugger::log("Processing break statement");
            consume(Token::BREAK);
            consume(Token::SEMICOLON);
            loopBreak = true;
            break;
        case Token::CONTINUE:
            Debugger::log("Processing continue statement");
            consume(Token::CONTINUE);
            consume(Token::SEMICOLON);
            loopContinue = true;
            break;
        case Token::WHILE:
            parseWhileStatement();
            break;
        case Token::PASS:
            Debugger::log("Processing pass statement");
            consume(Token::PASS);
            consume(Token::SEMICOLON);
            break;
        case Token::IF:
            parseIfStatement();
            break;
        case Token::VAR:
            parseVarOrFunctionCall();
            break;
        case Token::WRITE:
            parseWriteStatement();
            break;
        case Token::FUNCTION:
            parseFunctionDefinition();
            break;
        case Token::RETURN:
            parseReturnStatement();
            break;
        default:
            ErrorHandler::throwError("Unknown statement starting with token: " + currentToken.text, lexer.getLineNumber(lexer.getPosition()));
    }
}
bool Parser::parseCondition() {
    int left = expr();
    if (currentToken.type == Token::EQUAL || currentToken.type == Token::NOT_EQUAL || currentToken.type == Token::LESS || currentToken.type == Token::LESS_EQUAL || currentToken.type == Token::GREATER || currentToken.type == Token::GREATER_EQUAL) {
        Token op = currentToken;
        consume(op.type);
        int right = expr();
        switch (op.type) {
            case Token::EQUAL: return left == right;
            case Token::NOT_EQUAL: return left != right;
            case Token::LESS: return left < right;
            case Token::LESS_EQUAL: return left <= right;
            case Token::GREATER: return left > right;
            case Token::GREATER_EQUAL: return left >= right;
            default: ErrorHandler::throwError("Invalid comparison operator in condition", lexer.getLineNumber(lexer.getPosition())); return false;
        }
    } else {
        return left != 0;
    }
}
int Parser::term() {
    int result = factor();
    while (currentToken.type == Token::OP && (currentToken.text == "*" || currentToken.text == "/")) {
        std::string op = currentToken.text;
        consume(Token::OP);
        int rhs = factor();
        if (op == "*") result *= rhs;
        else {
            if (rhs == 0) ErrorHandler::throwError("Division by zero", lexer.getLineNumber(lexer.getPosition()));
            result /= rhs;
        }
    }
    return result;
}
int Parser::factor() {
    if (currentToken.type == Token::LPAREN) {
        consume(Token::LPAREN);
        int val = expr();
        consume(Token::RPAREN);
        return val;
    } else if (currentToken.type == Token::NUM) {
        int val = std::stoi(currentToken.text);
        Debugger::log(currentToken.text);
        consume(Token::NUM);
        return val;
    } else if (currentToken.type == Token::VAR) {
        std::string name = currentToken.text;
        Token nextToken = lexer.peekToken();
        if (nextToken.type == Token::LPAREN) {
            consume(Token::VAR);
            return parseFunctionCallArgsAndExecute(name);
        } else {
            consume(Token::VAR);
            Value val = lookupVariableValue(name);
            if (std::holds_alternative<int>(val)) {
                return std::get<int>(val);
            } else {
                ErrorHandler::throwError("Variable is not an integer: " + name, lexer.getLineNumber(lexer.getPosition()));
                return 0;
            }
        }
    } else if (currentToken.type == Token::OP && currentToken.text == "-") {
        consume(Token::OP);
        return -factor();
    } else {
        ErrorHandler::throwError("Unexpected token in factor: " + currentToken.text, lexer.getLineNumber(lexer.getPosition()));
        return 0;
    }
}
void Parser::parseReturnStatement() {
    consume(Token::RETURN);
    if (currentToken.type != Token::SEMICOLON) {
        returnValue = expr();
    } else {
        returnValue = Value();
    }
    hasReturnValue = true;
    consume(Token::SEMICOLON);
}
void Parser::executeBlock() {
    pushScope();
    int braceCount = 1;
    consume(Token::LBRACE);
    while (braceCount > 0 && currentToken.type != Token::END && !hasReturnValue) {
        if (currentToken.type == Token::LBRACE) {
            braceCount++;
            consume(Token::LBRACE);
        } else if (currentToken.type == Token::RBRACE) {
            braceCount--;
            consume(Token::RBRACE);
        } else {
            statement();
        }
    }
    popScope();
}
void Parser::parseAssignmentExpression() {
    std::string varName = currentToken.text;
    std::cout<<currentToken.text;
    consume(Token::VAR);
    consume(Token::ARROW); 
    Value val = expr();
    setVariableValue(varName, val);
}
Parser::ForUpdateInfo Parser::parseForUpdateExpression() {
    ForUpdateInfo info;
    info.varName = currentToken.text;
    consume(Token::VAR);
    if (currentToken.text == "++") {
        consume(Token::INCREMENT);
        info.op = UpdateOp::INCREMENT;
    } else if (currentToken.text == "--") {
        consume(Token::DECREMENT);
        info.op = UpdateOp::DECREMENT;
    } else if (currentToken.type == Token::ARROW) {
        consume(Token::ARROW);
        int value = expr();
        info.op = UpdateOp::ASSIGN;
        info.assignedValue = value;
        info.hasAssignedValue = true;
    } else {
        ErrorHandler::throwError("Invalid update expression in for loop", lexer.getLineNumber(lexer.getPosition()));
    }
    return info;
}
void Parser::parseForStatement() {
    Debugger::log("Parsing for loop");
    consume(Token::FOR);
    consume(Token::LPAREN);

    Debugger::log("Entering new scope for for loop");
    pushScope();

    if (currentToken.type != Token::SEMICOLON) {
        Debugger::log("Parsing initialization/assignment part of for loop");
        parseAssignmentExpression();
    }
    size_t conditionPos = lexer.getPosition();
    consume(Token::SEMICOLON);

    Debugger::log("Saving position for condition expression");
    
    bool condition = true;

    if (currentToken.type != Token::SEMICOLON) {
        Debugger::log("Parsing condition of for loop");
        condition = parseCondition();
        Debugger::log("Initial condition evaluated to: " + std::string(condition ? "true" : "false"));
    }
    consume(Token::SEMICOLON);

    size_t updatePos = lexer.getPosition();
    bool hasUpdate = (currentToken.type != Token::RPAREN);
    ForUpdateInfo updateInfo;

    if (hasUpdate) {
        Debugger::log("Parsing update expression of for loop");
        updateInfo = parseForUpdateExpression();
        Debugger::log("Parsed update: var = " + updateInfo.varName);
    }

    consume(Token::RPAREN);

    size_t blockStart = lexer.getPosition();
    Token savedToken = currentToken;

    while (condition && !hasReturnValue) {
        Debugger::log("For loop iteration start");
        loopBreak = false;
        loopContinue = false;

        lexer.setPosition(blockStart);
        currentToken = savedToken;

        Debugger::log("Executing for loop block");
        executeBlock();

        if (loopBreak) {
            Debugger::log("Loop break encountered");
            break;
        }
        if (hasReturnValue) {
            Debugger::log("Return encountered inside for loop");
            break;
        }

        if (hasUpdate) {
            Debugger::log("Applying update expression");
            Value oldVal = lookupVariableValue(updateInfo.varName);

            switch (updateInfo.op) {
                case UpdateOp::INCREMENT:
                    if (!std::holds_alternative<int>(oldVal))
                        ErrorHandler::throwError("Cannot increment non-integer variable: " + updateInfo.varName);
                    setVariableValue(updateInfo.varName, Value(std::get<int>(oldVal) + 1));
                    lookupVariableValue(updateInfo.varName);
                    Debugger::log("Incremented variable " + updateInfo.varName);
                    break;

                case UpdateOp::DECREMENT:
                    if (!std::holds_alternative<int>(oldVal))
                        ErrorHandler::throwError("Cannot decrement non-integer variable: " + updateInfo.varName);
                    setVariableValue(updateInfo.varName, Value(std::get<int>(oldVal) - 1));
                    Debugger::log("Decremented variable " + updateInfo.varName);
                    break;

                case UpdateOp::ASSIGN:
                    if (updateInfo.hasAssignedValue) {
                        setVariableValue(updateInfo.varName, updateInfo.assignedValue);
                        Debugger::log("Assigned value to variable " + updateInfo.varName);
                    } else {
                        ErrorHandler::throwError("No value for assignment update in for loop");
                    }
                    break;

                default:
                    Debugger::log("No update operation");
                    break;
            }
        }

        Debugger::log("Re-evaluating for loop condition");
        lexer.setPosition(conditionPos);
        currentToken = lexer.peekToken();
        Debugger::printContextTokens(lexer);
        condition = parseCondition();
        
        Debugger::log("Condition result: " + std::string(condition ? "true" : "false"));
    }

    Debugger::log("Popping for loop scope");
    popScope();

    if (!condition || hasReturnValue) {
        Debugger::log("Skipping remaining for loop block");
        lexer.setPosition(blockStart);
        currentToken = savedToken;
        skipBlock();
    }
}
void Parser::parseDoStatement() {
    consume(Token::DO);
    size_t loopStart = lexer.getPosition();
    executeBlock();                
}
void Parser::parseWhileStatement() {
    consume(Token::WHILE);
    consume(Token::LPAREN);
    size_t conditionPos = lexer.getPosition();
    bool condition = parseCondition();
    consume(Token::RPAREN);
    size_t blockStart = lexer.getPosition();
    Token savedToken = currentToken;
    while (condition && !hasReturnValue) {
        lexer.setPosition(blockStart);
        currentToken = savedToken;
        loopBreak = false;
        loopContinue = false;
        executeBlock();
        if (loopBreak) break;
        if (hasReturnValue) break;
        lexer.setPosition(conditionPos);
        currentToken = lexer.nextToken();
        condition = parseCondition();
        consume(Token::RPAREN); 
    }
    if (!condition || hasReturnValue) {
        lexer.setPosition(blockStart);
        currentToken = savedToken;
        skipBlock();  
    }
}
void Parser::skipBlock() {
    int braceCount = 0;
    if (currentToken.type == Token::LBRACE) {
        braceCount = 1;
        consume(Token::LBRACE);
    } else {
        statement();
        return;
    }
    while (braceCount > 0 && currentToken.type != Token::END) {
        if (currentToken.type == Token::LBRACE) {
            braceCount++;
            consume(Token::LBRACE);
        } else if (currentToken.type == Token::RBRACE) {
            braceCount--;
            consume(Token::RBRACE);
        } else {
            consume(currentToken.type);  // Just consume tokens inside block
        }
    }
}
void Parser::skipRemainingElifElseBlocks() {
    while (currentToken.type == Token::ELIF || currentToken.type == Token::ELSE) {
        if (currentToken.type == Token::ELIF) {
            consume(Token::ELIF);
            consume(Token::LPAREN);
            // Skip condition expression
            parseCondition();
            consume(Token::RPAREN);
            skipBlock();
        } else if (currentToken.type == Token::ELSE) {
            consume(Token::ELSE);
            skipBlock();
        }
    }
}
void Parser::parseIfStatement() {
    consume(Token::IF);
    consume(Token::LPAREN);
    bool condition = parseCondition();
    consume(Token::RPAREN);
    if (condition) {
        executeBlock();
        skipRemainingElifElseBlocks();
    } else {
        skipBlock();
        while (currentToken.type == Token::ELIF || currentToken.type == Token::ELSE) {
            if (currentToken.type == Token::ELIF) {
                consume(Token::ELIF);
                consume(Token::LPAREN);
                bool elifCondition = parseCondition();
                consume(Token::RPAREN);

                if (elifCondition) {
                    executeBlock();
                    skipRemainingElifElseBlocks();
                    break;
                } else {
                    skipBlock();
                }
            } else if (currentToken.type == Token::ELSE) {
                consume(Token::ELSE);
                executeBlock();
                break;
            }
        }
    }
}
void Parser::parseVarOrFunctionCall() {
    std::string name = currentToken.text;
    Token nextToken = lexer.peekToken();
    if (nextToken.type == Token::ARROW) {
        Debugger::log("Detected variable assignment to " + name);
        consume(Token::VAR);
        consume(Token::ARROW);
        if (currentToken.type == Token::STRING) {
            setVariableValue(name, currentToken.text);
            consume(Token::STRING);
        } else {
            int value = expr(); 
            setVariableValue(name, value);
        }
        consume(Token::SEMICOLON);
    }
    else if (nextToken.type == Token::LPAREN) {
        Debugger::log("Detected function call to " + name);
        consume(Token::VAR);
        parseFunctionCallArgsAndExecute(name);
        consume(Token::SEMICOLON);
    }
    else {
        ErrorHandler::throwError("Expected '->' or '(' after variable");
    }
}
std::vector<Value> Parser::parseFunctionArguments(const std::unordered_map<std::string, Value>& currentScope) {
    std::vector<Value> args;
    consume(Token::LPAREN);
    Debugger::log("Parsing function arguments...");
    if (currentToken.type != Token::RPAREN) {
        while (true) {
            Value argVal;
            Debugger::log("Current token in args: " + currentToken.text + " (type " + std::to_string(currentToken.type) + ")");
            if (currentToken.type == Token::STRING) {
                argVal = currentToken.text;
                Debugger::log("Parsed string argument: " + currentToken.text);
                consume(Token::STRING);
            }
            else if (currentToken.type == Token::NUM) {
                argVal = std::stoi(currentToken.text);
                Debugger::log("Parsed numeric argument: " + currentToken.text);
                consume(Token::NUM);
            }
            else if (currentToken.type == Token::VAR) {
                std::string varName = currentToken.text;
                Debugger::log("Parsed variable argument: " + varName);
                consume(Token::VAR);
                if (currentScope.find(varName) != currentScope.end()) {
                    argVal = currentScope.at(varName);
                    Debugger::log("Resolved variable " + varName + " to value");
                } else {
                    ErrorHandler::throwError("Undefined variable: " + varName);
                }
            }
            else {
                ErrorHandler::throwError("Invalid argument in function call");
            }
            args.push_back(argVal);
            if (currentToken.type == Token::COMMA) {
                consume(Token::COMMA);
                Debugger::log("Found comma, continuing to next argument...");
            } else {
                break;
            }
        }
    }
    consume(Token::RPAREN);
    Debugger::log("Completed parsing arguments. Total args: " + std::to_string(args.size()));
    return args;
}
int Parser::parseFunctionCallArgsAndExecute(const std::string& funcName) {
    Debugger::log("Detected function call to " + funcName);
    auto args = parseFunctionArguments(variableStack.back());
    if (functions.find(funcName) == functions.end())
        ErrorHandler::throwError("Undefined function: " + funcName);
    auto& func = functions[funcName];
    if (args.size() != func.params.size())
        ErrorHandler::throwError("Function " + funcName + " expects " + std::to_string(func.params.size()) + " arguments, but got " + std::to_string(args.size()));
    Debugger::log("Executing function '" + funcName + "' at position " + std::to_string(func.position));
    Debugger::log("Pushing return state: pos=" + std::to_string(lexer.getPosition()));
    returnStates.push({lexer.getPosition(), currentToken});
    lexer.setPosition(func.position);
    currentToken = lexer.nextToken();
    consume(Token::LBRACE);
    pushScope();
    for (size_t i = 0; i < func.params.size(); ++i)
        setVariableValue(func.params[i], args[i]);
    int braceCount = 1;
    hasReturnValue = false;
    lastReturnValue = 0;
    Debugger::log("Entering function body...");
    while (braceCount > 0 && currentToken.type != Token::END && !hasReturnValue) {
        if (currentToken.type == Token::LBRACE) {
            braceCount++;
            consume(Token::LBRACE);
        } else if (currentToken.type == Token::RBRACE) {
            braceCount--;
            consume(Token::RBRACE);
            if (braceCount == 0) break;
        } else
            statement();
    }
    Debugger::log("Exiting function '" + funcName + "' with return value " + std::to_string(lastReturnValue));
    popScope();
    Debugger::log("Popping return state");
    auto [pos, savedToken] = returnStates.top();
    returnStates.pop();
    Debugger::log("Restoring lexer position to " + std::to_string(pos));
    lexer.setPosition(pos);
    currentToken = savedToken;
    Debugger::log("Finished executing function '" + funcName + "'");
    return lastReturnValue;
}
void Parser::parseWriteStatement() {
    Debugger::log("Processing write statement");
    consume(Token::WRITE);
    consume(Token::LPAREN);
    if (currentToken.type == Token::VAR) {
        std::string varName = currentToken.text;
        consume(Token::VAR);
        if (!variableStack.empty() && variableStack.back().find(varName) != variableStack.back().end()) {
            const Value& val = variableStack.back().at(varName);
            if (std::holds_alternative<int>(val))
                std::cout << std::get<int>(val) << std::endl;
            else
                std::cout << std::get<std::string>(val) << std::endl;
        } else
            Debugger::log("Undefined variable: " + varName);
    } else if (currentToken.type == Token::STRING) {
        std::cout << currentToken.text << std::endl;
        consume(Token::STRING);
    } else
        ErrorHandler::throwError("Invalid argument to write()");
    consume(Token::RPAREN);
    consume(Token::SEMICOLON);
}
void Parser::parseFunctionDefinition() {
    Debugger::log("Parsing function definition");
    consume(Token::FUNCTION);
    if (currentToken.type != Token::VAR)
        ErrorHandler::throwError("Expected function name");
    std::string funcName = currentToken.text;
    consume(Token::VAR);
    consume(Token::LPAREN);
    std::vector<std::string> params;
    if (currentToken.type != Token::RPAREN) {
        while (true) {
            if (currentToken.type != Token::VAR)
                ErrorHandler::throwError("Expected parameter name");
            params.push_back(currentToken.text);
            consume(Token::VAR);
            if (currentToken.type == Token::COMMA)
                consume(Token::COMMA);
            else
                break;
        }
    }
    functions[funcName] = {lexer.getPosition(), params};
    consume(Token::RPAREN);
    Debugger::log("Stored function '" + funcName + "' at position " + std::to_string(lexer.getPosition()));
    consume(Token::LBRACE);
    int braceCount = 1;
    while (braceCount > 0) {
        Token tok = lexer.nextToken();
        if (tok.type == Token::LBRACE) braceCount++;
        else if (tok.type == Token::RBRACE) braceCount--;
    }
    currentToken = lexer.nextToken();
}
void Parser::parse() {
    while (currentToken.type != Token::END) {
        if (currentToken.type == Token::RBRACE) {
            Debugger::log("Skipping RBRACE at top level");
            currentToken = lexer.nextToken();
            continue;
        }
        if (currentToken.type == Token::FUNCTION) {
            Debugger::log("Skipping over function definition at top level");
            parseFunctionDefinition();
            continue;
        }
        if (currentToken.type == Token::SEMICOLON) {
            currentToken = lexer.nextToken();
            continue;
        }
        statement();
    }
}