#include "BitonicSorter.h"
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>

int main() {
    const size_t N = 16; // 2^4个元素
    
    // 生成随机double数组
    std::vector<double> arr(N);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    
    for (size_t i = 0; i < N; ++i) {
        arr[i] = dis(gen);
    }
    
    // 打印原始数组
    std::cout << "Original array: " << std::endl;
    for (const auto& elem : arr) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;
    
    // 使用BitonicSorter排序
    std::vector<double> arr_bitonic = arr;
    auto start_bitonic = std::chrono::high_resolution_clock::now();
    BitonicSort::BitonicSorter<N>::sort(arr_bitonic);
    auto end_bitonic = std::chrono::high_resolution_clock::now();
    auto duration_bitonic = std::chrono::duration_cast<std::chrono::nanoseconds>(end_bitonic - start_bitonic).count();
    
    // 使用std::sort排序
    std::vector<double> arr_std = arr;
    auto start_std = std::chrono::high_resolution_clock::now();
    std::sort(arr_std.begin(), arr_std.end());
    auto end_std = std::chrono::high_resolution_clock::now();
    auto duration_std = std::chrono::duration_cast<std::chrono::nanoseconds>(end_std - start_std).count();
    
    // 打印Bitonic排序结果
    std::cout << "Bitonic sorted array: " << std::endl;
    for (const auto& elem : arr_bitonic) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;
    
    // 打印std::sort排序结果
    std::cout << "std::sort sorted array: " << std::endl;
    for (const auto& elem : arr_std) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;
    
    // 比较两个排序结果是否相同
    if (arr_bitonic == arr_std) {
        std::cout << "Sorting results are identical!" << std::endl;
    } else {
        std::cout << "Sorting results are different!" << std::endl;
    }
    
    // 打印排序时间
    std::cout << "Bitonic sort time: " << duration_bitonic << " ns" << std::endl;
    std::cout << "std::sort time: " << duration_std << " ns" << std::endl;
    
    return 0;
}