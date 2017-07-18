#pragma once

#include "SmartSpan.h"

struct Interval
{
    float a;
    float b;
};

Interval operator+(Interval i1, Interval i2);
Interval operator*(Interval i1, Interval i2);
Interval operator*(float x, Interval i);
Interval invert(Interval i);
std::vector<Interval> diff(Interval i1, Interval i2);

class Expression3V
{
protected:
    SmartSpan<float>* xf;
    SmartSpan<float>* yf;
    SmartSpan<int>*   xi;
    SmartSpan<int>*   yi;
    float               zf;
    int                 zi;
    int width;

    std::vector<std::unique_ptr<Expression3V>> pChildren;

public:
    Expression3V();
    virtual bool isPrecise() const;
    virtual float priority() const;

    void addChild(std::unique_ptr<Expression3V> pC);
    std::unique_ptr<Expression3V> popChild();

    void setVars(SmartSpan<float>* xf_, SmartSpan<float>* yf_);
    void setVars(SmartSpan<int>* xi_, SmartSpan<int>* yi_);
    void setZ(float zf_);
    void setZ(int zi_);

    virtual SmartSpan<float> evaluateF();
    virtual SmartSpan<int> evaluateI();

    virtual Interval getImage(Interval& x, Interval& y, Interval& z) { return Interval{ 0,0 }; }

    virtual ~Expression3V() {}
};

class EVarX : public Expression3V
{
public:
    virtual bool isPrecise() const;

    virtual SmartSpan<float> evaluateF();
    virtual SmartSpan<int> evaluateI();

    virtual Interval getImage(Interval& x, Interval& y, Interval& z);
};

class EVarY : public Expression3V
{
public:
    virtual bool isPrecise() const;

    virtual SmartSpan<float> evaluateF();
    virtual SmartSpan<int> evaluateI();

    virtual Interval getImage(Interval& x, Interval& y, Interval& z);
};

class EVarZ : public Expression3V
{
public:
    virtual bool isPrecise() const;

    virtual SmartSpan<float> evaluateF();
    virtual SmartSpan<int> evaluateI();

    virtual Interval getImage(Interval& x, Interval& y, Interval& z);
};

class ESum : public Expression3V
{
public:
    virtual bool isPrecise() const;

    virtual SmartSpan<float> evaluateF();
    virtual SmartSpan<int> evaluateI();

    virtual Interval getImage(Interval& x, Interval& y, Interval& z);

    virtual float priority() const;
};

class EMult : public Expression3V
{
public:
    virtual bool isPrecise() const;

    virtual SmartSpan<float> evaluateF();
    virtual SmartSpan<int> evaluateI();

    virtual Interval getImage(Interval& x, Interval& y, Interval& z);
};

class EDiv : public Expression3V
{
public:
    virtual bool isPrecise() const;

    virtual SmartSpan<float> evaluateF();

    virtual Interval getImage(Interval& x, Interval& y, Interval& z);
};

class EMod : public Expression3V
{
public:
    virtual bool isPrecise() const;

    virtual SmartSpan<float> evaluateF();
    virtual SmartSpan<int> evaluateI();

    virtual Interval getImage(Interval& x, Interval& y, Interval& z);
};

class EFloor : public Expression3V
{
public:
    virtual bool isPrecise() const;

    virtual SmartSpan<float> evaluateF();
    virtual SmartSpan<int> evaluateI();

    virtual Interval getImage(Interval& x, Interval& y, Interval& z);
};

class EScaleI : public Expression3V
{
    int coef;
    float coef_f;
public:
    EScaleI(int scalar);
    virtual bool isPrecise() const;

    virtual SmartSpan<float> evaluateF();
    virtual SmartSpan<int> evaluateI();

    virtual Interval getImage(Interval& x, Interval& y, Interval& z);
};

class EScaleF : public Expression3V
{
    float coef;
public:
    EScaleF(float scalar);
    virtual bool isPrecise() const;

    virtual SmartSpan<float> evaluateF();

    virtual Interval getImage(Interval& x, Interval& y, Interval& z);
};


class EConstI : public Expression3V
{
    int value;
    float value_f;
public:
    EConstI(int c);
    virtual bool isPrecise() const;

    virtual SmartSpan<float> evaluateF();
    virtual SmartSpan<int> evaluateI();

    virtual Interval getImage(Interval& x, Interval& y, Interval& z);
};

class EConstF : public Expression3V
{
    float value;
public:
    EConstF(float c);
    virtual bool isPrecise() const;

    virtual SmartSpan<float> evaluateF();

    virtual SmartSpan<int> evaluateI();

    virtual Interval getImage(Interval& x, Interval& y, Interval& z);
};

class EClampI : public Expression3V
{
    int low, high;
    float low_f, high_f;
public:
    EClampI(int low_, int high_);
    virtual bool isPrecise() const;

    virtual SmartSpan<float> evaluateF();
    virtual SmartSpan<int> evaluateI();

    virtual Interval getImage(Interval& x, Interval& y, Interval& z);
};