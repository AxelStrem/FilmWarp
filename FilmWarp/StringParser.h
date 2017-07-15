#pragma once

#include "Expression3V.h"

struct ParseError
{
    std::string message;
};

std::unique_ptr<Expression3V> parseExpression(std::string expr);
std::array<std::unique_ptr<Expression3V>, 3> parseExprTriplet(std::string expr);