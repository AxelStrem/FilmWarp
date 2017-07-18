#include "stdafx.h"
#include "Video.h"

using namespace std;
using namespace cv;

Color32 operator*(float f, Color8 c)
{
    return Color32{ f*c.r, f*c.g, f*c.b };
}

Color32 operator*(float f, Color32 c)
{
    return Color32{ f*c.r, f*c.g, f*c.b };
}

Color32 operator+(Color32 c1, Color32 c2)
{
    return Color32{ c1.r + c2.r,c1.g + c2.g, c1.b + c2.b };
}

Color8 compress(Color32 c)
{
    return Color8{ static_cast<unsigned char>(clamp(static_cast<int>(c.r),0,255)),
        static_cast<unsigned char>(clamp(static_cast<int>(c.g),0,255)),
        static_cast<unsigned char>(clamp(static_cast<int>(c.b),0,255)) };
}

Color8 compress(Color8 c)
{
    return c;
}

void Video::rewind()
{
    source.release();
    source.open(file);
    if (!source.isOpened())
        throw IOError{ "Could not rewind input file" };
    current_frame = 0;
}

void Video::readFrame()
{
    source >> cached_frames[current_frame++];
    if (cached_frames[current_frame - 1].empty())
    {
        frame_count = std::min(frame_count,current_frame-1);
    }
}

bool Video::isCached(int frame)
{
    return cached_frames.find(frame) != cached_frames.end();
}

void Video::skipFrame()
{
    source.grab();
    current_frame++;
}

Video::Video(std::string filename) : source(filename), file(filename), current_frame(0)
{
    if (!source.isOpened())
        throw IOError{ "Could not open input file" };

    resolution = Size(static_cast<int>(source.get(CAP_PROP_FRAME_WIDTH)),
        static_cast<int>(source.get(CAP_PROP_FRAME_HEIGHT)));

    source_fps = (source.get(CAP_PROP_FPS));
    frame_count = static_cast<int>(source.get(CAP_PROP_FRAME_COUNT));
    maxframes = frame_count;

    codec_fourcc = static_cast<int>(source.get(CAP_PROP_FOURCC));
}

void Video::loadFrame(int frame)
{
    if (isCached(frame))
        return;

    if (current_frame > frame)
        rewind();

    while (current_frame < frame)
        skipFrame();

    readFrame();
}

void Video::loadFrame(int from, int to)
{
    while ((from < to) && (isCached(from)))
        from++;

    if (from == to)
        return;

    if (from < current_frame)
        rewind();

    while (current_frame < from)
        skipFrame();

    while (current_frame <= to)
        readFrame();
}

void Video::keepFrames(int from, int to)
{
    for (int f = 0; f < from; f++)
    {
        cached_frames.erase(f);
    }

    for (int f = to; f < frame_count; f++)
    {
        cached_frames.erase(f);
    }
}

void Video::setMaxFrames(int mf)
{
    maxframes = mf;
}

cv::Mat Video::getFrame(int frame)
{
    return cached_frames[frame];
}

Color8 Video::pixel(int x, int y, int frame)
{
    auto &f = cached_frames[frame];
    unsigned char* ptr = f.data + f.step[0] * y + f.step[1] * x;
    return Color8{ ptr[0], ptr[1], ptr[2] };
}

Color32 Video::pixel(float x, int y, int frame)
{
    int x1 = static_cast<int>(x);
    int x2 = min(x1 + 1, resolution.width - 1);
    x -= x1;

    Color8 c1 = pixel(x1, y, frame);
    Color8 c2 = pixel(x2, y, frame);

    return x*c2 + (1.f - x)*c1;
}

Color32 Video::pixel(float x, float y, int frame)
{
    int y1 = static_cast<int>(y);
    int y2 = min(y1 + 1, resolution.height - 1);
    y -= y1;

    Color32 c1 = pixel(x, y1, frame);
    Color32 c2 = pixel(x, y2, frame);

    return y*c2 + (1.f - y)*c1;
}

Color32 Video::pixel(float x, float y, float frame)
{
    int f1 = static_cast<int>(frame);
    int f2 = min(f1 + 1, frame_count - 1);
    float f = static_cast<float>(frame - f1);

    Color32 c1 = pixel(x, y, f1);
    Color32 c2 = pixel(x, y, f2);

    return f*c2 + (1.f - f)*c1;
}

Color32 Video::pixel(int x, int y, float frame)
{
    int f1 = static_cast<int>(frame);
    int f2 = min(f1 + 1, frame_count - 1);
    float f = static_cast<float>(frame - f1);

    Color8 c1 = pixel(x, y, f1);
    Color8 c2 = pixel(x, y, f2);

    return f*c2 + (1.f - f)*c1;
}
