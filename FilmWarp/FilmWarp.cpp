#include "stdafx.h"

using namespace std;
using namespace cv;
using namespace boost;

int main(int argc, char *argv[])
{
    const string sourceReference = argv[1];   // Input file name - arg 1
    const string destReference = argv[2];     // Output file name - arg 2
    //const string expression = argv[3];        // arg 3 is ignored for now

    const int RS_DEL = 20; // Number of pixel rows to lag by 1 frame  

    /* Open source file & get some properties */
    VideoCapture source(sourceReference);
    if (!source.isOpened())
    {
        cout << "Could not open input file" << endl;
        return -1;
    }

    auto resolution = Size(static_cast<int>(source.get(CAP_PROP_FRAME_WIDTH)),
                           static_cast<int>(source.get(CAP_PROP_FRAME_HEIGHT)));

    auto fps = source.get(CAP_PROP_FPS);
    auto frame_count = static_cast<long long>(source.get(CAP_PROP_FRAME_COUNT));

    int ex = static_cast<int>(source.get(CAP_PROP_FOURCC));

    /* Create output file with same resolution, fps & codec */
    VideoWriter dest(destReference, ex, fps, resolution, true);

    if (!dest.isOpened())
    {
        cout << "Could not open output file" << endl;
        return -1;
    }

    /* Use first frame as a baseline */
    Mat frame;
    source >> frame;

    /* Prepare a container for cached frames */
    circular_buffer<Mat> cached_frames(resolution.height / RS_DEL + 1);
    for (int k = 0; k < resolution.height/RS_DEL; k++)
        cached_frames.push_back(frame.clone());

    Mat result = frame.clone();

    // poor man's progress bar
    long long progress_chunk = frame_count / 25;
    long long progress_limit = progress_chunk;

    for (long long current_frame = 1; current_frame < frame_count; ++current_frame)
    {
        if (current_frame > progress_limit)
        {
            progress_limit += progress_chunk;
            cout << '*';
        }

        // read one frame into the circular buffer
        source >> frame;  

        if (frame.empty()) break;

        cached_frames.push_front(frame.clone());

        /*
            the actual work:
            each row is calculated as a linear combination of the corresponding rows of 2 frames 
        */
        for (int i = 0; i < resolution.height; i += RS_DEL)
        {
            for (int j = 0; j < RS_DEL; j++)
            {
                float alpha = static_cast<float>(j) / RS_DEL;
                float beta = 1.f - alpha;

                result.row(i + j) = beta * cached_frames[i/RS_DEL].row(i + j) + alpha * cached_frames[i/RS_DEL + 1].row(i + j);
            }
        }

        // save the result & slide the buffer
        dest << result;
        cached_frames.pop_back();
    }

    return 0;
}