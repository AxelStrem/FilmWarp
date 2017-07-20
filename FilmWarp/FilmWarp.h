#pragma once

#include "Expression3V.h"
#include "Video.h"
#include "Recorder.h"

class FilmWarper
{
    std::function<void(int)> callback_onframe;

    template<class T> SmartSpan<T> evaluate(std::unique_ptr<Expression3V>& pExpr)
    {
        return pExpr->evaluateI();
    }

    template<> SmartSpan<float> evaluate(std::unique_ptr<Expression3V>& pExpr)
    {
        return pExpr->evaluateF();
    }

    template<class XT, class YT, class ZT>
    void process3(Video& input, Recorder& dest, std::array<std::unique_ptr<Expression3V>, 3>& coord_exprs)
    {
        Mat frame = dest.getSampleFrame();

        int pixel_amount = dest.width() * dest.height();

        SmartSpan<int> coord_x, coord_y;
        SmartSpan<float> coord_xf, coord_yf;

        coord_x.size = coord_y.size = coord_xf.size = coord_yf.size = pixel_amount;

        coord_x.type = coord_xf.type = SpanType::SparseLinear;
        coord_y.type = coord_yf.type = SpanType::Sparse;

        coord_x.offsets.clear();

        for (int i = 0; i < dest.height() + 1; i++)
            coord_x.offsets.push_back(i*dest.width());

        coord_y.offsets = coord_xf.offsets = coord_yf.offsets = coord_x.offsets;

        coord_x.data.clear();
        coord_xf.data.clear();
        coord_y.data.clear();
        coord_yf.data.clear();

        for (int i = 0; i < dest.height(); i++)
        {
            coord_x.data.push_back(0);
            coord_x.data.push_back(1);
            coord_xf.data.push_back(0.f);
            coord_xf.data.push_back(1.f);
            coord_y.data.push_back(i);
            coord_yf.data.push_back(static_cast<float>(i));
        }

        for (auto &expr : coord_exprs)
        {
            expr->setVars(&coord_x, &coord_y);
            expr->setVars(&coord_xf, &coord_yf);
        }

        Interval full_x{ 0.f, static_cast<float>(dest.width()) };
        Interval full_y{ 0.f, static_cast<float>(dest.height()) };


        const int bstep = 24;
        for (int bstart = 0, bend = min(bstart+bstep, dest.framecount()); bstart < dest.framecount(); bstart = bend, bend = min(bstart + bstep, dest.framecount()))
        {
            Interval zint{ static_cast<float>(bstart),static_cast<float>(bend) };
            auto frame_span = coord_exprs[2]->getImage(full_x, full_y, zint);

            input.loadFrame(static_cast<int>(frame_span.a), static_cast<int>(frame_span.b) + 2);

            for (int f = bstart; f < bend; f++)
            {
                float ft = static_cast<float>(f);
                for (auto &expr : coord_exprs)
                {
                    expr->setZ(f);
                    expr->setZ(ft);
                }

                SmartSpan<std::common_type<XT, YT>::type> xvals_s = evaluate<std::common_type<XT, YT>::type>(coord_exprs[0]);
                SmartSpan<std::common_type<XT, YT>::type> yvals_s = evaluate<std::common_type<XT, YT>::type>(coord_exprs[1]);
                SmartSpan<ZT>                             zvals_s = evaluate<ZT>(coord_exprs[2]);

                xvals_s.to_dense();
                yvals_s.to_dense();
                zvals_s.to_dense();

                auto xvals = xvals_s.data;
                auto yvals = yvals_s.data;
                auto zvals = zvals_s.data;



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
                if(callback_onframe)
                    callback_onframe(f);
            }

            zint = Interval{ static_cast<float>(bend), static_cast<float>(dest.framecount()) };
            auto frame_togo = coord_exprs[2]->getImage(full_x, full_y, zint);

            while ((length(zint) > bstep) && (length(frame_togo) > input.max_frames()))
            {
                zint.b = zint.a + (zint.b - zint.a) / 2;
                frame_togo = coord_exprs[2]->getImage(full_x, full_y, zint);
            }

            input.keepFrames(static_cast<int>(frame_togo.a), static_cast<int>(frame_togo.b) + 2);
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

public:
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

    void setFrameCallback(const std::function<void(int)>& func)
    {
        callback_onframe = func;
    }
};


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