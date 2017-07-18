#pragma once

class Recorder
{
    cv::Size resolution;
    double target_fps;
    int frame_count;
    int codec_fourcc;

public:
    Recorder(int fourcc, double fps, cv::Size res, int target_framecount);

    virtual void pushFrame(cv::Mat& frame) = 0;
    virtual cv::Mat getSampleFrame() = 0;

    int width() { return resolution.width; }
    int height() { return resolution.height; }
    double fps() { return target_fps; }
    int framecount() { return frame_count; }
    int fourcc() { return codec_fourcc; }

    virtual ~Recorder() {}
};

class VideoRecorder : public Recorder
{
    cv::VideoWriter dest;
public:
    VideoRecorder(std::string filename, int fourcc, double fps, cv::Size res, int target_framecount);
    virtual void pushFrame(cv::Mat& frame);
    virtual cv::Mat getSampleFrame();
};

class ImageRecorder : public Recorder
{
    cv::Mat data;
    std::string fname;
public:
    ImageRecorder(std::string filename, cv::Size res);

    virtual void pushFrame(cv::Mat& frame);
    virtual cv::Mat getSampleFrame();
    virtual ~ImageRecorder();
};