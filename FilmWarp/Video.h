#pragma once

struct IOError
{
    std::string message;
};

struct Color8
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

struct Color32
{
    float r;
    float g;
    float b;
};

Color32 operator*(float f, Color8 c);
Color32 operator*(float f, Color32 c);
Color32 operator+(Color32 c1, Color32 c2);
Color8 compress(Color32 c);

class Video
{
    cv::VideoCapture                 source;
    std::unordered_map<long long, cv::Mat> cached_frames;

    cv::Size resolution;
    double source_fps;
    long long frame_count;
    int codec_fourcc;

    long long current_frame;
    std::string file;

    void rewind();
    void readFrame();
    bool isCached(long long frame);
    void skipFrame();
public:
    Video(std::string filename);

    void loadFrame(long long frame);
    void loadFrame(long long from, long long to);

    cv::Mat getFrame(long long frame);

    int width() { return resolution.width; }
    int height() { return resolution.height; }
    double fps() { return source_fps; }
    long long framecount() { return frame_count; }
    int fourcc() { return codec_fourcc; }

    Color8 pixel(int x, int y, long long frame);
    Color32 pixel(float x, int y, long long frame);
    Color32 pixel(float x, float y, long long frame);
    Color32 pixel(float x, float y, double frame);
    Color32 pixel(int x, int y, double frame);
};