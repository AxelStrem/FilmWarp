#pragma once

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

enum class SpanType
{
    Dense,
    Sparse,
    SparseLinear
};

template<class T> struct SmartSpan
{
    SpanType type;
    int      size;
    std::vector<T>   data;
    std::vector<int> offsets;

    SmartSpan(int size_, T val = 0) : size(size_), type(SpanType::Sparse), data{ T{val} }, offsets{ 0,size }
    {}

    SmartSpan() : size(0), type(SpanType::Sparse), data{}, offsets{}
    {}

    template<class F> void foreach_dense(F func)
    {
        for (int i = 0; i < size; i++)
        {
            func(i, data[i]);
        }
    }

    template<class F> void foreach_sparse(F func)
    {
        int i = 0;
        for (int off = 0; off < offsets.size() - 1; off++)
        {
            for (int j = offsets[off]; j < offsets[off + 1]; j++, i++)
            {
                func(i, data[off]);
            }
        }
    }

    template<class F> void foreach_sparselinear(F func)
    {
        int i = 0;
        for (int off = 0; off < offsets.size() - 1; off++)
        {
        T val = data[2 * off];
        T step = data[2 * off + 1];
        for (int j = offsets[off]; j < offsets[off + 1]; j++, i++)
        {
            func(i, val);
            val += step;
        }
        }
    }

    template<class F> void foreach_dense(F func) const
    {
        for (int i = 0; i < size; i++)
        {
            func(i, data[i]);
        }
    }

    template<class F> void foreach_sparse(F func) const
    {
        int i = 0;
        for (int off = 0; off < offsets.size() - 1; off++)
        {
            for (int j = offsets[off]; j < offsets[off + 1]; j++, i++)
            {
                func(i, data[off]);
            }
        }
    }

    template<class F> void foreach_sparselinear(F func) const
    {
        int i = 0;
        for (int off = 0; off < offsets.size() - 1; off++)
        {
            T val = data[2 * off];
            T step = data[2 * off + 1];
            for (int j = offsets[off]; j < offsets[off + 1]; j++, i++)
            {
                func(i, val);
                val += step;
            }
        }
    }

    template<class F> void foreach(F func)
    {
        switch (type)
        {
        case SpanType::Dense:        foreach_dense(func); break;
        case SpanType::Sparse:       foreach_sparse(func); break;
        case SpanType::SparseLinear: foreach_sparselinear(func); break;
        }
    }

    template<class F> void foreach(F func) const
    {
        switch (type)
        {
        case SpanType::Dense:        foreach_dense(func); break;
        case SpanType::Sparse:       foreach_sparse(func); break;
        case SpanType::SparseLinear: foreach_sparselinear(func); break;
        }
    }

    void to_dense()
    {
        if (type == SpanType::Dense) return;
        std::vector<T> ndata(size);
        foreach([&](int i, T val) {ndata[i] = val;  });
        data = move(ndata);
        type = SpanType::Dense;
    }
};

template<class T> void dense_add(SmartSpan<T>& dense_dst, const SmartSpan<T>& src)
{
    src.foreach([&](int i, T val) { dense_dst.data[i] += val; });
}

template<class T> void sparselinear_add(SmartSpan<T>& dst, const SmartSpan<T>& src)
{
    SmartSpan<T> result;
    result.type = SpanType::SparseLinear;
    result.offsets.push_back(0);
    result.size = dst.size;
    int off1 = 1;
    int off2 = 1;
    if (src.type == SpanType::SparseLinear)
    {
        while ((off1 < dst.offsets.size()) && (off2 < src.offsets.size()))
        {
            result.data.push_back(dst.data[2 * (off1-1)] + src.data[2 * (off2-1)]);
            result.data.push_back(dst.data[2 * (off1-1) + 1] + src.data[2 * (off2-1) + 1]);

            if (dst.offsets[off1] == src.offsets[off2])
            {
                result.offsets.push_back(dst.offsets[off1]);
                off1++;
                off2++;
            }
            else if (dst.offsets[off1] < src.offsets[off2])
            {
                result.offsets.push_back(dst.offsets[off1]);
                off1++;
            }
            else
            {
                result.offsets.push_back(src.offsets[off2]);
                off2++;
            }
        }
    }
    else // src is Sparse
    {
        while ((off1 < dst.offsets.size()) && (off2 < src.offsets.size()))
        {
            result.data.push_back(dst.data[2 * (off1-1)] + src.data[(off2-1)]);
            result.data.push_back(dst.data[2 * (off1-1) + 1]);

            if (dst.offsets[off1] == src.offsets[off2])
            {
                result.offsets.push_back(dst.offsets[off1]);
                off1++;
                off2++;
            }
            else if (dst.offsets[off1] < src.offsets[off2])
            {
                result.offsets.push_back(dst.offsets[off1]);
                off1++;
            }
            else
            {
                result.offsets.push_back(src.offsets[off2]);
                off2++;
            }
        }
    }
    result.offsets.push_back(result.size);
    dst = std::move(result);
}

template<class T, class F> void sparse_op(SmartSpan<T>& dst, const SmartSpan<T>& src, F op)
{
        SmartSpan<T> result;
        result.type = SpanType::Sparse;
        result.offsets.push_back(0);
        result.size = dst.size;
        int off1 = 1;
        int off2 = 1;
        while ((off1 < dst.offsets.size()) && (off2 < src.offsets.size()))
        {
            result.data.push_back( op(dst.data[off1-1], src.data[off2-1]) );

            if (dst.offsets[off1] == src.offsets[off2])
            {
                result.offsets.push_back(dst.offsets[off1]);
                off1++;
                off2++;
            }
            else if (dst.offsets[off1] < src.offsets[off2])
            {
                result.offsets.push_back(dst.offsets[off1]);
                off1++;
            }
            else
            {
                result.offsets.push_back(src.offsets[off2]);
                off2++;
            }
        }

        dst = std::move(result);
}

template<class T> SmartSpan<T> operator+(SmartSpan<T> a, SmartSpan<T> b)
{
    if (a.type == SpanType::Dense)
    {
        dense_add(a, b);
        return a;
    }
    if (b.type == SpanType::Dense)
    {
        dense_add(b, a);
        return b;
    }
    if (a.type == SpanType::SparseLinear)
    {
        sparselinear_add(a, b);
        return a;
    }
    if (b.type == SpanType::SparseLinear)
    {
        sparselinear_add(b, a);
        return b;
    }
    
    sparse_op(a, b, [](T x, T y) { return x + y; });
    return a;
}

template<class T> void dense_mult(SmartSpan<T>& dense_dst, const SmartSpan<T>& src)
{
    src.foreach([&](int i, T val) { dense_dst.data[i] *= val; });
}

template<class T> void sparselinear_mult(SmartSpan<T>& dst, const SmartSpan<T>& src)
{
    if (src.type == SpanType::SparseLinear)
    {
        dst.to_dense();
        dense_mult(dst, src);
        return;
    }
    else // src is Sparse
    {
        SmartSpan<T> result;
        result.type = SpanType::SparseLinear;
        result.offsets.push_back(0);
        result.size = dst.size;
        int off1 = 1;
        int off2 = 1;

        while ((off1 < dst.offsets.size()) && (off2 < src.offsets.size()))
        {
            result.data.push_back(dst.data[2 * (off1-1)]*src.data[(off2-1)]);
            result.data.push_back(dst.data[2 * (off1-1) + 1]*src.data[off2-1]);

            if (dst.offsets[off1] == src.offsets[off2])
            {
                result.offsets.push_back(dst.offsets[off1]);
                off1++;
                off2++;
            }
            else if (dst.offsets[off1] < src.offsets[off2])
            {
                result.offsets.push_back(dst.offsets[off1]);
                off1++;
            }
            else
            {
                result.offsets.push_back(src.offsets[off2]);
                off2++;
            }
        }

        result.offsets.push_back(result.size);
        dst = std::move(result);
    }
}


template<class T> SmartSpan<T> operator*(SmartSpan<T> a, SmartSpan<T> b)
{
    if (a.type == SpanType::Dense)
    {
        dense_mult(a, b);
        return a;
    }
    if (b.type == SpanType::Dense)
    {
        dense_mult(b, a);
        return b;
    }
    if (a.type == SpanType::SparseLinear)
    {
        sparselinear_mult(a, b);
        return a;
    }
    if (b.type == SpanType::SparseLinear)
    {
        sparselinear_mult(b, a);
        return b;
    }

    sparse_op(a, b, [](T x, T y) { return x * y; });
    return a;
}


template<class T, class F> void generic_op(SmartSpan<T>& dst, const SmartSpan<T>& src, F op)
{
    if ((dst.type == SpanType::Sparse) && (src.type == SpanType::Sparse))
    {
        return sparse_op(dst, src, op);
    }

    dst.to_dense();
    src.foreach([&](int i, T val) { dst.data[i] = op(dst.data[i], val); });
}

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