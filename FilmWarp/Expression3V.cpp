#pragma once
#include "stdafx.h"
#include "Expression3V.h"

Interval operator+(Interval i1, Interval i2)
{
    return Interval{ i1.a + i2.a, i1.b + i2.b };
}

Interval operator*(Interval i1, Interval i2)
{
    Interval result;
    result.a = std::min({ i1.a*i2.a, i1.a*i2.b, i1.b*i2.a });
    result.b = std::max(i1.a*i2.a, i1.b*i2.b);
    return result;
}

Interval operator*(float x, Interval i)
{
    if (x >= 0)
        return Interval{ x*i.a, x*i.b };
    else
        return Interval{ x*i.b, x*i.a };
}

Interval invert(Interval i)
{
    if ((i.a < 0) && (i.b > 0))
        return Interval{ std::numeric_limits<float>::min(), std::numeric_limits<float>::max() };
    Interval result;
    result.a = ((i.b == 0.f) ? std::numeric_limits<float>::min() : (1.f / i.b));
    result.b = ((i.a == 0.f) ? std::numeric_limits<float>::max() : (1.f / i.a));
    return result;
}

float length(Interval i)
{
    return fabs(i.b - i.a);
}


std::vector<Interval> diff(Interval i1, Interval i2)
{
    std::vector<Interval> result;
    if (i2.a < i1.a)
        result.push_back({ i2.a, i1.a });
    if (i1.b < i2.b)
        result.push_back({ i1.b, i2.b });
    return result;
}

Expression3V::Expression3V() : xf(nullptr), yf(nullptr), xi(nullptr), yi(nullptr), zf(0.0), zi(0), width(0) {}

bool Expression3V::isPrecise() const { return true; }

void Expression3V::addChild(std::unique_ptr<Expression3V> pC)
{
    pChildren.push_back(std::move(pC));
}

std::unique_ptr<Expression3V> Expression3V::popChild()
{
    auto r = move(pChildren.back());
    pChildren.pop_back();
    return r;
}

void Expression3V::setVars(SmartSpan<float>* xf_, SmartSpan<float>* yf_)
{
    xf = xf_; yf = yf_;
    width = static_cast<int>(yf_->size);
    for (auto& p : pChildren)
        p->setVars(xf_, yf_);
}

void Expression3V::setVars(SmartSpan<int>* xi_, SmartSpan<int>* yi_)
{
    xi = xi_; yi = yi_;
    width = static_cast<int>(yi_->size);
    for (auto& p : pChildren)
        p->setVars(xi_, yi_);
}

void Expression3V::setZ(float zf_)
{
    zf = zf_;
    for (auto& p : pChildren)
        p->setZ(zf_);
}

void Expression3V::setZ(int zi_)
{
    zi = zi_;
    for (auto& p : pChildren)
        p->setZ(zi_);
}

float Expression3V::priority() const
{
    return 0.f;
}

SmartSpan<float> Expression3V::evaluateF() { return SmartSpan<float>(width); }
SmartSpan<int> Expression3V::evaluateI() { return SmartSpan<int>(width); }


bool EVarX::isPrecise() const { return true; }

SmartSpan<float> EVarX::evaluateF() { return *xf; }
SmartSpan<int> EVarX::evaluateI() { return *xi; }

Interval EVarX::getImage(Interval & x, Interval & y, Interval & z)
{
    return x;
}

bool EVarY::isPrecise() const { return true; }

SmartSpan<float> EVarY::evaluateF() { return *yf; }
SmartSpan<int> EVarY::evaluateI() { return *yi; }

Interval EVarY::getImage(Interval & x, Interval & y, Interval & z)
{
    return y;
}

bool EVarZ::isPrecise() const { return true; }

SmartSpan<float> EVarZ::evaluateF() { return SmartSpan<float>(width, zf); }
SmartSpan<int> EVarZ::evaluateI() { return SmartSpan<int>(width, zi); }

Interval EVarZ::getImage(Interval & x, Interval & y, Interval & z)
{
    return z;
}

bool ESum::isPrecise() const
{
    return std::all_of(pChildren.begin(), pChildren.end(), [](auto& ptr) { return ptr->isPrecise(); });
}

SmartSpan<float> ESum::evaluateF()
{
    auto vec = pChildren[0]->evaluateF();
    for (auto it = pChildren.begin() + 1; it != pChildren.end(); ++it)
    {
        vec = vec + (*it)->evaluateF();
    }
    return vec;
}

SmartSpan<int> ESum::evaluateI()
{
    auto vec = pChildren[0]->evaluateI();
    for (auto it = pChildren.begin() + 1; it != pChildren.end(); ++it)
    {
        vec = vec + (*it)->evaluateI();
    }
    return vec;
}

Interval ESum::getImage(Interval & x, Interval & y, Interval & z)
{
    return std::accumulate(pChildren.begin(), pChildren.end(), Interval{ 0,0 }, [&](Interval s, auto& pChild)
    {
        return s + pChild->getImage(x, y, z);
    });
}

float ESum::priority() const
{
    return 1.0f;
}


EScaleI::EScaleI(int scalar) : coef(scalar), coef_f(static_cast<float>(scalar)) {}
bool EScaleI::isPrecise() const
{
    return pChildren[0]->isPrecise();
}

SmartSpan<float> EScaleI::evaluateF()
{
    auto vec = pChildren[0]->evaluateF();
    vec = vec * SmartSpan<float>(width, coef_f);
    return vec;
}

SmartSpan<int> EScaleI::evaluateI()
{
    auto vec = pChildren[0]->evaluateI();
    vec = vec * SmartSpan<int>(width, coef);
    return vec;
}

Interval EScaleI::getImage(Interval & x, Interval & y, Interval & z)
{
    return coef_f*pChildren[0]->getImage(x,y,z);
}

EScaleF::EScaleF(float scalar) : coef(scalar) {}

bool EScaleF::isPrecise() const
{
    return false;
}

SmartSpan<float> EScaleF::evaluateF()
{
    auto vec = pChildren[0]->evaluateF();
    vec = vec * SmartSpan<float>(width, coef);
    return vec;
}

Interval EScaleF::getImage(Interval & x, Interval & y, Interval & z)
{
    return coef*pChildren[0]->getImage(x, y, z);
}

EConstI::EConstI(int c) : value(c), value_f(static_cast<float>(c)) {}

bool EConstI::isPrecise() const
{
    return true;
}

SmartSpan<float> EConstI::evaluateF()
{
    return SmartSpan<float>(width, value_f);
}

SmartSpan<int> EConstI::evaluateI()
{
    return SmartSpan<int>(width, value);
}

    Interval EConstI::getImage(Interval & x, Interval & y, Interval & z)
    {
        return Interval{ value_f, value_f };
    }


EConstF::EConstF(float c) : value(c) {}

bool EConstF::isPrecise() const
{
    return false;
}

SmartSpan<float> EConstF::evaluateF()
{
    return SmartSpan<float>(width, value);
}

SmartSpan<int> EConstF::evaluateI()
{
    return SmartSpan<int>(width, 0);
}

Interval EConstF::getImage(Interval & x, Interval & y, Interval & z)
{
    return Interval{ value, value };
}

EClampI::EClampI(int low_, int high_) : low(low_), high(high_), low_f(static_cast<float>(low_)), high_f(static_cast<float>(high_))
{
}

bool EClampI::isPrecise() const
{
    return pChildren[0]->isPrecise();;
}

template<class T> void sparselinear_clamp(SmartSpan<T>& vec, T low, T high)
{
    SmartSpan<T> result;
    result.type = SpanType::SparseLinear;
    result.size = vec.size;
    result.offsets.push_back(0);
    for (int i = 0; i < vec.offsets.size() - 1; i++)
    {
        T v = vec.data[2 * i];
        T vs = v;
        T step = vec.data[2 * i + 1];
        int j = vec.offsets[i];

        if (v < low)
        {
            for (; j < vec.offsets[i + 1]; j++, v += step)
            {
                if (v > low)
                {
                    result.data.push_back(low);
                    result.data.push_back(0);
                    result.offsets.push_back(j);
                    goto loop_mid;
                }
            }

            result.data.push_back(low);
            result.data.push_back(0);
            result.offsets.push_back(j);
            goto loop_exit;
        }

        if (v > high)
        {
            for (; j < vec.offsets[i + 1]; j++, v += step)
            {
                if (v < high)
                {
                    result.data.push_back(high);
                    result.data.push_back(0);
                    result.offsets.push_back(j);
                    goto loop_mid;
                }
            }

            result.data.push_back(high);
            result.data.push_back(0);
            result.offsets.push_back(j);
            goto loop_exit;
        }

    loop_mid:
        vs = v;
        int js = j;

        for (; j < vec.offsets[i + 1]; j++, v += step)
        {
            if ((v > high) || (v < low))
            {
                if (j > js)
                {
                    result.data.push_back(vs);
                    result.data.push_back(step);
                    result.offsets.push_back(j);
                }
                goto loop_last;
            }
        }

        result.data.push_back(vs);
        result.data.push_back(step);
        result.offsets.push_back(j);

        goto loop_exit;
    loop_last:

        if (v > high)
        {
            result.data.push_back(high);
            result.data.push_back(0);
            result.offsets.push_back(j);
        }
        else
        {
            result.data.push_back(low);
            result.data.push_back(0);
            result.offsets.push_back(j);
        }

    loop_exit:;
    }

    vec = std::move(result);
}

SmartSpan<float> EClampI::evaluateF()
{
    auto vec = pChildren[0]->evaluateF();
    switch (vec.type)
    {
    case SpanType::Dense:
    case SpanType::Sparse: for (auto& val : vec.data) val = clamp<float>(val, low_f, high_f);  break;
    case SpanType::SparseLinear: sparselinear_clamp<float>(vec, low_f, high_f);
    }
    return vec;
}

SmartSpan<int> EClampI::evaluateI()
{
    auto vec = pChildren[0]->evaluateI();
    switch(vec.type)
    {
    case SpanType::Dense:
    case SpanType::Sparse: for(auto& val : vec.data) val = clamp<int>(val, low, high);  break;
    case SpanType::SparseLinear:  sparselinear_clamp<int>(vec, low, high); break;
    }
    return vec;
}

Interval EClampI::getImage(Interval & x, Interval & y, Interval & z)
{
    Interval i = pChildren[0]->getImage(x, y, z);
    return Interval{clamp<float>(i.a,low_f,high_f), clamp<float>(i.b,low_f,high_f)};
}

bool EMult::isPrecise() const
{
    return std::all_of(pChildren.begin(), pChildren.end(), [](auto& ptr) { return ptr->isPrecise(); });
}

SmartSpan<float> EMult::evaluateF()
{
    auto vec = pChildren[0]->evaluateF();
    for (auto it = pChildren.begin() + 1; it != pChildren.end(); ++it)
    {
        vec = vec * (*it)->evaluateF();
    }
    return vec;
}

SmartSpan<int> EMult::evaluateI()
{
    auto vec = pChildren[0]->evaluateI();
    for (auto it = pChildren.begin() + 1; it != pChildren.end(); ++it)
    {
        vec = vec * (*it)->evaluateI();
    }
    return vec;
}

Interval EMult::getImage(Interval & x, Interval & y, Interval & z)
{
    return std::accumulate(pChildren.begin(), pChildren.end(), Interval{ 1.f,1.f }, [&](Interval s, auto& pChild)
    {
        return s * pChild->getImage(x, y, z);
    });
}

bool EMod::isPrecise() const
{
    return std::all_of(pChildren.begin(), pChildren.end(), [](auto& ptr) { return ptr->isPrecise(); });
}

float mod(float x, float y)
{
    return x - floor(x / y)*y;
}

int mod(int x, int y)
{
    return x%y;
}

template<class T> void mod_spans(SmartSpan<T>& vec, const SmartSpan<T>& vop)
{
    // if ((vec.type == SpanType::SparseLinear) && (vop.type == SpanType::Sparse))
    // {
    //      // can be improved in to return SparseLinear span in this case      
    // }

    generic_op(vec, vop, [](auto x, auto y) { return mod(x, y); });
}

SmartSpan<float> EMod::evaluateF()
{
    auto vec = pChildren[0]->evaluateF();
    auto vop = pChildren[1]->evaluateF();

    mod_spans<float>(vec, vop);
    return vec;
}

SmartSpan<int> EMod::evaluateI()
{
    auto vec = pChildren[0]->evaluateI();
    auto vop = pChildren[1]->evaluateI();

    mod_spans<int>(vec, vop);
    return vec;
}

Interval EMod::getImage(Interval & x, Interval & y, Interval & z)
{
    Interval a = pChildren[0]->getImage(x, y, z);
    Interval b = pChildren[1]->getImage(x, y, z);

    if (a.a > 0 && a.b <= b.a)
        return a;

    return Interval{ 0.f, b.b};
}

bool EDiv::isPrecise() const
{
    return false;
}

SmartSpan<float> EDiv::evaluateF()
{
    auto vec = pChildren[0]->evaluateF();
    auto vop = pChildren[1]->evaluateF();

    generic_op(vec, vop, [](auto &x, auto &y) { return x / y; });

    return vec;
}

Interval EDiv::getImage(Interval & x, Interval & y, Interval & z)
{
    Interval a = pChildren[0]->getImage(x, y, z);
    Interval b = pChildren[1]->getImage(x, y, z);
    return a * invert(b);
}

bool EFloor::isPrecise() const
{
    return std::all_of(pChildren.begin(), pChildren.end(), [](auto& ptr) { return ptr->isPrecise(); });
}

float floor_op(float a, float b)
{
    return floor(a / b)*b;
}

int floor_op(int a, int b)
{
    return a - (a%b);
}

SmartSpan<float> EFloor::evaluateF()
{
    auto vec = pChildren[0]->evaluateF();
    auto vop = pChildren[1]->evaluateF();

    generic_op(vec, vop, [](auto &x, auto &y) { return floor_op(x, y); });

    return vec;
}

SmartSpan<int> EFloor::evaluateI()
{
    auto vec = pChildren[0]->evaluateI();
    auto vop = pChildren[1]->evaluateI();

    generic_op(vec, vop, [](auto &x, auto &y) { return floor_op(x, y); });

    return vec;
}

Interval EFloor::getImage(Interval & x, Interval & y, Interval & z)
{
    Interval a = pChildren[0]->getImage(x, y, z);
    Interval b = pChildren[1]->getImage(x, y, z);

    return a;
}
