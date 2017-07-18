#pragma once

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
Color8 compress(Color8 c);

class Video
{
    cv::VideoCapture                 source;
    std::unordered_map<int, cv::Mat> cached_frames;

    cv::Size resolution;
    double source_fps;
    int frame_count;
    int codec_fourcc;

    int current_frame;
    std::string file;

    void rewind();
    void readFrame();
    bool isCached(int frame);
    void skipFrame();
public:
    Video(std::string filename);

    void loadFrame(int frame);
    void loadFrame(int from, int to);

    void keepFrames(int from, int to);

    cv::Mat getFrame(int frame);

    int width() { return resolution.width; }
    int height() { return resolution.height; }
    double fps() { return source_fps; }
    int framecount() { return frame_count; }
    int fourcc() { return codec_fourcc; }

    Color8 pixel(int x, int y, int frame);
    Color32 pixel(float x, int y, int frame);
    Color32 pixel(float x, float y, int frame);
    Color32 pixel(float x, float y, float frame);
    Color32 pixel(int x, int y, float frame);
};