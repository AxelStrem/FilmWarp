#include "stdafx.h"
#include "Expression3V.h"
#include "Video.h"

using namespace std;
using namespace cv;

struct ParseError
{
    std::string message;
};

std::unique_ptr<Expression3V> parseExpression(std::string expr);

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

    if ((expr[0] == 'z')||(expr[0]=='t'))
    {
        expr = expr.substr(1);
        return make_unique<EVarZ>();
    }

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
    case '^': return 3;
    }
    return -1;
}

std::unique_ptr<Expression3V> parseExpressionRanked(std::string &expr, int priority)
{
    std::unique_ptr<Expression3V> output;

    output = (priority<3)?parseExpressionRanked(expr,priority+1):readTerm(expr);

    if (expr.empty()||(operatorPriority(expr[0])<priority))
        return move(output);

    std::unique_ptr<Expression3V> tmp = move(output);
    
    switch (priority)
    {
    case 1:  output = make_unique<ESum>(); break;
    case 2:  output = make_unique<ESum>(); break;
    case 3:  output = make_unique<ESum>(); break;
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

int main(int argc, char *argv[])
{
    stringstream conv;
    const string sourceReference = argv[1];
    const string destReference = argv[2];
    const string expression = argv[3];

    Video input(sourceReference);
    
    
    
    VideoWriter dest(destReference, input.fourcc(), input.fps(), cv::Size(input.width(), input.height()), true);
    if (!dest.isOpened())
    {
        cout << "Could not open output file" << endl;
        return -1;
    }

    int pixel_amount = input.width() * input.height();
    std::vector<int> coord_x(pixel_amount), coord_y(pixel_amount);

    for (int i = 0; i < input.height(); i++)
        for (int j = 0; j < input.width(); j++)
        {
            coord_x[i*input.width() + j] = j;
            coord_y[i*input.width() + j] = i;
        }


    auto tst = parseExpression(expression);

    std::array<std::unique_ptr<Expression3V>, 3> coord_exprs{ move(tst), make_unique<EVarY>(), make_unique<EVarZ>() };

    auto x_clamp = make_unique<EClampI>(0, input.width()-1);
    auto y_clamp = make_unique<EClampI>(0, input.height()-1);
    auto z_clamp = make_unique<EClampI>(0, input.framecount()-1);

    x_clamp->addChild(move(coord_exprs[0]));
    y_clamp->addChild(move(coord_exprs[1]));
    z_clamp->addChild(move(coord_exprs[2]));

    coord_exprs[0] = move(x_clamp);
    coord_exprs[1] = move(y_clamp);
    coord_exprs[2] = move(z_clamp);


    input.loadFrame(0, input.framecount());

    Mat frame;
    input.getFrame(0).copyTo(frame);

    for(auto &expr : coord_exprs)
    {
        expr->setIVars(&coord_x, &coord_y);
    }

    for (long long f = 0; f < input.framecount(); f++)
    {
        for (auto &expr : coord_exprs)
        {
            expr->setIZ(f);
        }

        std::vector<int> xvals = coord_exprs[0]->evaluateI();
        std::vector<int> yvals = coord_exprs[1]->evaluateI();
        std::vector<int> zvals = coord_exprs[2]->evaluateI();

        int offset = 0;

        for (int i = 0; i < input.height(); ++i)
            for (int j = 0; j < input.width(); ++j)
            {
                unsigned char* ptr = frame.data + frame.step[0] * i + frame.step[1] * j;
                Color8 c = input.pixel(xvals[offset], yvals[offset], static_cast<long long>(zvals[offset]));
                offset++;
                ptr[0] = c.r;
                ptr[1] = c.g;
                ptr[2] = c.b;
            }

        dest << frame;
    }

    return 0;
}