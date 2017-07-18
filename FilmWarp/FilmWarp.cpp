#include "stdafx.h"
#include "StringParser.h"
#include "FilmWarp.h"

using namespace std;
using namespace cv;

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

    input.setMaxFrames(128);

    StringParser sp;
    sp.setConsts(input.width(), input.height(), input.framecount());

    if (params.find("s") != params.end())
    {
        auto sz_exprs = sp.parseExprTriplet(params["s"]);
        SmartSpan<int> nvec_i(1, 0);
        SmartSpan<float> nvec_f(1, 0.f);
        
        for (auto& pExpr : sz_exprs)
        {
            pExpr->setVars(&nvec_i, &nvec_i);
            pExpr->setVars(&nvec_f, &nvec_f);
            pExpr->setZ(0);
            pExpr->setZ(0.f);
        }

        apply_result(sz_exprs[0], [&out_w](auto vec) { out_w = static_cast<int>(vec.data[0]); });
        apply_result(sz_exprs[1], [&out_h](auto vec) { out_h = static_cast<int>(vec.data[0]); });
        apply_result(sz_exprs[2], [&out_fc](auto vec) { out_fc = static_cast<int>(vec.data[0]); });
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

    FilmWarper fw;

    fw.process(input, *dest, coord_exprs);

    return 0;
}