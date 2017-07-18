#pragma once

#include "Expression3V.h"

struct ParseError
{
    std::string message;
};

class StringParser
{
    int h;
    int w;
    int l;

    std::unique_ptr<Expression3V> readBrackets(std::string& expr);
    std::unique_ptr<Expression3V> readNumber(std::string& expr);
    std::unique_ptr<Expression3V> readTerm(std::string& expr);
    int operatorPriority(char c);
    std::unique_ptr<Expression3V> formBinaryOp(char op, std::unique_ptr<Expression3V> c1, std::unique_ptr<Expression3V> c2);
    std::unique_ptr<Expression3V> parseExpressionRanked(std::string &expr, int priority);

public:
    std::unique_ptr<Expression3V> parseExpression(std::string expr);
    std::array<std::unique_ptr<Expression3V>, 3> parseExprTriplet(std::string expr);
    void setConsts(int w_, int h_, int l_);
};