#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <cmath>
#include <iomanip>

using namespace std;

// 统一压缩接口
class Compressor {
public:
    virtual vector<uint8_t> compress(const string& input) = 0;
    virtual string decompress(const vector<uint8_t>& compressed) = 0;
    virtual ~Compressor() = default;
};

// LZ77 实现
class LZ77Compressor : public Compressor {
private:
    const int WINDOW_SIZE = 4096;
    const int LOOKAHEAD_SIZE = 18;

    struct LZ77Token {
        int offset;
        int length;
        char next_char;
    };

    vector<LZ77Token> encodeLZ77(const string& input) {
        vector<LZ77Token> tokens;
        int i = 0;
        int n = input.size();

        while (i < n) {
            int max_len = 0;
            int max_offset = 0;

            int start = max(0, i - WINDOW_SIZE);
            for (int j = start; j < i; ++j) {
                int len = 0;
                while (i + len < n && input[j + len] == input[i + len] && len < LOOKAHEAD_SIZE) {
                    len++;
                }
                if (len > max_len) {
                    max_len = len;
                    max_offset = i - j;
                }
            }

            char next_char = (i + max_len < n) ? input[i + max_len] : '\0';
            tokens.push_back({max_offset, max_len, next_char});
            i += max_len + 1;
        }

        return tokens;
    }

    string decodeLZ77(const vector<LZ77Token>& tokens) {
        string output;

        for (const auto& token : tokens) {
            if (token.length > 0) {
                int start = output.size() - token.offset;
                for (int i = 0; i < token.length; ++i) {
                    output += output[start + i];
                }
            }
            if (token.next_char != '\0') {
                output += token.next_char;
            }
        }

        return output;
    }

    // Huffman 编码辅助结构
    struct HuffmanNode {
        char data;
        int freq;
        HuffmanNode *left, *right;

        HuffmanNode(char data, int freq) : data(data), freq(freq), left(nullptr), right(nullptr) {}
    };

    struct Compare {
        bool operator()(HuffmanNode* a, HuffmanNode* b) {
            return a->freq > b->freq;
        }
    };

    void generateHuffmanCodes(HuffmanNode* root, string code, unordered_map<char, string>& huffmanCodes) {
        if (!root) return;

        if (!root->left && !root->right) {
            huffmanCodes[root->data] = code;
        }

        generateHuffmanCodes(root->left, code + "0", huffmanCodes);
        generateHuffmanCodes(root->right, code + "1", huffmanCodes);
    }

    HuffmanNode* buildHuffmanTree(unordered_map<char, int>& freq) {
        priority_queue<HuffmanNode*, vector<HuffmanNode*>, Compare> minHeap;

        for (auto& pair : freq) {
            minHeap.push(new HuffmanNode(pair.first, pair.second));
        }

        while (minHeap.size() > 1) {
            HuffmanNode* left = minHeap.top();
            minHeap.pop();

            HuffmanNode* right = minHeap.top();
            minHeap.pop();

            HuffmanNode* newNode = new HuffmanNode('$', left->freq + right->freq);
            newNode->left = left;
            newNode->right = right;
            minHeap.push(newNode);
        }

        return minHeap.top();
    }

    string huffmanEncode(const string& input, unordered_map<char, string>& huffmanCodes) {
        unordered_map<char, int> freq;
        for (char c : input) {
            freq[c]++;
        }

        HuffmanNode* root = buildHuffmanTree(freq);
        generateHuffmanCodes(root, "", huffmanCodes);

        string encoded;
        for (char c : input) {
            encoded += huffmanCodes[c];
        }

        // 释放内存
        deleteHuffmanTree(root);

        return encoded;
    }

    string huffmanDecode(const string& encoded, unordered_map<char, string>& huffmanCodes) {
        unordered_map<string, char> reverseHuffmanCodes;
        for (auto& pair : huffmanCodes) {
            reverseHuffmanCodes[pair.second] = pair.first;
        }

        string currentCode;
        string decoded;

        for (char bit : encoded) {
            currentCode += bit;
            if (reverseHuffmanCodes.find(currentCode) != reverseHuffmanCodes.end()) {
                decoded += reverseHuffmanCodes[currentCode];
                currentCode.clear();
            }
        }

        return decoded;
    }

    void deleteHuffmanTree(HuffmanNode* root) {
        if (!root) return;
        deleteHuffmanTree(root->left);
        deleteHuffmanTree(root->right);
        delete root;
    }

    // 将二进制字符串转换为字节数组
    vector<uint8_t> binaryStringToBytes(const string& binary) {
        vector<uint8_t> bytes;
        int n = binary.size();

        for (int i = 0; i < n; i += 8) {
            string byteStr = binary.substr(i, 8);
            // 如果最后一个字节不足8位，补0
            while (byteStr.size() < 8) {
                byteStr += '0';
            }
            uint8_t byte = stoi(byteStr, nullptr, 2);
            bytes.push_back(byte);
        }

        return bytes;
    }

    // 将字节数组转换为二进制字符串
    string bytesToBinaryString(const vector<uint8_t>& bytes) {
        string binary;

        for (uint8_t byte : bytes) {
            string byteStr;
            for (int i = 7; i >= 0; --i) {
                byteStr += ((byte >> i) & 1) ? '1' : '0';
            }
            binary += byteStr;
        }

        return binary;
    }

public:
    vector<uint8_t> compress(const string& input) override {
        // 第一步：LZ77 编码
        vector<LZ77Token> lz77Tokens = encodeLZ77(input);

        // 第二步：将 LZ77 令牌转换为字符串
        string lz77String;
        for (const auto& token : lz77Tokens) {
            // 使用特殊分隔符分隔三个字段
            lz77String += to_string(token.offset) + "|" + to_string(token.length) + "|" + token.next_char;
        }

        // 第三步：Huffman 编码
        unordered_map<char, string> huffmanCodes;
        string huffmanEncoded = huffmanEncode(lz77String, huffmanCodes);

        // 第四步：将 Huffman 编码和 Huffman 树结构转换为字节数组
        vector<uint8_t> compressed;

        // 保存 Huffman 树大小
        int huffmanSize = huffmanCodes.size();
        compressed.push_back((huffmanSize >> 8) & 0xFF);
        compressed.push_back(huffmanSize & 0xFF);

        // 保存 Huffman 树结构
        for (auto& pair : huffmanCodes) {
            compressed.push_back(pair.first);
            int codeLength = pair.second.size();
            compressed.push_back(codeLength);
            // 将 Huffman 编码转换为字节
            vector<uint8_t> codeBytes = binaryStringToBytes(pair.second);
            compressed.insert(compressed.end(), codeBytes.begin(), codeBytes.end());
        }

        // 保存 Huffman 编码的长度
        int encodedLength = huffmanEncoded.size();
        compressed.push_back((encodedLength >> 24) & 0xFF);
        compressed.push_back((encodedLength >> 16) & 0xFF);
        compressed.push_back((encodedLength >> 8) & 0xFF);
        compressed.push_back(encodedLength & 0xFF);

        // 保存 Huffman 编码的字节
        vector<uint8_t> encodedBytes = binaryStringToBytes(huffmanEncoded);
        compressed.insert(compressed.end(), encodedBytes.begin(), encodedBytes.end());

        return compressed;
    }

    string decompress(const vector<uint8_t>& compressed) override {
        if (compressed.empty()) return "";

        int index = 0;

        // 读取 Huffman 树大小
        int huffmanSize = ((compressed[index] << 8) | compressed[index + 1]);
        index += 2;

        // 读取 Huffman 树结构
        unordered_map<char, string> huffmanCodes;
        for (int i = 0; i < huffmanSize; ++i) {
            char data = compressed[index++];
            int codeLength = compressed[index++];
            int codeBytesLength = ceil(codeLength / 8.0);
            string code;
            for (int j = 0; j < codeBytesLength; ++j) {
                uint8_t byte = compressed[index++];
                string byteStr;
                for (int k = 7; k >= 0; --k) {
                    byteStr += ((byte >> k) & 1) ? '1' : '0';
                }
                code += byteStr;
            }
            // 截取实际长度的编码
            code = code.substr(0, codeLength);
            huffmanCodes[data] = code;
        }

        // 读取 Huffman 编码的长度
        int encodedLength = ((compressed[index] << 24) | (compressed[index + 1] << 16) |
                             (compressed[index + 2] << 8) | compressed[index + 3]);
        index += 4;

        // 读取 Huffman 编码的字节
        vector<uint8_t> encodedBytes(compressed.begin() + index, compressed.end());
        string huffmanEncoded = bytesToBinaryString(encodedBytes);
        // 截取实际长度的编码
        huffmanEncoded = huffmanEncoded.substr(0, encodedLength);

        // Huffman 解码
        string lz77String = huffmanDecode(huffmanEncoded, huffmanCodes);

        // LZ77 解码
        vector<LZ77Token> lz77Tokens;
        int pos = 0;
        while (pos < lz77String.size()) {
            // 解析 offset
            int offsetEnd = lz77String.find('|', pos);
            int offset = stoi(lz77String.substr(pos, offsetEnd - pos));
            pos = offsetEnd + 1;

            // 解析 length
            int lengthEnd = lz77String.find('|', pos);
            int length = stoi(lz77String.substr(pos, lengthEnd - pos));
            pos = lengthEnd + 1;

            // 解析 next_char
            char next_char = lz77String[pos];
            pos++;

            lz77Tokens.push_back({offset, length, next_char});
        }

        return decodeLZ77(lz77Tokens);
    }
};

// LZ78 实现
class LZ78Compressor : public Compressor {
public:
    vector<uint8_t> compress(const string& input) override {
        unordered_map<string, int> dictionary;
        vector<pair<int, char>> tokens;
        string current;

        // 初始化字典
        for (int i = 0; i < 256; ++i) {
            dictionary[string(1, static_cast<char>(i))] = i + 1;
        }
        int nextCode = 257;

        for (char c : input) {
            string temp = current + c;
            if (dictionary.find(temp) != dictionary.end()) {
                current = temp;
            } else {
                int code = dictionary[current];
                tokens.emplace_back(code, c);
                dictionary[temp] = nextCode++;
                current.clear();
            }
        }

        if (!current.empty()) {
            int code = dictionary[current];
            tokens.emplace_back(code, '\0');
        }

        // 将令牌转换为字节数组
        vector<uint8_t> compressed;
        for (const auto& token : tokens) {
            // 保存 code（2 字节）
            compressed.push_back((token.first >> 8) & 0xFF);
            compressed.push_back(token.first & 0xFF);
            // 保存 char（1 字节）
            compressed.push_back(static_cast<uint8_t>(token.second));
        }

        return compressed;
    }

    string decompress(const vector<uint8_t>& compressed) override {
        if (compressed.empty()) return "";

        unordered_map<int, string> dictionary;
        string output;

        // 初始化字典
        for (int i = 0; i < 256; ++i) {
            dictionary[i + 1] = string(1, static_cast<char>(i));
        }
        int nextCode = 257;

        int index = 0;
        int prevCode = ((compressed[index] << 8) | compressed[index + 1]);
        index += 2;
        char prevChar = static_cast<char>(compressed[index++]);
        output += dictionary[prevCode] + prevChar;

        while (index < compressed.size()) {
            int currentCode = ((compressed[index] << 8) | compressed[index + 1]);
            index += 2;
            char currentChar = static_cast<char>(compressed[index++]);

            string entry;
            if (dictionary.find(currentCode) != dictionary.end()) {
                entry = dictionary[currentCode];
            } else {
                entry = dictionary[prevCode] + prevChar;
            }
            output += entry + currentChar;

            dictionary[nextCode++] = dictionary[prevCode] + prevChar;
            prevCode = currentCode;
            prevChar = currentChar;
        }

        // 处理最后一个令牌（如果存在）
        if (prevCode != 0 && prevChar == '\0') {
            output += dictionary[prevCode];
        }

        return output;
    }
};

// LZW 实现
class LZWCompressor : public Compressor {
public:
    vector<uint8_t> compress(const string& input) override {
        unordered_map<string, int> dictionary;
        vector<int> codes;
        string current;

        // 初始化字典
        for (int i = 0; i < 256; ++i) {
            dictionary[string(1, static_cast<char>(i))] = i;
        }
        int nextCode = 256;

        for (char c : input) {
            string temp = current + c;
            if (dictionary.find(temp) != dictionary.end()) {
                current = temp;
            } else {
                codes.push_back(dictionary[current]);
                dictionary[temp] = nextCode++;
                current = string(1, c);
            }
        }

        if (!current.empty()) {
            codes.push_back(dictionary[current]);
        }

        // 将代码转换为字节数组（使用 12 位编码）
        vector<uint8_t> compressed;
        int bitBuffer = 0;
        int bitCount = 0;

        for (int code : codes) {
            bitBuffer = (bitBuffer << 12) | code;
            bitCount += 12;

            while (bitCount >= 8) {
                bitCount -= 8;
                uint8_t byte = (bitBuffer >> bitCount) & 0xFF;
                compressed.push_back(byte);
            }
        }

        // 处理剩余的位
        if (bitCount > 0) {
            bitBuffer <<= (8 - bitCount);
            uint8_t byte = bitBuffer & 0xFF;
            compressed.push_back(byte);
        }

        return compressed;
    }

    string decompress(const vector<uint8_t>& compressed) override {
        if (compressed.empty()) return "";

        unordered_map<int, string> dictionary;
        string output;

        // 初始化字典
        for (int i = 0; i < 256; ++i) {
            dictionary[i] = string(1, static_cast<char>(i));
        }
        int nextCode = 256;

        int bitBuffer = 0;
        int bitCount = 0;
        int index = 0;

        // 读取第一个代码
        while (bitCount < 12 && index < compressed.size()) {
            bitBuffer = (bitBuffer << 8) | compressed[index++];
            bitCount += 8;
        }
        int prevCode = (bitBuffer >> (bitCount - 12)) & 0xFFF;
        bitCount -= 12;
        output += dictionary[prevCode];

        while (index < compressed.size() || bitCount >= 12) {
            // 读取下一个代码
            while (bitCount < 12 && index < compressed.size()) {
                bitBuffer = (bitBuffer << 8) | compressed[index++];
                bitCount += 8;
            }

            if (bitCount < 12) break;

            int currentCode = (bitBuffer >> (bitCount - 12)) & 0xFFF;
            bitCount -= 12;

            string entry;
            if (dictionary.find(currentCode) != dictionary.end()) {
                entry = dictionary[currentCode];
            } else {
                entry = dictionary[prevCode] + dictionary[prevCode][0];
            }
            output += entry;

            dictionary[nextCode++] = dictionary[prevCode] + entry[0];
            prevCode = currentCode;
        }

        return output;
    }
};

// 计算压缩率
double calculateCompressionRatio(const string& original, const vector<uint8_t>& compressed) {
    double originalSize = original.size() * 8.0; // 转换为比特
    double compressedSize = compressed.size() * 8.0;
    return (1.0 - (compressedSize / originalSize)) * 100.0;
}
