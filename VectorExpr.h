#ifndef VECTOR_EXPR_H
#define VECTOR_EXPR_H

#include <cstddef>
#include <type_traits>
#include <algorithm>
#include "MathFunctions.h"

// 表达式模板基类
template <typename Expr>
class ExprBase {
public:
    // 获取表达式的大小
    size_t size() const {
        return static_cast<const Expr*>(this)->size();
    }

    // 转换为Expr类型
    const Expr& asExpr() const {
        return static_cast<const Expr&>(*this);
    }
};

// 向量类
template <typename T, size_t N>
class Vector {
public:
    static constexpr size_t Size = N;
    using ValueType = T;

    // 默认构造函数
    Vector() = default;

    // 构造函数：使用初始化列表
    Vector(std::initializer_list<T> init) {
        size_t i = 0;
        for (auto val : init) {
            if (i < N) {
                data_[i] = val;
                ++i;
            }
        }
    }

    // 构造函数：从表达式构造
    template <typename Expr>
    Vector(const Expr& expr) {
        static_assert(std::is_same<typename Expr::ValueType, T>::value, "Type mismatch");
        static_assert(Expr::Size == N, "Size mismatch");

        // 优化的循环赋值，支持循环展开和预取
        constexpr size_t UnrollFactor = 4;
        size_t i = 0;

        // 处理对齐的部分
        for (; i + UnrollFactor <= N; i += UnrollFactor) {
            // 预取下一组数据
            __builtin_prefetch(&data_[i + UnrollFactor], 1, 0);
            __builtin_prefetch(&data_[i + UnrollFactor + 1], 1, 0);
            __builtin_prefetch(&data_[i + UnrollFactor + 2], 1, 0);
            __builtin_prefetch(&data_[i + UnrollFactor + 3], 1, 0);

            // 循环展开赋值
            data_[i] = expr[i];
            data_[i + 1] = expr[i + 1];
            data_[i + 2] = expr[i + 2];
            data_[i + 3] = expr[i + 3];
        }

        // 处理剩余部分
        for (; i < N; ++i) {
            data_[i] = expr[i];
        }
    }

    // 获取向量的值
    T operator[](size_t i) const {
        return data_[i];
    }

    // 获取向量的引用
    T& operator[](size_t i) {
        return data_[i];
    }

    // 获取向量的大小
    size_t size() const {
        return N;
    }

    // 向量赋值运算符：从表达式赋值
    template <typename Expr>
    Vector<T, N>& operator=(const Expr& expr) {
        static_assert(std::is_same<typename Expr::ValueType, T>::value, "Type mismatch");
        static_assert(Expr::Size == N, "Size mismatch");

        // 优化的循环赋值，支持循环展开和预取
        constexpr size_t UnrollFactor = 4;
        size_t i = 0;

        // 处理对齐的部分
        for (; i + UnrollFactor <= N; i += UnrollFactor) {
            // 预取下一组数据
            __builtin_prefetch(&data_[i + UnrollFactor], 1, 0);
            __builtin_prefetch(&data_[i + UnrollFactor + 1], 1, 0);
            __builtin_prefetch(&data_[i + UnrollFactor + 2], 1, 0);
            __builtin_prefetch(&data_[i + UnrollFactor + 3], 1, 0);

            // 循环展开赋值
            data_[i] = expr[i];
            data_[i + 1] = expr[i + 1];
            data_[i + 2] = expr[i + 2];
            data_[i + 3] = expr[i + 3];
        }

        // 处理剩余部分
        for (; i < N; ++i) {
            data_[i] = expr[i];
        }

        return *this;
    }

    // 向量加法赋值运算符
    template <typename Expr>
    Vector<T, N>& operator+=(const Expr& expr) {
        *this = *this + expr;
        return *this;
    }

    // 向量减法赋值运算符
    template <typename Expr>
    Vector<T, N>& operator-=(const Expr& expr) {
        *this = *this - expr;
        return *this;
    }

    // 向量乘法赋值运算符
    template <typename Expr>
    Vector<T, N>& operator*=(const Expr& expr) {
        *this = *this * expr;
        return *this;
    }

    // 向量除法赋值运算符
    template <typename Expr>
    Vector<T, N>& operator/=(const Expr& expr) {
        *this = *this / expr;
        return *this;
    }

    // Reduce操作：求和
    T sum() const {
        T result = T();
        constexpr size_t UnrollFactor = 4;
        size_t i = 0;

        // 处理对齐的部分
        for (; i + UnrollFactor <= N; i += UnrollFactor) {
            // 预取下一组数据
            __builtin_prefetch(&data_[i + UnrollFactor], 0, 0);
            __builtin_prefetch(&data_[i + UnrollFactor + 1], 0, 0);
            __builtin_prefetch(&data_[i + UnrollFactor + 2], 0, 0);
            __builtin_prefetch(&data_[i + UnrollFactor + 3], 0, 0);

            // 循环展开求和
            result += data_[i];
            result += data_[i + 1];
            result += data_[i + 2];
            result += data_[i + 3];
        }

        // 处理剩余部分
        for (; i < N; ++i) {
            result += data_[i];
        }

        return result;
    }

    // Reduce操作：求平均值
    T mean() const {
        return sum() / static_cast<T>(N);
    }

    // Reduce操作：求最大值
    T max() const {
        T result = data_[0];
        for (size_t i = 1; i < N; ++i) {
            if (data_[i] > result) {
                result = data_[i];
            }
        }
        return result;
    }

    // Reduce操作：求最小值
    T min() const {
        T result = data_[0];
        for (size_t i = 1; i < N; ++i) {
            if (data_[i] < result) {
                result = data_[i];
            }
        }
        return result;
    }

private:
    // 数据存储，确保对齐
    alignas(16) T data_[N];
};

// 二元运算符表达式模板：加法
template <typename Expr1, typename Expr2>
class AddExpr {
public:
    using ValueType = typename Expr1::ValueType;
    static constexpr size_t Size = Expr1::Size;

    AddExpr(const Expr1& expr1, const Expr2& expr2) 
        : expr1_(expr1), expr2_(expr2) {}

    ValueType operator[](size_t i) const {
        return expr1_[i] + expr2_[i];
    }

    size_t size() const {
        return Size;
    }

private:
    const Expr1& expr1_;
    const Expr2& expr2_;
};

// 二元运算符表达式模板：减法
template <typename Expr1, typename Expr2>
class SubExpr {
public:
    using ValueType = typename Expr1::ValueType;
    static constexpr size_t Size = Expr1::Size;

    SubExpr(const Expr1& expr1, const Expr2& expr2) 
        : expr1_(expr1), expr2_(expr2) {}

    ValueType operator[](size_t i) const {
        return expr1_[i] - expr2_[i];
    }

    size_t size() const {
        return Size;
    }

private:
    const Expr1& expr1_;
    const Expr2& expr2_;
};

// 二元运算符表达式模板：乘法
template <typename Expr1, typename Expr2>
class MulExpr {
public:
    using ValueType = typename Expr1::ValueType;
    static constexpr size_t Size = Expr1::Size;

    MulExpr(const Expr1& expr1, const Expr2& expr2) 
        : expr1_(expr1), expr2_(expr2) {}

    ValueType operator[](size_t i) const {
        return expr1_[i] * expr2_[i];
    }

    size_t size() const {
        return Size;
    }

private:
    const Expr1& expr1_;
    const Expr2& expr2_;
};

// 二元运算符表达式模板：除法
template <typename Expr1, typename Expr2>
class DivExpr {
public:
    using ValueType = typename Expr1::ValueType;
    static constexpr size_t Size = Expr1::Size;

    DivExpr(const Expr1& expr1, const Expr2& expr2) 
        : expr1_(expr1), expr2_(expr2) {}

    ValueType operator[](size_t i) const {
        return expr1_[i] / expr2_[i];
    }

    size_t size() const {
        return Size;
    }

private:
    const Expr1& expr1_;
    const Expr2& expr2_;
};

// 广播表达式模板：将标量广播为向量
template <typename T, size_t N>
class BroadcastExpr {
public:
    using ValueType = T;
    static constexpr size_t Size = N;

    explicit BroadcastExpr(T value) : value_(value) {}

    ValueType operator[](size_t i) const {
        return value_;
    }

    size_t size() const {
        return N;
    }

private:
    T value_;
};

// 切片表达式模板：从向量中提取一部分
template <typename Expr, size_t Start, size_t Length>
class SliceExpr {
public:
    using ValueType = typename Expr::ValueType;
    static constexpr size_t Size = Length;

    explicit SliceExpr(const Expr& expr) : expr_(expr) {}

    ValueType operator[](size_t i) const {
        return expr_[Start + i];
    }

    size_t size() const {
        return Length;
    }

private:
    const Expr& expr_;
};

// 一元函数表达式模板
template <typename Expr, typename Func>
class UnaryExpr {
public:
    using ValueType = typename Expr::ValueType;
    static constexpr size_t Size = Expr::Size;

    explicit UnaryExpr(const Expr& expr, Func func) : expr_(expr), func_(func) {}

    // 获取表达式的值
    ValueType operator[](size_t i) const {
        return func_(expr_[i]);
    }

    // 获取表达式的大小
    size_t size() const {
        return expr_.size();
    }

private:
    const Expr& expr_;
    Func func_;
};

// 数学函数的表达式模板定义在MathFunctions.h中

// 数学函数的表达式模板定义在MathFunctions.h中

 // 二元运算符重载：加法（仅适用于两个表达式类）
 template <typename Expr1, typename Expr2, typename = typename std::enable_if<!std::is_arithmetic<Expr2>::value>::type>
 AddExpr<Expr1, Expr2> operator+(const Expr1& expr1, const Expr2& expr2) {
     return AddExpr<Expr1, Expr2>(expr1, expr2);
 }

 // 二元运算符重载：减法（仅适用于两个表达式类）
 template <typename Expr1, typename Expr2, typename = typename std::enable_if<!std::is_arithmetic<Expr2>::value>::type>
 SubExpr<Expr1, Expr2> operator-(const Expr1& expr1, const Expr2& expr2) {
     return SubExpr<Expr1, Expr2>(expr1, expr2);
 }

 // 二元运算符重载：乘法（仅适用于两个表达式类）
 template <typename Expr1, typename Expr2, typename = typename std::enable_if<!std::is_arithmetic<Expr2>::value>::type>
 MulExpr<Expr1, Expr2> operator*(const Expr1& expr1, const Expr2& expr2) {
     return MulExpr<Expr1, Expr2>(expr1, expr2);
 }

 // 二元运算符重载：除法（仅适用于两个表达式类）
 template <typename Expr1, typename Expr2, typename = typename std::enable_if<!std::is_arithmetic<Expr2>::value>::type>
 DivExpr<Expr1, Expr2> operator/(const Expr1& expr1, const Expr2& expr2) {
     return DivExpr<Expr1, Expr2>(expr1, expr2);
 }

 // 标量与向量的运算重载（仅适用于标量类型）

template <typename Expr, typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
AddExpr<Expr, BroadcastExpr<T, Expr::Size>> operator+(const Expr& expr, T value) {
    return AddExpr<Expr, BroadcastExpr<T, Expr::Size>>(expr, BroadcastExpr<T, Expr::Size>(value));
}

template <typename Expr, typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
AddExpr<BroadcastExpr<T, Expr::Size>, Expr> operator+(T value, const Expr& expr) {
    return AddExpr<BroadcastExpr<T, Expr::Size>, Expr>(BroadcastExpr<T, Expr::Size>(value), expr);
}

template <typename Expr, typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
SubExpr<Expr, BroadcastExpr<T, Expr::Size>> operator-(const Expr& expr, T value) {
    return SubExpr<Expr, BroadcastExpr<T, Expr::Size>>(expr, BroadcastExpr<T, Expr::Size>(value));
}

template <typename Expr, typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
SubExpr<BroadcastExpr<T, Expr::Size>, Expr> operator-(T value, const Expr& expr) {
    return SubExpr<BroadcastExpr<T, Expr::Size>, Expr>(BroadcastExpr<T, Expr::Size>(value), expr);
}

template <typename Expr, typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
MulExpr<Expr, BroadcastExpr<T, Expr::Size>> operator*(const Expr& expr, T value) {
    return MulExpr<Expr, BroadcastExpr<T, Expr::Size>>(expr, BroadcastExpr<T, Expr::Size>(value));
}

template <typename Expr, typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
MulExpr<BroadcastExpr<T, Expr::Size>, Expr> operator*(T value, const Expr& expr) {
    return MulExpr<BroadcastExpr<T, Expr::Size>, Expr>(BroadcastExpr<T, Expr::Size>(value), expr);
}

template <typename Expr, typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
DivExpr<Expr, BroadcastExpr<T, Expr::Size>> operator/(const Expr& expr, T value) {
    return DivExpr<Expr, BroadcastExpr<T, Expr::Size>>(expr, BroadcastExpr<T, Expr::Size>(value));
}

template <typename Expr, typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
DivExpr<BroadcastExpr<T, Expr::Size>, Expr> operator/(T value, const Expr& expr) {
    return DivExpr<BroadcastExpr<T, Expr::Size>, Expr>(BroadcastExpr<T, Expr::Size>(value), expr);
}

// 切片函数
template <size_t Start, size_t Length, typename Expr>
SliceExpr<Expr, Start, Length> slice(const Expr& expr) {
    static_assert(Start + Length <= Expr::Size, "Slice out of bounds");
    return SliceExpr<Expr, Start, Length>(expr);
}

// 矩阵类（简化版，支持基本的矩阵运算）
template <typename T, size_t Rows, size_t Cols>
class Matrix {
public:
    using ValueType = T;
    static constexpr size_t RowsCount = Rows;
    static constexpr size_t ColsCount = Cols;

    // 默认构造函数
    Matrix() = default;

    // 获取矩阵的行
    Vector<T, Cols> operator[](size_t i) const {
        return rows_[i];
    }

    // 获取矩阵的行引用
    Vector<T, Cols>& operator[](size_t i) {
        return rows_[i];
    }

    // 获取矩阵的行数
    size_t rows() const {
        return Rows;
    }

    // 获取矩阵的列数
    size_t cols() const {
        return Cols;
    }

private:
    // 矩阵存储为向量的数组
    Vector<T, Cols> rows_[Rows];
};

#endif // VECTOR_EXPR_H
