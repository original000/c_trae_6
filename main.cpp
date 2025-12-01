#include <iostream>
#include <chrono>
#include "VectorExpr.h"

using namespace std;
using namespace std::chrono;

// 定义向量大小
constexpr size_t VECTOR_SIZE = 1000;

int main() {
    // 创建测试向量
    Vector<float, VECTOR_SIZE> a, b, c, d, e, result;

    // 初始化向量
    for (size_t i = 0; i < VECTOR_SIZE; ++i) {
        a[i] = static_cast<float>(i) / VECTOR_SIZE;
        b[i] = static_cast<float>(i * i) / VECTOR_SIZE;
        c[i] = static_cast<float>(i + 1) / VECTOR_SIZE;
        d[i] = static_cast<float>(i * 2) / VECTOR_SIZE;
        e[i] = static_cast<float>(i * 3.14159) / VECTOR_SIZE;
    }

    // 输出初始化结果（前5个元素）
    cout << "初始化结果（前5个元素）:" << endl;
    for (size_t i = 0; i < 5; ++i) {
        cout << "a[" << i << "] = " << a[i] << ", b[" << i << "] = " << b[i] << endl;
        cout << "c[" << i << "] = " << c[i] << ", d[" << i << "] = " << d[i] << endl;
        cout << "e[" << i << "] = " << e[i] << endl;
    }

    // 性能测试：执行100次计算
    const int iterations = 100;
    auto start = high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        // 表达式模板会自动融合为一个循环
        result = (a + b) * (c - d) + sin(e);
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    // 输出性能结果
    cout << "执行" << iterations << "次计算的总时间: " << duration.count() << " 毫秒" << endl;
    cout << "平均每次计算时间: " << static_cast<double>(duration.count()) / iterations << " 毫秒" << endl;

    // 验证计算结果（检查前几个元素）
    cout << endl << "验证计算结果（前5个元素）:" << endl;
    for (size_t i = 0; i < 5; ++i) {
        float expected = (a[i] + b[i]) * (c[i] - d[i]) + sin(e[i]);
        cout << "result[" << i << "] = " << result[i] << ", expected = " << expected << endl;
    }

    // 测试其他功能：广播、切片、reduce
    cout << endl << "测试其他功能:" << endl;

    // 广播测试
    Vector<float, VECTOR_SIZE> broadcast_result = a + 1.0f;
    cout << "广播测试: a[0] + 1.0 = " << broadcast_result[0] << endl;

    // 切片测试
    constexpr size_t SliceStart = 10;
    constexpr size_t SliceLength = 5;
    auto sliced = slice<SliceStart, SliceLength>(a);
    cout << "切片测试: a[10..14] = ";
    for (size_t i = 0; i < SliceLength; ++i) {
        cout << sliced[i] << " ";
    }
    cout << endl;

    // Reduce测试
    float sum = a.sum();
    float mean = a.mean();
    float max_val = a.max();
    float min_val = a.min();
    cout << "Reduce测试: sum = " << sum << ", mean = " << mean 
         << ", max = " << max_val << ", min = " << min_val << endl;

    return 0;
}
