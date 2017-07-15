#pragma once
#include "stdafx.h"
#include "Expression3V.h"

Expression3V::Expression3V() : xf(nullptr), yf(nullptr), xi(nullptr), yi(nullptr), zf(0.0), zi(0) {}

bool Expression3V::isPrecise() const { return true; }

void Expression3V::addChild(std::unique_ptr<Expression3V> pC)
{
    pChildren.push_back(std::move(pC));
}

void Expression3V::setFVars(std::vector<float>* xf_, std::vector<float>* yf_)
{
    xf = xf_; yf = yf_;
    for (auto& p : pChildren)
        p->setFVars(xf_, yf_);
}

void Expression3V::setIVars(std::vector<int>* xi_, std::vector<int>* yi_)
{
    xi = xi_; yi = yi_;
    for (auto& p : pChildren)
        p->setIVars(xi_, yi_);
}

void Expression3V::setFZ(double zf_)
{
    zf = zf_;
    for (auto& p : pChildren)
        p->setFZ(zf_);
}

void Expression3V::setIZ(long long zi_)
{
    zi = zi_;
    for (auto& p : pChildren)
        p->setIZ(zi_);
}

float Expression3V::priority() const
{
    return 0.f;
}

std::vector<float> Expression3V::evaluateF() { return std::vector<float>(xf->size()); }
std::vector<int> Expression3V::evaluateI() { return std::vector<int>(xi->size()); }


bool EVarX::isPrecise() const { return true; }

std::vector<float> EVarX::evaluateF() { return *xf; }
std::vector<int> EVarX::evaluateI() { return *xi; }

bool EVarY::isPrecise() const { return true; }

std::vector<float> EVarY::evaluateF() { return *yf; }
std::vector<int> EVarY::evaluateI() { return *yi; }

bool EVarZ::isPrecise() const { return true; }

std::vector<float> EVarZ::evaluateF() { return std::vector<float>(xf->size(), zf); }
std::vector<int> EVarZ::evaluateI() { return std::vector<int>(xi->size(), zi); }

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

std::vector<int> EScaleF::evaluateI()
{
    auto vec = pChildren[0]->evaluateI();
    for (int i = 0; i < vec.size(); i++)
    {
        vec[i] *= coef;
    }
    return vec;
}

EConstI::EConstI(int c) : value(c) {}

bool EConstI::isPrecise() const
{
    return true;
}

std::vector<float> EConstI::evaluateF()
{
    return std::vector<float>(xf->size(), value);
}

    std::vector<int> EConstI::evaluateI()
{
    return std::vector<int>(xi->size(), value);
}


EConstF::EConstF(float c) : value(c) {}

bool EConstF::isPrecise() const
{
    return false;
}

std::vector<float> EConstF::evaluateF()
{
    return std::vector<float>(xf->size(), value);
}

std::vector<int> EConstF::evaluateI()
{
    return std::vector<int>(xi->size(), 0);
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
        vec[i] = clamp<float>(vec[i],low,high);
    }
    return vec;
}

std::vector<int> EClampI::evaluateI()
{
    auto vec = pChildren[0]->evaluateI();
    for (int i = 0; i < vec.size(); i++)
    {
        vec[i] = clamp<float>(vec[i], low, high);
    }
    return vec;
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
