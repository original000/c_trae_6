#ifndef MATH_FUNCTIONS_H
#define MATH_FUNCTIONS_H

#include <cmath>
#include "VectorExpr.h"

// 一元函数表达式模板定义在VectorExpr.h中

// 定义各种数学函数的表达式模板

// Sin函数
template <typename Expr>
class SinExpr {
public:
    using ValueType = typename Expr::ValueType;
    
    explicit SinExpr(const Expr& expr) : expr_(expr) {}

    // 获取表达式的值
    ValueType operator[](size_t i) const {
        return std::sin(expr_[i]);
    }

    // 获取表达式的大小
    size_t size() const {
        return expr_.size();
    }

private:
    const Expr& expr_;
};

// Cos函数
template <typename Expr>
class CosExpr {
public:
    using ValueType = typename Expr::ValueType;
    
    explicit CosExpr(const Expr& expr) : expr_(expr) {}

    // 获取表达式的值
    ValueType operator[](size_t i) const {
        return std::cos(expr_[i]);
    }

    // 获取表达式的大小
    size_t size() const {
        return expr_.size();
    }

private:
    const Expr& expr_;
};

// Tan函数
template <typename Expr>
class TanExpr {
public:
    using ValueType = typename Expr::ValueType;
    
    explicit TanExpr(const Expr& expr) : expr_(expr) {}

    // 获取表达式的值
    ValueType operator[](size_t i) const {
        return std::tan(expr_[i]);
    }

    // 获取表达式的大小
    size_t size() const {
        return expr_.size();
    }

private:
    const Expr& expr_;
};

// Exp函数
template <typename Expr>
class ExpExpr {
public:
    using ValueType = typename Expr::ValueType;
    
    explicit ExpExpr(const Expr& expr) : expr_(expr) {}

    // 获取表达式的值
    ValueType operator[](size_t i) const {
        return std::exp(expr_[i]);
    }

    // 获取表达式的大小
    size_t size() const {
        return expr_.size();
    }

private:
    const Expr& expr_;
};

// Log函数
template <typename Expr>
class LogExpr {
public:
    using ValueType = typename Expr::ValueType;
    
    explicit LogExpr(const Expr& expr) : expr_(expr) {}

    // 获取表达式的值
    ValueType operator[](size_t i) const {
        return std::log(expr_[i]);
    }

    // 获取表达式的大小
    size_t size() const {
        return expr_.size();
    }

private:
    const Expr& expr_;
};

// Sqrt函数
template <typename Expr>
class SqrtExpr {
public:
    using ValueType = typename Expr::ValueType;
    
    explicit SqrtExpr(const Expr& expr) : expr_(expr) {}

    // 获取表达式的值
    ValueType operator[](size_t i) const {
        return std::sqrt(expr_[i]);
    }

    // 获取表达式的大小
    size_t size() const {
        return expr_.size();
    }

private:
    const Expr& expr_;
};

// Abs函数
template <typename Expr>
class AbsExpr {
public:
    using ValueType = typename Expr::ValueType;
    
    explicit AbsExpr(const Expr& expr) : expr_(expr) {}

    // 获取表达式的值
    ValueType operator[](size_t i) const {
        return std::abs(expr_[i]);
    }

    // 获取表达式的大小
    size_t size() const {
        return expr_.size();
    }

private:
    const Expr& expr_;
};

// 二元函数表达式模板
template <typename Expr1, typename Expr2, typename Func>
class BinaryExpr {
public:
    using ValueType = typename Expr1::ValueType;
    
    BinaryExpr(const Expr1& expr1, const Expr2& expr2, Func func) 
        : expr1_(expr1), expr2_(expr2), func_(func) {}

    // 获取表达式的值
    ValueType operator[](size_t i) const {
        return func_(expr1_[i], expr2_[i]);
    }

    // 获取表达式的大小
    size_t size() const {
        return expr1_.size();
    }

private:
    const Expr1& expr1_;
    const Expr2& expr2_;
    Func func_;
};

// Pow函数（二元）
template <typename Expr1, typename Expr2>
class PowExpr {
public:
    using ValueType = typename Expr1::ValueType;
    
    PowExpr(const Expr1& expr1, const Expr2& expr2) 
        : expr1_(expr1), expr2_(expr2) {}

    // 获取表达式的值
    ValueType operator[](size_t i) const {
        return std::pow(expr1_[i], expr2_[i]);
    }

    // 获取表达式的大小
    size_t size() const {
        return expr1_.size();
    }

private:
    const Expr1& expr1_;
    const Expr2& expr2_;
};

// 为Vector提供数学函数的接口
template <typename T, size_t N>
class Vector;

// 一元函数的接口函数
template <typename Expr>
SinExpr<Expr> sin(const Expr& expr) {
    return SinExpr<Expr>(expr);
}

template <typename Expr>
CosExpr<Expr> cos(const Expr& expr) {
    return CosExpr<Expr>(expr);
}

template <typename Expr>
TanExpr<Expr> tan(const Expr& expr) {
    return TanExpr<Expr>(expr);
}

template <typename Expr>
ExpExpr<Expr> exp(const Expr& expr) {
    return ExpExpr<Expr>(expr);
}

template <typename Expr>
LogExpr<Expr> log(const Expr& expr) {
    return LogExpr<Expr>(expr);
}

template <typename Expr>
SqrtExpr<Expr> sqrt(const Expr& expr) {
    return SqrtExpr<Expr>(expr);
}

template <typename Expr>
AbsExpr<Expr> abs(const Expr& expr) {
    return AbsExpr<Expr>(expr);
}

// 二元函数的接口函数
template <typename Expr1, typename Expr2>
PowExpr<Expr1, Expr2> pow(const Expr1& expr1, const Expr2& expr2) {
    return PowExpr<Expr1, Expr2>(expr1, expr2);
}

#endif // MATH_FUNCTIONS_H
