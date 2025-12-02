#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include "compressor.cpp"

using namespace std;

// 计算压缩率
double calculateCompressionRatio(const string& original, const vector<uint8_t>& compressed) {
    double originalSize = original.size() * 8.0; // 转换为比特
    double compressedSize = compressed.size() * 8.0;
    return (1.0 - (compressedSize / originalSize)) * 100.0;
}

int main() {
    // 使用较小的测试数据
