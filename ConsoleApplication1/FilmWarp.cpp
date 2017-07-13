#include "stdafx.h"

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
    stringstream conv;
    const string sourceReference = argv[1];
    const string destReference = argv[2];
    const string expression = argv[3];

    VideoCapture source(sourceReference);
    if (!source.isOpened())
    {
        cout << "Could not open input file" << endl;
        return -1;
    }

    Size res = Size(static_cast<int>(source.get(CAP_PROP_FRAME_WIDTH)),
        static_cast<int>(source.get(CAP_PROP_FRAME_HEIGHT)));

    double fps = (source.get(CAP_PROP_FPS));
    auto frame_count = static_cast<long long>(source.get(CAP_PROP_FRAME_COUNT));

    int ex = static_cast<int>(source.get(CAP_PROP_FOURCC));

    VideoWriter dest(destReference, ex, fps, res, true);

    if (!dest.isOpened())
    {
        cout << "Could not open output file" << endl;
        return -1;
    }

    Mat frame, result;
    vector<Mat> spl;

    for (long long f = 0; f < frame_count; f++)
    {
        source >> frame;              
           
        split(frame, spl);               
        std::swap(spl[0], spl[2]);
        merge(spl, result);

        dest << result;
    }

    return 0;
}