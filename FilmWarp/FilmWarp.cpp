#include "stdafx.h"

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
    stringstream conv;
    const string sourceReference = argv[1];
    const string destReference = argv[2];
    const string expression = argv[3];

    const int RS_DEL = 20;

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

    vector<Mat> qm;
    for (int k = 0; k < res.height/RS_DEL; k++)
        qm.push_back(frame.clone());

    int s1 = frame.step[0];
    int s2 = frame.step[1];

    int cf = 0;

    for (long long f = 1; f < frame_count; f++)
    {
        if (f > limit)
        {
            limit += chunk;
            cout << '*';
        }
        source >> qm[cf];  
        if (qm[cf].empty()) break;

        if (--cf < 0)
            cf += qm.size();

        for (int i = 0; i < res.height; i += RS_DEL)
        {
            int f1 = (cf + i / RS_DEL) % qm.size();
            int f2 = (cf + 1 + i / RS_DEL) % qm.size();
            for (int j = 0; j < RS_DEL; j++)
            {
                float alpha = static_cast<float>(j) / RS_DEL;
                float beta = 1.f - alpha;

                result.row(i + j) = beta * qm[f1].row(i + j) + alpha * qm[f2].row(i + j);
            }
        }

        dest << result;
    }

    return 0;
}