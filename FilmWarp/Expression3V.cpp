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
    Interval result;
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

void Expression3V::setVars(std::vector<float>* xf_, std::vector<float>* yf_)
{
    xf = xf_; yf = yf_;
    width = static_cast<int>(yf_->size());
    for (auto& p : pChildren)
        p->setVars(xf_, yf_);
}

void Expression3V::setVars(std::vector<int>* xi_, std::vector<int>* yi_)
{
    xi = xi_; yi = yi_;
    width = static_cast<int>(yi_->size());
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

std::vector<float> Expression3V::evaluateF() { return std::vector<float>(width); }
std::vector<int> Expression3V::evaluateI() { return std::vector<int>(width); }


bool EVarX::isPrecise() const { return true; }

std::vector<float> EVarX::evaluateF() { return *xf; }
std::vector<int> EVarX::evaluateI() { return *xi; }

Interval EVarX::getImage(Interval & x, Interval & y, Interval & z)
{
    return x;
}

bool EVarY::isPrecise() const { return true; }

std::vector<float> EVarY::evaluateF() { return *yf; }
std::vector<int> EVarY::evaluateI() { return *yi; }

Interval EVarY::getImage(Interval & x, Interval & y, Interval & z)
{
    return y;
}

bool EVarZ::isPrecise() const { return true; }

std::vector<float> EVarZ::evaluateF() { return std::vector<float>(width, zf); }
std::vector<int> EVarZ::evaluateI() { return std::vector<int>(width, zi); }

Interval EVarZ::getImage(Interval & x, Interval & y, Interval & z)
{
    return z;
}

bool ESum::isPrecise() const
{
    return std::all_of(pChildren.begin(), pChildren.end(), [](auto& ptr) { return ptr->isPrecise(); });
}

std::vector<float> ESum::evaluateF()
{
    auto vec = pChildren[0]->evaluateF();
    for (auto it = pChildren.begin() + 1; it != pChildren.end(); ++it)
    {
        auto vadd = (*it)->evaluateF();
        for (int i = 0; i < vec.size(); i++)
        {
            vec[i] += vadd[i];
        }
    }
    return vec;
}

std::vector<int> ESum::evaluateI()
{
    auto vec = pChildren[0]->evaluateI();
    for (auto it = pChildren.begin() + 1; it != pChildren.end(); ++it)
    {
        auto vadd = (*it)->evaluateI();
        for (int i = 0; i < vec.size(); i++)
        {
            vec[i] += vadd[i];
        }
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


EScaleI::EScaleI(int scalar) : coef(scalar) {}
bool EScaleI::isPrecise() const
{
    return pChildren[0]->isPrecise();
}

std::vector<float> EScaleI::evaluateF()
{
    auto vec = pChildren[0]->evaluateF();
    for (int i = 0; i < vec.size(); i++)
    {
        vec[i] *= coef;
    }
    return vec;
}

std::vector<int> EScaleI::evaluateI()
{
    auto vec = pChildren[0]->evaluateI();
    for (int i = 0; i < vec.size(); i++)
    {
        vec[i] *= coef;
    }
    return vec;
}

Interval EScaleI::getImage(Interval & x, Interval & y, Interval & z)
{
    return coef*pChildren[0]->getImage(x,y,z);
}

EScaleF::EScaleF(float scalar) : coef(scalar) {}

bool EScaleF::isPrecise() const
{
    return false;
}

std::vector<float> EScaleF::evaluateF()
{
    auto vec = pChildren[0]->evaluateF();
    for (int i = 0; i < vec.size(); i++)
    {
        vec[i] *= coef;
    }
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

std::vector<float> EConstI::evaluateF()
{
    return std::vector<float>(width, value_f);
}

    std::vector<int> EConstI::evaluateI()
{
    return std::vector<int>(width, value);
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

std::vector<float> EConstF::evaluateF()
{
    return std::vector<float>(width, value);
}

std::vector<int> EConstF::evaluateI()
{
    return std::vector<int>(width, 0);
}

Interval EConstF::getImage(Interval & x, Interval & y, Interval & z)
{
    return Interval{ value, value };
}

EClampI::EClampI(int low_, int high_) : low(low_), high(high_)
{
}

bool EClampI::isPrecise() const
{
    return pChildren[0]->isPrecise();;
}

std::vector<float> EClampI::evaluateF()
{
    auto vec = pChildren[0]->evaluateF();
    for (int i = 0; i < vec.size(); i++)
    {
        vec[i] = clamp<float>(vec[i], static_cast<float>(low), static_cast<float>(high));
    }
    return vec;
}

std::vector<int> EClampI::evaluateI()
{
    auto vec = pChildren[0]->evaluateI();
    for (int i = 0; i < vec.size(); i++)
    {
        vec[i] = clamp<int>(vec[i], low, high);
    }
    return vec;
}

Interval EClampI::getImage(Interval & x, Interval & y, Interval & z)
{
    Interval i = pChildren[0]->getImage(x, y, z);
    return Interval{clamp<float>(i.a,low,high), clamp<float>(i.b,low,high)};
}

bool EMult::isPrecise() const
{
    return std::all_of(pChildren.begin(), pChildren.end(), [](auto& ptr) { return ptr->isPrecise(); });
}

std::vector<float> EMult::evaluateF()
{
    auto vec = pChildren[0]->evaluateF();
    for (auto it = pChildren.begin() + 1; it != pChildren.end(); ++it)
    {
        auto vmul = (*it)->evaluateF();
        for (int i = 0; i < vec.size(); i++)
        {
            vec[i] *= vmul[i];
        }
    }
    return vec;
}

std::vector<int> EMult::evaluateI()
{
    auto vec = pChildren[0]->evaluateI();
    for (auto it = pChildren.begin() + 1; it != pChildren.end(); ++it)
    {
        auto vmul = (*it)->evaluateI();
        for (int i = 0; i < vec.size(); i++)
        {
            vec[i] *= vmul[i];
        }
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

std::vector<float> EMod::evaluateF()
{
    auto vec = pChildren[0]->evaluateF();
    auto vop = pChildren[1]->evaluateF();
    for (int i = 0; i < vec.size(); i++)
    {
        float dv = vec[i] / vop[i];
        vec[i] = vec[i] - floor(dv)*vop[i];
    }
    return vec;
}

std::vector<int> EMod::evaluateI()
{
    auto vec = pChildren[0]->evaluateI();
    auto vop = pChildren[1]->evaluateI();
    for (int i = 0; i < vec.size(); i++)
    {
        vec[i] %= vop[i];
    }
    return vec;
}

Interval EMod::getImage(Interval & x, Interval & y, Interval & z)
{
    Interval a = pChildren[0]->getImage(x, y, z);
    Interval b = pChildren[1]->getImage(x, y, z);

    if (a.b <= b.a)
        return a;

    return Interval{ 0.f, b.b};
}

bool EDiv::isPrecise() const
{
    return false;
}

std::vector<float> EDiv::evaluateF()
{
    auto vec = pChildren[0]->evaluateF();
    auto vop = pChildren[1]->evaluateF();
    for (int i = 0; i < vec.size(); i++)
    {
        vec[i] /= vop[i];
    }
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

std::vector<float> EFloor::evaluateF()
{
    auto vec = pChildren[0]->evaluateF();
    auto vop = pChildren[1]->evaluateF();
    for (int i = 0; i < vec.size(); i++)
    {
        float dv = vec[i] / vop[i];
        vec[i] = floor(dv)*vop[i];
    }
    return vec;
}

std::vector<int> EFloor::evaluateI()
{
    auto vec = pChildren[0]->evaluateI();
    auto vop = pChildren[1]->evaluateI();
    for (int i = 0; i < vec.size(); i++)
    {
        vec[i] -= vec[i]%vop[i];
    }
    return vec;
}

Interval EFloor::getImage(Interval & x, Interval & y, Interval & z)
{
    Interval a = pChildren[0]->getImage(x, y, z);
    Interval b = pChildren[1]->getImage(x, y, z);

    return a;
}
