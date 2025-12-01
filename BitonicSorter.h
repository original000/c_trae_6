#ifndef BITONIC_SORTER_H
#define BITONIC_SORTER_H

#include "ThreadPoolSimple.h"
#include <vector>
#include <functional>
#include <algorithm>

namespace BitonicSort {

// 生成比较交换网络的模板元编程实现
namespace detail {

// 迭代生成所有层的比较交换操作
template <size_t N, typename Compare = std::less<>>
struct BitonicNetwork {
    template <typename T>
    static void sort(std::vector<T>& arr, Compare comp = Compare()) {
        // 生成所有层的比较交换操作
        for (size_t k = 2; k <= N; k <<= 1) {
            // 生成当前层的比较交换操作
            for (size_t j = k >> 1; j > 0; j >>= 1) {
                for (size_t i = 0; i < N; ++i) {
                    size_t l = i ^ j;
                    if (l > i) {
                        // 升序比较：如果arr[i] > arr[l]，交换它们
                        if ((i & k) == 0 && comp(arr[l], arr[i])) {
                            std::swap(arr[i], arr[l]);
                        } else if ((i & k) != 0 && comp(arr[i], arr[l])) {
                            std::swap(arr[i], arr[l]);
                        }
                    }
                }
            }
        }
    }
};

} // namespace detail

// BitonicSorter类：提供静态sort方法
template <size_t N, typename Compare = std::less<>>
class BitonicSorter {
public:
    // 排序方法：接受一个vector和一个可选的比较器
    template <typename T>
    static void sort(std::vector<T>& arr, Compare comp = Compare()) {
        // 检查输入序列的长度是否为N
        if (arr.size() != N) {
            throw std::invalid_argument("Input sequence length must be " + std::to_string(N));
        }
        // 检查N是否为2的幂
        if ((N & (N - 1)) != 0) {
            throw std::invalid_argument("N must be a power of 2");
        }
        // 调用detail::BitonicNetwork::sort方法生成比较交换网络并执行排序
        detail::BitonicNetwork<N, Compare>::sort(arr, comp);
    }
};

} // namespace BitonicSort

#endif // BITONIC_SORTER_H