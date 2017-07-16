#include "stdafx.h"
#include "Expression3V.h"
#include "Video.h"
#include "StringParser.h"

using namespace std;
using namespace cv;

template<class T> std::vector<T> evaluate(std::unique_ptr<Expression3V>& pExpr)
{
    return pExpr->evaluateI();
}

template<> std::vector<float> evaluate(std::unique_ptr<Expression3V>& pExpr)
{
    return pExpr->evaluateF();
}

template<class XT, class YT, class ZT>
void process3(Video& input, VideoWriter& dest, std::array<std::unique_ptr<Expression3V>, 3>& coord_exprs)
{
    Mat frame;
    input.getFrame(0).copyTo(frame);

    int pixel_amount = input.width() * input.height();
    std::vector<int> coord_x(pixel_amount), coord_y(pixel_amount);
    std::vector<float> coord_xf(pixel_amount), coord_yf(pixel_amount);

    for (int i = 0; i < input.height(); i++)
        for (int j = 0; j < input.width(); j++)
        {
            coord_x[i*input.width() + j] = j;
            coord_y[i*input.width() + j] = i;
            coord_xf[i*input.width() + j] = j;
            coord_yf[i*input.width() + j] = i;
        }

    for (auto &expr : coord_exprs)
    {
        expr->setVars(&coord_x, &coord_y);
        expr->setVars(&coord_xf, &coord_yf);
    }

    for (int f = 0; f < input.framecount(); f++)
    {
        float ft = f;
        for (auto &expr : coord_exprs)
        {
            expr->setZ(f);
            expr->setZ(ft);
        }

        std::vector<std::common_type<XT, YT>::type> xvals = evaluate<std::common_type<XT, YT>::type>(coord_exprs[0]);
        std::vector<std::common_type<XT, YT>::type> yvals = evaluate<std::common_type<XT, YT>::type>(coord_exprs[1]);
        std::vector<ZT>                             zvals = evaluate<ZT>(coord_exprs[2]);

        int offset = 0;

        for (int i = 0; i < input.height(); ++i)
            for (int j = 0; j < input.width(); ++j)
            {
                unsigned char* ptr = frame.data + frame.step[0] * i + frame.step[1] * j;
                Color8 c = compress(input.pixel(xvals[offset], yvals[offset], zvals[offset]));
                offset++;
                ptr[0] = c.r;
                ptr[1] = c.g;
                ptr[2] = c.b;
            }

        dest << frame;
    }
}

template<class XT, class YT>
void process2(Video& input, VideoWriter& dest, std::array<std::unique_ptr<Expression3V>, 3>& coord_exprs)
{
    if (coord_exprs[2]->isPrecise())
    {
        process3<XT, YT, int>(input, dest, coord_exprs);
    }
    else
    {
        process3<XT, YT, float>(input, dest, coord_exprs);
    }
}

template<class XT>
void process1(Video& input, VideoWriter& dest, std::array<std::unique_ptr<Expression3V>, 3>& coord_exprs)
{
    if (coord_exprs[1]->isPrecise())
    {
        process2<XT, int>(input, dest, coord_exprs);
    }
    else
    {
        process2<XT, float>(input, dest, coord_exprs);
    }
}

void process(Video& input, VideoWriter& dest, std::array<std::unique_ptr<Expression3V>, 3>& coord_exprs)
{
    if (coord_exprs[0]->isPrecise())
    {
        process1<int>(input, dest, coord_exprs);
    }
    else
    {
        process1<float>(input, dest, coord_exprs);
    }
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

    std::array<std::unique_ptr<Expression3V>, 3> coord_exprs = parseExprTriplet(expression);

    input.loadFrame(0, input.framecount());

    auto x_clamp = make_unique<EClampI>(0, input.width()-1);
    auto y_clamp = make_unique<EClampI>(0, input.height()-1);
    auto z_clamp = make_unique<EClampI>(0, input.framecount()-1);

    x_clamp->addChild(move(coord_exprs[0]));
    y_clamp->addChild(move(coord_exprs[1]));
    z_clamp->addChild(move(coord_exprs[2]));

    coord_exprs[0] = move(x_clamp);
    coord_exprs[1] = move(y_clamp);
    coord_exprs[2] = move(z_clamp);

    process(input, dest, coord_exprs);

    return 0;
}