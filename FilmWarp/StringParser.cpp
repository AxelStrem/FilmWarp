#include "stdafx.h"
#include "StringParser.h"

using namespace std;
using namespace cv;


std::unique_ptr<Expression3V> readBrackets(std::string& expr)
{
    int cb = 1;
    int bcount = 1;
    do
    {
        if (cb >= expr.size())
        {
            throw ParseError{ "Parsing error: ill-formed bracket structure" };
        }
        if (expr[cb] == '(')
            bcount++;
        if (expr[cb] == ')')
            bcount--;
        cb++;
    } while (bcount != 0);
    string in_brackets = expr.substr(1, cb - 2);
    expr = expr.substr(cb);
    return parseExpression(in_brackets);
}

std::unique_ptr<Expression3V> readNumber(std::string& expr)
{
    int num = 0;
    while (expr[0] >= '0'&& expr[0] <= '9')
    {
        num = num * 10 + (expr[0] - '0');
        expr = expr.substr(1);
    }

    if (expr[0] != '.')
        return make_unique<EConstI>(num);

    expr = expr.substr(1);

    float fract = 1.f;
    while (expr[0] >= '0'&& expr[0] <= '9')
    {
        num = num * 10 + (expr[0] - '0');
        fract *= 10.f;
        expr = expr.substr(1);
    }

    return make_unique<EConstF>(num / fract);
}

std::unique_ptr<Expression3V> readTerm(std::string& expr)
{
    if (expr[0] == '(')
        return readBrackets(expr);

    if (expr[0] == '+')
    {
        expr = expr.substr(1);
        return readTerm(expr);
    }

    if (expr[0] == '-')
    {
        expr = expr.substr(1);
        unique_ptr<Expression3V> ptr = make_unique<EScaleI>(-1);
        ptr->addChild(readTerm(expr));
        return move(ptr);
    }

    if (expr[0] == 'x')
    {
        expr = expr.substr(1);
        return make_unique<EVarX>();
    }

    if (expr[0] == 'y')
    {
        expr = expr.substr(1);
        return make_unique<EVarY>();
    }

    if ((expr[0] == 'z') || (expr[0] == 't'))
    {
        expr = expr.substr(1);
        return make_unique<EVarZ>();
    }

    if ((expr[0] >= '0') && (expr[0] <= '9'))
        return readNumber(expr);

    throw ParseError{ "Parsing error: unknown symbol" };
}


std::unique_ptr<Expression3V> readOperator(std::string& expr)
{
    if (expr[0] == '+')
    {
        expr = expr.substr(1);
        return make_unique<ESum>();
    }
}

int operatorPriority(char c)
{
    switch (c)
    {
    case '+': return 1;
    case '-': return 1;
    case '*': return 2;
    case '/': return 2;
    case '%': return 2;
    case '^': return 3;
    }
    return -1;
}

std::unique_ptr<Expression3V> parseExpressionRanked(std::string &expr, int priority)
{
    std::unique_ptr<Expression3V> output;

    output = (priority<3) ? parseExpressionRanked(expr, priority + 1) : readTerm(expr);

    if (expr.empty() || (operatorPriority(expr[0])<priority))
        return move(output);

    std::unique_ptr<Expression3V> tmp = move(output);

    switch (priority)
    {
    case 1:  output = make_unique<ESum>(); break;
    case 2:  output = make_unique<EMult>(); break;
    case 3:  output = make_unique<EMult>(); break;
    }

    output->addChild(move(tmp));

    while (!expr.empty() && (operatorPriority(expr[0]) == priority))
    {
        if (expr[0] != '-')
            expr = expr.substr(1);

        output->addChild(parseExpressionRanked(expr, priority + 1));
    }

    return output;
}

std::unique_ptr<Expression3V> parseExpression(std::string expr)
{
    return parseExpressionRanked(expr, 1);
}

std::array<std::unique_ptr<Expression3V>, 3> parseExprTriplet(std::string expr)
{
    if ((expr[0] != '[') || (expr.back() != ']'))
        throw ParseError{ "Parsing error: ill-formed expression" };

    int f1 = expr.find(';');
    int f2 = expr.find(';', f1 + 1);

    std::array<std::unique_ptr<Expression3V>, 3> result;

    expr.pop_back();

    result[0] = parseExpression(expr.substr(1, f1 - 1));
    result[1] = parseExpression(expr.substr(f1 + 1, f2 - f1 - 1));
    result[2] = parseExpression(expr.substr(f2 + 1));

    return result;
}