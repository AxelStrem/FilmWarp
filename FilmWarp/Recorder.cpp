#include "stdafx.h"
#include "Recorder.h"

using namespace cv;
using namespace std;

Recorder::Recorder(int fourcc, double fps, cv::Size res, int target_framecount)
    : resolution(res), target_fps(fps), frame_count(target_framecount), codec_fourcc(fourcc)
{}

VideoRecorder::VideoRecorder(std::string filename, int fourcc, double fps, cv::Size res, int target_framecount)
    : Recorder(fourcc, fps, res, target_framecount), dest(filename, fourcc, fps, res, true)
{
    if (!dest.isOpened())
    {
        throw IOError{ "Could not open output file" };
    }
}

void VideoRecorder::pushFrame(cv::Mat & frame)
{
    dest << frame;
}

cv::Mat VideoRecorder::getSampleFrame()
{
    return cv::Mat(cv::Size(width(), height()), CV_8UC3);
}

ImageRecorder::ImageRecorder(std::string filename, cv::Size res)
    : Recorder(0, 0.0, res, 1), fname(filename)
{

}

void ImageRecorder::pushFrame(cv::Mat & frame)
{
    frame.copyTo(data);
}

cv::Mat ImageRecorder::getSampleFrame()
{
    return cv::Mat(cv::Size(width(), height()), CV_8UC3);
}

ImageRecorder::~ImageRecorder()
{
    cv::InputArray res(data);
    cv::imwrite(fname, res);
}
