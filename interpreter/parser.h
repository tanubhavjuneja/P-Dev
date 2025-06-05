#pragma once
#include "lexer.h"
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <stack>
#include <functional>
#include <unordered_map>
using Value = std::variant<int, std::string>;
class Parser {
public:
    enum class UpdateOp {
        NONE,
        INCREMENT,
        DECREMENT,
        ASSIGN
    };
    struct ForUpdateInfo {
        UpdateOp op = UpdateOp::NONE;
        std::string varName;
        Value assignedValue; 
        bool hasAssignedValue = false;
    };
    Parser(Lexer lexer);
    void parse();
    void statement();
    bool parseCondition();
    void parseIfStatement();
    void parseVarOrFunctionCall();
    int parseFunctionCallArgsAndExecute(const std::string& funcName);
    void parseWriteStatement();
    void parseFunctionDefinition();
    void parseReturnStatement();
    void parseForStatement();
    void parseDoStatement();
    void parseWhileStatement();
    ForUpdateInfo parseForUpdateExpression();
    void parseAssignmentExpression();
    bool loopBreak = false;
    bool loopContinue = false;
    Value returnValue;
    bool hasReturnValue = false;
    int lastReturnValue = 0;
    std::string forUpdateVarName;
    std::function<Value()> forUpdateExprFunc = nullptr;
    std::vector<std::unordered_map<std::string, Value>> variableStack;
    std::vector<Value> parseFunctionArguments(const std::unordered_map<std::string, Value>& currentScope);
private:
    struct FunctionInfo {
        size_t position;
        std::vector<std::string> params;
    };
    void pushScope();
    void popScope();
    Value lookupVariableValue(const std::string& name);  
    void executeBlock();  
    void skipBlock();
    void skipRemainingElifElseBlocks();
    void setVariableValue(const std::string& name, Value value);
    std::map<std::string, FunctionInfo> functions;
    std::stack<std::pair<size_t, Token>> returnStates;
    Lexer lexer;
    Token currentToken;
    void consume(Token::Type expected);
    int expr();
    int term();
    int factor();
};