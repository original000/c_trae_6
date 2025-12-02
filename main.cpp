#include <iostream>
#include <experimental/string_view>
#include <array>
#include <type_traits>
#include <cctype>
#include <cstring>
#include <vector>
#include <string>
#include "suffix_automaton.h"

// 为了兼容性，定义std::string_view为std::experimental::string_view
namespace std {
    using string_view = experimental::string_view;
}

// 模板元编程辅助函数：检查一个类型是否是std::array
template <typename T>
struct is_std_array : std::false_type {};

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

// 解析器辅助函数：跳过空白字符
constexpr const char* skip_whitespace(const char* str) {
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        ++str;
    }
    return str;
}

// 解析器辅助函数：解析字符串
constexpr const char* parse_string(const char* str, const char*& out_start, std::size_t& out_length) {
    if (*str != '"') {
        return nullptr; // 不是字符串
    }
    ++str; // 跳过开头的"
    out_start = str;
    out_length = 0;
    while (*str != '"' && *str != '\0') {
        if (*str == '\\') {
            ++str; // 跳过转义字符
        }
        ++str;
        ++out_length;
    }
    if (*str != '"') {
        return nullptr; // 字符串未结束
    }
    return str + 1; // 跳过结尾的"
}

// 解析string值
template <typename T>
constexpr typename std::enable_if<std::is_same<T, std::string_view>::value, const char*>::type
parse_json_value(const char* str, T& out) {
    const char* start = nullptr;
    std::size_t length = 0;
    const char* result = parse_string(str, start, length);
    out = std::string_view(start, length);
    return result;
}

// 解析器辅助函数：检查是否为数字字符（constexpr版本）
constexpr bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

// 解析器辅助函数：解析数字（支持整数）
constexpr const char* parse_number(const char* str, int& out) {
    out = 0;
    bool negative = false;
    if (*str == '-') {
        negative = true;
        ++str;
    }
    if (!is_digit(*str)) {
        return nullptr; // 不是数字
    }
    while (is_digit(*str)) {
        out = out * 10 + (*str - '0');
        ++str;
    }
    if (negative) {
        out = -out;
    }
    return str;
}

// 解析器辅助函数：比较两个字符串是否相等（constexpr版本）
constexpr bool str_equal(const char* a, const char* b) {
    while (*a != '\0' && *b != '\0') {
        if (*a != *b) {
            return false;
        }
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

// 解析器辅助函数：解析bool值
constexpr const char* parse_bool(const char* str, bool& out) {
    if (str_equal(str, "true")) {
        out = true;
        return str + 4;
    } else if (str_equal(str, "false")) {
        out = false;
        return str + 5;
    }
    return nullptr; // 不是bool值
}

// 解析器辅助函数：解析null值
constexpr const char* parse_null(const char* str) {
    if (str_equal(str, "null")) {
        return str + 4;
    }
    return nullptr; // 不是null值
}

// 解析器辅助函数：比较两个字符串是否相等（constexpr版本）
constexpr bool string_view_equal(const char* a, std::size_t a_len, const char* b) {
    std::size_t b_len = 0;
    while (b[b_len] != '\0') {
        ++b_len;
    }
    if (a_len != b_len) {
        return false;
    }
    for (std::size_t i = 0; i < a_len; ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

// 解析器辅助函数：比较两个string_view是否相等（constexpr版本）
constexpr bool string_view_equal(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

// 解析器辅助函数：比较string_view和const char*是否相等（constexpr版本）
constexpr bool string_view_equal(std::string_view a, const char* b) {
    std::size_t b_len = 0;
    while (b[b_len] != '\0') {
        ++b_len;
    }
    if (a.size() != b_len) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

// 解析器核心：解析JSON值
// 使用SFINAE和模板元编程来实现编译期的类型分支

// 解析null值
template <typename T>
constexpr typename std::enable_if<std::is_same<T, decltype(nullptr)>::value, const char*>::type
parse_json_value(const char* str, T& out) {
    const char* result = parse_null(str);
    return result;
}

// 解析bool值
template <typename T>
constexpr typename std::enable_if<std::is_same<T, bool>::value, const char*>::type
parse_json_value(const char* str, T& out) {
    const char* result = parse_bool(str, out);
    return result;
}

// 解析int值
template <typename T>
constexpr typename std::enable_if<std::is_same<T, int>::value, const char*>::type
parse_json_value(const char* str, T& out) {
    const char* result = parse_number(str, out);
    return result;
}



// 解析array值（使用std::array）
template <typename T, std::size_t N>
constexpr const char* parse_json_array(const char* str, std::array<T, N>& out) {
    str = skip_whitespace(str);
    if (*str != '[') {
        return nullptr; // 不是数组
    }
    ++str; // 跳过开头的[
    str = skip_whitespace(str);

    // 解析第一个元素
    if (N >= 1) {
        str = parse_json_value(str, out[0]);
        if (str == nullptr) {
            return nullptr; // 解析数组元素失败
        }
        str = skip_whitespace(str);
        if (N > 1) {
            if (*str != ',') {
                return nullptr; // 数组元素之间缺少逗号
            }
            ++str; // 跳过逗号
            str = skip_whitespace(str);
        } else {
            if (*str != ']') {
                return nullptr; // 数组未结束
            }
        }
    }

    // 解析第二个元素
    if (N >= 2) {
        str = parse_json_value(str, out[1]);
        if (str == nullptr) {
            return nullptr; // 解析数组元素失败
        }
        str = skip_whitespace(str);
        if (N > 2) {
            if (*str != ',') {
                return nullptr; // 数组元素之间缺少逗号
            }
            ++str; // 跳过逗号
            str = skip_whitespace(str);
        } else {
            if (*str != ']') {
                return nullptr; // 数组未结束
            }
        }
    }

    // 解析第三个元素
    if (N >= 3) {
        str = parse_json_value(str, out[2]);
        if (str == nullptr) {
            return nullptr; // 解析数组元素失败
        }
        str = skip_whitespace(str);
        if (N > 3) {
            if (*str != ',') {
                return nullptr; // 数组元素之间缺少逗号
            }
            ++str; // 跳过逗号
            str = skip_whitespace(str);
        } else {
            if (*str != ']') {
                return nullptr; // 数组未结束
            }
        }
    }

    // 解析第四个元素
    if (N >= 4) {
        str = parse_json_value(str, out[3]);
        if (str == nullptr) {
            return nullptr; // 解析数组元素失败
        }
        str = skip_whitespace(str);
        if (N > 4) {
            if (*str != ',') {
                return nullptr; // 数组元素之间缺少逗号
            }
            ++str; // 跳过逗号
            str = skip_whitespace(str);
        } else {
            if (*str != ']') {
                return nullptr; // 数组未结束
            }
        }
    }

    // 解析第五个元素
    if (N >= 5) {
        str = parse_json_value(str, out[4]);
        if (str == nullptr) {
            return nullptr; // 解析数组元素失败
        }
        str = skip_whitespace(str);
        if (N > 5) {
            if (*str != ',') {
                return nullptr; // 数组元素之间缺少逗号
            }
            ++str; // 跳过逗号
            str = skip_whitespace(str);
        } else {
            if (*str != ']') {
                return nullptr; // 数组未结束
            }
        }
    }

    // 检查数组是否结束
    if (*str != ']') {
        return nullptr; // 数组未结束
    }

    return str + 1; // 跳过结尾的]
}

// 解析array值的SFINAE版本
template <typename T>
constexpr typename std::enable_if<is_std_array<T>::value, const char*>::type
parse_json_value(const char* str, T& out) {
    const char* result = parse_json_array(str, out);
    return result;
}

// 解析object值的辅助函数
template <typename T>
constexpr const char* parse_json_object(const char* str, T& out) {
    str = skip_whitespace(str);
    if (*str != '{') {
        return nullptr; // 不是对象
    }
    ++str; // 跳过开头的{
    str = skip_whitespace(str);

    // 解析name字段
    const char* name_start = nullptr;
    std::size_t name_length = 0;
    str = parse_string(str, name_start, name_length);
    if (str == nullptr) {
        return nullptr; // 解析字段名失败
    }
    if (!string_view_equal(name_start, name_length, "name")) {
        return nullptr; // 未知字段
    }
    str = skip_whitespace(str);
    if (*str != ':') {
        return nullptr; // 字段名后缺少冒号
    }
    ++str; // 跳过冒号
    str = skip_whitespace(str);
    str = parse_json_value(str, out.name);
    if (str == nullptr) {
        return nullptr; // 解析字段值失败
    }
    str = skip_whitespace(str);
    if (*str == ',') {
        ++str; // 跳过逗号
        str = skip_whitespace(str);
    } else if (*str != '}') {
        return nullptr; // 字段之间缺少逗号
    }

    // 解析port字段
    const char* port_start = nullptr;
    std::size_t port_length = 0;
    str = parse_string(str, port_start, port_length);
    if (str == nullptr) {
        return nullptr; // 解析字段名失败
    }
    if (!string_view_equal(port_start, port_length, "port")) {
        return nullptr; // 未知字段
    }
    str = skip_whitespace(str);
    if (*str != ':') {
        return nullptr; // 字段名后缺少冒号
    }
    ++str; // 跳过冒号
    str = skip_whitespace(str);
    str = parse_json_value(str, out.port);
    if (str == nullptr) {
        return nullptr; // 解析字段值失败
    }
    str = skip_whitespace(str);
    if (*str == ',') {
        ++str; // 跳过逗号
        str = skip_whitespace(str);
    } else if (*str != '}') {
        return nullptr; // 字段之间缺少逗号
    }

    // 解析debug字段
    const char* debug_start = nullptr;
    std::size_t debug_length = 0;
    str = parse_string(str, debug_start, debug_length);
    if (str == nullptr) {
        return nullptr; // 解析字段名失败
    }
    if (!string_view_equal(debug_start, debug_length, "debug")) {
        return nullptr; // 未知字段
    }
    str = skip_whitespace(str);
    if (*str != ':') {
        return nullptr; // 字段名后缺少冒号
    }
    ++str; // 跳过冒号
    str = skip_whitespace(str);
    str = parse_json_value(str, out.debug);
    if (str == nullptr) {
        return nullptr; // 解析字段值失败
    }
    str = skip_whitespace(str);
    if (*str == ',') {
        ++str; // 跳过逗号
        str = skip_whitespace(str);
    } else if (*str != '}') {
        return nullptr; // 字段之间缺少逗号
    }

    // 解析tags字段
    const char* tags_start = nullptr;
    std::size_t tags_length = 0;
    str = parse_string(str, tags_start, tags_length);
    if (str == nullptr) {
        return nullptr; // 解析字段名失败
    }
    if (!string_view_equal(tags_start, tags_length, "tags")) {
        return nullptr; // 未知字段
    }
    str = skip_whitespace(str);
    if (*str != ':') {
        return nullptr; // 字段名后缺少冒号
    }
    ++str; // 跳过冒号
    str = skip_whitespace(str);
    str = parse_json_value(str, out.tags);
    if (str == nullptr) {
        return nullptr; // 解析字段值失败
    }
    str = skip_whitespace(str);
    if (*str != '}') {
        return nullptr; // 对象未结束
    }

    return str + 1; // 跳过结尾的}
}

// 解析object值
template <typename T>
constexpr typename std::enable_if<!is_std_array<T>::value && std::is_class<T>::value && !std::is_same<T, std::string_view>::value, const char*>::type
parse_json_value(const char* str, T& out) {
    const char* result = parse_json_object(str, out);
    return result;
}

// JSON解析器入口
template <typename T, std::size_t N>
constexpr T parse_json(const char (&str)[N]) {
    T result{}; // 使用值初始化确保所有成员都被初始化
    const char* end = parse_json_value(str, result);
    end = skip_whitespace(end);
    return result;
}

// 示例：用户定义的配置struct
struct Config {
    std::string_view name;
    int port;
    bool debug;
    std::array<std::string_view, 2> tags;
    
    // 提供constexpr构造函数确保所有成员都被初始化
    constexpr Config() : name{}, port{}, debug{}, tags{} {}
};

int main() {
    // Test suffix automaton functionality
    std::cout << "--- Suffix Automaton Functionality Test ---\n";

    // Create a test string
    std::string test_str = "ababaabab";
    std::cout << "Original string: " << test_str << std::endl;

    // Build suffix automaton
    SuffixAutomaton sa(test_str);
    std::cout << "Number of states in suffix automaton: " << sa.get_size() << std::endl;

    // Test substring containment
    std::vector<std::string> test_substrings = {"aba", "abab", "bab", "abc", "aabab"};
    for (const auto& substr : test_substrings) {
        bool contains = sa.contains(substr);
        std::cout << "Contains substring \"" << substr << "\": " << (contains ? "Yes" : "No") << std::endl;
    }

    // Test finding all occurrences
    std::vector<std::string> test_occurrences = {"aba", "ab", "b"};
    for (const auto& substr : test_occurrences) {
        std::vector<int> occurrences = sa.find_all_occurrences(substr);
        std::cout << "Occurrences of substring \"" << substr << "\": ";
        if (occurrences.empty()) {
            std::cout << "None";
        } else {
            for (size_t i = 0; i < occurrences.size(); ++i) {
                if (i > 0) {
                    std::cout << ", ";
                }
                std::cout << occurrences[i];
            }
        }
        std::cout << std::endl;
    }

    return 0;
}
