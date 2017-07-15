#include "stdafx.h"

using namespace std;
using namespace cv;

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

    void rewind()
    {
        source.release();
        source.open(file);
        if (!source.isOpened())
            throw IOError{ "Could not rewind input file" };
        current_frame = 0;
    }

    void readFrame()
    {
        source >> cached_frames[current_frame++];
    }

    bool isCached(long long frame)
    {
        return cached_frames.find(frame) != cached_frames.end();
    }

    void skipFrame()
    {
        source.grab();
        current_frame++;
    }
public:
    Video(std::string filename) : source(filename), file(filename), current_frame(0)
    {
        if (!source.isOpened())
            throw IOError{ "Could not open input file" };

        resolution = Size(static_cast<int>(source.get(CAP_PROP_FRAME_WIDTH)),
            static_cast<int>(source.get(CAP_PROP_FRAME_HEIGHT)));

        source_fps = (source.get(CAP_PROP_FPS));
        frame_count = static_cast<long long>(source.get(CAP_PROP_FRAME_COUNT));

        codec_fourcc = static_cast<int>(source.get(CAP_PROP_FOURCC));
    }

    void loadFrame(long long frame)
    {
        if (isCached(frame))
            return;
        
        if (current_frame > frame)
            rewind();

        while (current_frame < frame)
            skipFrame();

        readFrame();
    }

    void loadFrame(long long from, long long to)
    {
        while ((from < to) && (isCached(from)))
            from++;

        if (from == to)
            return;

        if (from < current_frame)
            rewind();

        while (current_frame < from)
            skipFrame();

        while(current_frame < to)
            readFrame();
    }

    cv::Mat getFrame(long long frame)
    {
        return cached_frames[frame];
    }

    int width() { return resolution.width; }
    int height() { return resolution.height; }
    double fps() { return source_fps; }
    long long framecount() { return frame_count; }
    int fourcc() { return codec_fourcc; }

    Color8 pixel(int x, int y, long long frame)
    {
        auto &f = cached_frames[frame];
        unsigned char* ptr = f.data + f.step[0]*y + f.step[1]*x;
        return Color8{ ptr[0], ptr[1], ptr[2] };
    }

    Color32 pixel(float x, int y, long long frame)
    {
        int x1 = static_cast<int>(x);
        int x2 = min(x1 + 1, resolution.width - 1);
        x -= x1;

        Color8 c1 = pixel(x1, y, frame);
        Color8 c2 = pixel(x2, y, frame);

        return x*c2 + (1.f-x)*c1;
    }

    Color32 pixel(float x, float y, long long frame)
    {
        int y1 = static_cast<int>(y);
        int y2 = min(y1 + 1, resolution.height - 1);
        y -= y1;

        Color32 c1 = pixel(x, y1, frame);
        Color32 c2 = pixel(x, y2, frame);

        return y*c2 + (1.f - y)*c1;
    }

    Color32 pixel(float x, float y, double frame)
    {
        long long f1 = static_cast<long long>(frame);
        long long f2 = min(f1 + 1, frame_count - 1);
        float f = static_cast<float>(frame - f1);

        Color32 c1 = pixel(x, y, f1);
        Color32 c2 = pixel(x, y, f2);

        return f*c2 + (1.f - f)*c1;
    }

    Color32 pixel(int x, int y, double frame)
    {
        long long f1 = static_cast<long long>(frame);
        long long f2 = min(f1 + 1, frame_count - 1);
        float f = static_cast<float>(frame - f1);

        Color8 c1 = pixel(x, y, f1);
        Color8 c2 = pixel(x, y, f2);

        return f*c2 + (1.f - f)*c1;
    }
};

class Expression3V
{
protected:
    std::vector<float>* xf;
    std::vector<float>* yf;
    std::vector<int>*   xi;
    std::vector<int>*   yi;
    double              zf;
    long long           zi;

    std::vector<Expression3V*> pChildren;

public:
    Expression3V() : xf(nullptr), yf(nullptr), xi(nullptr), yi(nullptr), zf(0.0), zi(0) {}
    virtual bool isPrecise() const { return true; }
    
    void setFVars(std::vector<float>* xf_, std::vector<float>* yf_) 
    { 
        xf = xf_; yf = yf_;
        for (auto p : pChildren)
            p->setFVars(xf_, yf_);
    }

    void setIVars(std::vector<int>* xi_, std::vector<int>* yi_)
    {
        xi = xi_; yi = yi_;
        for (auto p : pChildren)
            p->setIVars(xi_, yi_);
    }

    void setFZ(double zf_)
    {
        zf = zf_;
        for (auto p : pChildren)
            p->setFZ(zf_);
    }

    void setIZ(long long zi_)
    {
        zi = zi_;
        for (auto p : pChildren)
            p->setIZ(zi_);
    }

    virtual std::vector<float> evaluateF() { return std::vector<float>(xf->size()); }
    virtual std::vector<int> evaluateI() { return std::vector<int>(xi->size()); }
};

class EVarX : public Expression3V
{
public:
    virtual bool isPrecise() const { return true; }
  
    virtual std::vector<float> evaluateF() { return *xf; }
    virtual std::vector<int> evaluateI() { return *xi; }
};

class EVarY : public Expression3V
{
public:
    virtual bool isPrecise() const { return true; }

    virtual std::vector<float> evaluateF() { return *yf; }
    virtual std::vector<int> evaluateI() { return *yi; }
};

class EVarZ : public Expression3V
{
public:
    virtual bool isPrecise() const { return true; }

    virtual std::vector<float> evaluateF() { return std::vector<float>(xf->size(), zf); }
    virtual std::vector<int> evaluateI() { return std::vector<int>(xi->size(), zi); }
};

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

    std::array<std::unique_ptr<Expression3V>, 3> coord_exprs{ make_unique<EVarX>(), make_unique<EVarY>(), make_unique<EVarZ>() };

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