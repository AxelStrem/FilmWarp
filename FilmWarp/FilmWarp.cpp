#include "stdafx.h"
#include "Expression3V.h"
#include "Video.h"
#include "StringParser.h"

using namespace std;
using namespace cv;

class Recorder
{
    cv::Size resolution;
    double target_fps;
    int frame_count;
    int codec_fourcc;
public:
    Recorder(int fourcc, double fps, cv::Size res, int target_framecount)
        : resolution(res), target_fps(fps), frame_count(target_framecount), codec_fourcc(fourcc)
    {}

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
    VideoRecorder(std::string filename, int fourcc, double fps, cv::Size res, int target_framecount)
    : Recorder(fourcc, fps, res, target_framecount), dest(filename, fourcc, fps, res, true)
    {
        if (!dest.isOpened())
        {
            throw IOError{ "Could not open output file" };
        }
    }

    virtual void pushFrame(cv::Mat& frame)
    {
        dest << frame;
    }

    virtual cv::Mat getSampleFrame()
    {
        return cv::Mat(cv::Size(width(),height()), CV_8UC3);
    }
};

class ImageRecorder : public Recorder
{
    cv::Mat data;
    std::string fname;
public:
    ImageRecorder(std::string filename, cv::Size res)
        : Recorder(0, 0.0, res, 1), fname(filename)
    {

    }

    virtual void pushFrame(cv::Mat& frame)
    {
        frame.copyTo(data);
    }

    virtual cv::Mat getSampleFrame()
    {
        return cv::Mat(cv::Size(width(), height()), CV_8UC3);
    }

    virtual ~ImageRecorder()
    {
        cv::InputArray res(data);
        cv::imwrite(fname, res);
    }
};

template<class T> std::vector<T> evaluate(std::unique_ptr<Expression3V>& pExpr)
{
    return pExpr->evaluateI();
}

template<> std::vector<float> evaluate(std::unique_ptr<Expression3V>& pExpr)
{
    return pExpr->evaluateF();
}

template<class F> void apply_result(std::unique_ptr<Expression3V>& pExpr, F func)
{
    if (pExpr->isPrecise())
    {
        func(pExpr->evaluateI());
    }
    else
    {
        func(pExpr->evaluateF());
    }
}

template<class XT, class YT, class ZT>
void process3(Video& input, Recorder& dest, std::array<std::unique_ptr<Expression3V>, 3>& coord_exprs)
{
    Mat frame = dest.getSampleFrame();

    int pixel_amount = dest.width() * dest.height();
    std::vector<int> coord_x(pixel_amount), coord_y(pixel_amount);
    std::vector<float> coord_xf(pixel_amount), coord_yf(pixel_amount);

    for (int i = 0; i < dest.height(); i++)
        for (int j = 0; j < dest.width(); j++)
        {
            coord_x[i*dest.width() + j] = j;
            coord_y[i*dest.width() + j] = i;
            coord_xf[i*dest.width() + j] = static_cast<float>(j);
            coord_yf[i*dest.width() + j] = static_cast<float>(i);
        }

    for (auto &expr : coord_exprs)
    {
        expr->setVars(&coord_x, &coord_y);
        expr->setVars(&coord_xf, &coord_yf);
    }

    Interval full_x{ 0.f, static_cast<float>(dest.width()) };
    Interval full_y{ 0.f, static_cast<float>(dest.height()) };


    const int bstep = 24;
    for (int bstart = 0, bend = min(bstep,dest.framecount()); bstart < dest.framecount(); bstart = bend, bend = min(bstart + bstep,dest.framecount()))
    {
        Interval zint{ static_cast<float>(bstart),static_cast<float>(bend) };
        auto frame_span = coord_exprs[2]->getImage(full_x, full_y, zint);

        input.loadFrame(static_cast<int>(frame_span.a), static_cast<int>(frame_span.b) + 1);

        for (int f = bstart; f < bend; f++)
        {
            float ft = static_cast<float>(f);
            for (auto &expr : coord_exprs)
            {
                expr->setZ(f);
                expr->setZ(ft);
            }

            std::vector<std::common_type<XT, YT>::type> xvals = evaluate<std::common_type<XT, YT>::type>(coord_exprs[0]);
            std::vector<std::common_type<XT, YT>::type> yvals = evaluate<std::common_type<XT, YT>::type>(coord_exprs[1]);
            std::vector<ZT>                             zvals = evaluate<ZT>(coord_exprs[2]);

            int offset = 0;

            for (int i = 0; i < dest.height(); ++i)
                for (int j = 0; j < dest.width(); ++j)
                {
                    unsigned char* ptr = frame.data + frame.step[0] * i + frame.step[1] * j;
                    Color8 c = compress(input.pixel(xvals[offset], yvals[offset], zvals[offset]));
                    offset++;
                    ptr[0] = c.r;
                    ptr[1] = c.g;
                    ptr[2] = c.b;
                }

            dest.pushFrame(frame);
        }

        zint = Interval{ static_cast<float>(bend), static_cast<float>(dest.framecount()) };
        auto frame_togo = coord_exprs[2]->getImage(full_x, full_y, zint);
        
        input.keepFrames(static_cast<int>(frame_togo.a), static_cast<int>(frame_togo.b) + 1);
    }
}

template<class XT, class YT>
void process2(Video& input, Recorder& dest, std::array<std::unique_ptr<Expression3V>, 3>& coord_exprs)
{
    if (coord_exprs[2]->isPrecise())
    {
        process3<XT, YT, int>(input, dest, coord_exprs);
    }
    else
    {
        process3<XT, YT, float>(input, dest, coord_exprs);
    }
}

template<class XT>
void process1(Video& input, Recorder& dest, std::array<std::unique_ptr<Expression3V>, 3>& coord_exprs)
{
    if (coord_exprs[1]->isPrecise())
    {
        process2<XT, int>(input, dest, coord_exprs);
    }
    else
    {
        process2<XT, float>(input, dest, coord_exprs);
    }
}

void process(Video& input, Recorder& dest, std::array<std::unique_ptr<Expression3V>, 3>& coord_exprs)
{
    if (coord_exprs[0]->isPrecise())
    {
        process1<int>(input, dest, coord_exprs);
    }
    else
    {
        process1<float>(input, dest, coord_exprs);
    }
}

int main(int argc, char *argv[])
{
    stringstream conv;
    const string sourceReference = argv[1];
    const string destReference = argv[2];
    const string expression = argv[3];

    map<string, string> params;

    for (int a = 4; a < argc; a++)
    {
        string param = argv[a];
        if (!param.empty() && param[0] == '-')
        {
            int spl = static_cast<int>(param.find('='));
            params[param.substr(1, spl - 1)] = param.substr(spl + 1);
        }
    }

    Video input(sourceReference);   

    int out_w  = input.width();
    int out_h  = input.height();
    int out_fc = input.framecount();
    double out_fps = input.fps();

    StringParser sp;
    sp.setConsts(input.width(), input.height(), input.framecount());

    if (params.find("s") != params.end())
    {
        auto sz_exprs = sp.parseExprTriplet(params["s"]);
        std::vector<int> nvec_i{ 0 };
        std::vector<float> nvec_f{ 0.f };
        for (auto& pExpr : sz_exprs)
        {
            pExpr->setVars(&nvec_i, &nvec_i);
            pExpr->setVars(&nvec_f, &nvec_f);
            pExpr->setZ(0);
            pExpr->setZ(0.f);
        }

        apply_result(sz_exprs[0], [&out_w](auto vec) { out_w = static_cast<int>(vec[0]); });
        apply_result(sz_exprs[1], [&out_h](auto vec) { out_h = static_cast<int>(vec[0]); });
        apply_result(sz_exprs[2], [&out_fc](auto vec) { out_fc = static_cast<int>(vec[0]); });
    }
    
    std::unique_ptr<Recorder> dest = (out_fc>1)
        ? std::unique_ptr<Recorder>(make_unique<VideoRecorder>(destReference, input.fourcc(), out_fps, cv::Size(out_w, out_h), out_fc))
        : std::unique_ptr<Recorder>(make_unique<ImageRecorder>(destReference, cv::Size(out_w, out_h)));
    
    std::array<std::unique_ptr<Expression3V>, 3> coord_exprs = sp.parseExprTriplet(expression);

   // input.loadFrame(0, input.framecount());

    auto x_clamp = make_unique<EClampI>(0, input.width()-1);
    auto y_clamp = make_unique<EClampI>(0, input.height()-1);
    auto z_clamp = make_unique<EClampI>(0, input.framecount()-1);

    x_clamp->addChild(move(coord_exprs[0]));
    y_clamp->addChild(move(coord_exprs[1]));
    z_clamp->addChild(move(coord_exprs[2]));

    coord_exprs[0] = move(x_clamp);
    coord_exprs[1] = move(y_clamp);
    coord_exprs[2] = move(z_clamp);

    process(input, *dest, coord_exprs);

    return 0;
}