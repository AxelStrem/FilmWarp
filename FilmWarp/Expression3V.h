#pragma once


class Expression3V
{
protected:
    std::vector<float>* xf;
    std::vector<float>* yf;
    std::vector<int>*   xi;
    std::vector<int>*   yi;
    double              zf;
    long long           zi;

    std::vector<std::unique_ptr<Expression3V>> pChildren;

public:
    Expression3V();
    virtual bool isPrecise() const;

    void addChild(std::unique_ptr<Expression3V> pC);

    void setFVars(std::vector<float>* xf_, std::vector<float>* yf_);
    void setIVars(std::vector<int>* xi_, std::vector<int>* yi_);
    void setFZ(double zf_);
    void setIZ(long long zi_);

    virtual std::vector<float> evaluateF();
    virtual std::vector<int> evaluateI();
};

class EVarX : public Expression3V
{
public:
    virtual bool isPrecise() const;

    virtual std::vector<float> evaluateF();
    virtual std::vector<int> evaluateI();
};

class EVarY : public Expression3V
{
public:
    virtual bool isPrecise() const;

    virtual std::vector<float> evaluateF();
    virtual std::vector<int> evaluateI();
};

class EVarZ : public Expression3V
{
public:
    virtual bool isPrecise() const;

    virtual std::vector<float> evaluateF();
    virtual std::vector<int> evaluateI();
};

class ESum : public Expression3V
{
public:
    virtual bool isPrecise() const;

    virtual std::vector<float> evaluateF();
    virtual std::vector<int> evaluateI();
};

class EScaleI : public Expression3V
{
    int coef;
public:
    EScaleI(int scalar);
    virtual bool isPrecise() const;

    virtual std::vector<float> evaluateF();
    virtual std::vector<int> evaluateI();
};

class EScaleF : public Expression3V
{
    float coef;
public:
    EScaleF(float scalar);
    virtual bool isPrecise() const;

    virtual std::vector<float> evaluateF();
    virtual std::vector<int> evaluateI();
};


class EConstI : public Expression3V
{
    int value;
public:
    EConstI(int c);
    virtual bool isPrecise() const;

    virtual std::vector<float> evaluateF();
    virtual std::vector<int> evaluateI();
};

class EConstF : public Expression3V
{
    float value;
public:
    EConstF(float c);
    virtual bool isPrecise() const;

    virtual std::vector<float> evaluateF();

    virtual std::vector<int> evaluateI();
};