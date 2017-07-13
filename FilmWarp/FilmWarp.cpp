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

    Mat frame;
    vector<Mat> spl;

    long long chunk = frame_count / 25;
    long long limit = chunk;

    cout << endl;

    source >> frame;

    Mat result = frame.clone();
    dest << result;

    queue<Mat> qm;
    for (int k = 0; k < res.height; k++)
        qm.push(frame.clone());

    int s1 = frame.step[0];
    int s2 = frame.step[1];


    for (long long f = 1; f < frame_count; f++)
    {
        if (f > limit)
        {
            limit += chunk;
            cout << '*';
        }
        source >> frame;  
        if (frame.empty()) break;

        for (int i = 0; i < 20000; i++)
        {
            int x = rand() % (res.width);
            int y = rand() % (res.height);

            unsigned char* s = frame.data + frame.step[0] * y + frame.step[1] * x;
            unsigned char* d = result.data + result.step[0] * y + result.step[1] * x;

            int k = rand() % 3;
            {
                d[k] = s[k];
            }
        }

        dest << result;
    }

    return 0;
}