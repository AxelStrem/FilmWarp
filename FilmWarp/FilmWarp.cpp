#include "stdafx.h"
#include "Expression3V.h"
#include "Video.h"

using namespace std;
using namespace cv;


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

    std::array<std::unique_ptr<Expression3V>, 3> coord_exprs{ make_unique<EVarY>(), make_unique<EVarY>(), make_unique<EVarZ>() };

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