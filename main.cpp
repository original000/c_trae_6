#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <windows.h> // 用于设置控制台编码

class StringAlgo {
public:
    // Manacher算法：返回奇偶回文半径数组
    static std::pair<std::vector<int>, std::vector<int>> manacher(const std::string& s) {
        int n = s.size();
        std::vector<int> odd(n), even(n);

        // 处理奇数长度回文
        for (int i = 0, l = 0, r = -1; i < n; ++i) {
            int k = (i > r) ? 1 : std::min(odd[l + r - i], r - i + 1);
            while (i - k >= 0 && i + k < n && s[i - k] == s[i + k]) {
                k++;
            }
            odd[i] = k--;
            if (i + k > r) {
                l = i - k;
                r = i + k;
            }
        }

        // 处理偶数长度回文
        for (int i = 0, l = 0, r = -1; i < n; ++i) {
            int k = (i > r) ? 0 : std::min(even[l + r - i + 1], r - i + 1);
            while (i - k - 1 >= 0 && i + k < n && s[i - k - 1] == s[i + k]) {
                k++;
            }
            even[i] = k--;
            if (i + k > r) {
                l = i - k - 1;
                r = i + k;
            }
        }

        return {odd, even};
    }

    // Z函数：返回Z数组
    static std::vector<int> z_function(const std::string& s) {
        int n = s.size();
        std::vector<int> z(n, 0);
        for (int i = 1, l = 0, r = 0; i < n; ++i) {
            if (i <= r) {
                z[i] = std::min(r - i + 1, z[i - l]);
            }
            while (i + z[i] < n && s[z[i]] == s[i + z[i]]) {
                z[i]++;
            }
            if (i + z[i] - 1 > r) {
                l = i;
                r = i + z[i] - 1;
            }
        }
        z[0] = n; // 第一个元素是字符串长度
        return z;
    }

    // KMP算法：返回增强版π数组（支持最小周期）
    static std::vector<int> kmp_pi(const std::string& s) {
        int n = s.size();
        std::vector<int> pi(n, 0);
        for (int i = 1; i < n; ++i) {
            int j = pi[i - 1];
            while (j > 0 && s[i] != s[j]) {
                j = pi[j - 1];
            }
            if (s[i] == s[j]) {
                j++;
            }
            pi[i] = j;
        }
        return pi;
    }

    // 生成所有回文子串的生成器
    static void all_palindromic_substrings(const std::string& s, std::vector<std::string>& result) {
        std::pair<std::vector<int>, std::vector<int>> manacher_result = manacher(s);
        const std::vector<int>& odd = manacher_result.first;
        const std::vector<int>& even = manacher_result.second;
        int n = s.size();

        // 处理奇数长度回文
        for (int i = 0; i < n; ++i) {
            for (int k = 1; k <= odd[i]; ++k) {
                int len = 2 * k - 1;
                int start = i - k + 1;
                result.push_back(s.substr(start, len));
            }
        }

        // 处理偶数长度回文
        for (int i = 0; i < n; ++i) {
            for (int k = 1; k <= even[i]; ++k) {
                int len = 2 * k;
                int start = i - k;
                result.push_back(s.substr(start, len));
            }
        }

        // 去重并排序
        std::sort(result.begin(), result.end());
        auto last = std::unique(result.begin(), result.end());
        result.erase(last, result.end());
    }

    // Booth算法：返回最小字典序旋转的起点
    static int minimal_rotation(const std::string& s) {
        std::string t = s + s;
        int n = s.size();
        int i = 0, ans = 0;
        while (i < n) {
            ans = i;
            int j = i + 1, k = i;
            while (j < 2 * n && t[k] <= t[j]) {
                if (t[k] < t[j]) {
                    k = i;
                } else {
                    k++;
                }
                j++;
            }
            while (i <= k) {
                i += j - k;
            }
        }
        return ans;
    }
};

// 经典例题1：最长回文子串
std::string longest_palindromic_substring(const std::string& s) {
    if (s.empty()) return "";
    std::pair<std::vector<int>, std::vector<int>> manacher_result = StringAlgo::manacher(s);
    const std::vector<int>& odd = manacher_result.first;
    const std::vector<int>& even = manacher_result.second;
    int n = s.size();
    int max_len = 1;
    int start = 0;

    // 检查奇数长度回文
    for (int i = 0; i < n; ++i) {
        int len = 2 * odd[i] - 1;
        if (len > max_len) {
            max_len = len;
            start = i - odd[i] + 1;
        }
    }

    // 检查偶数长度回文
    for (int i = 0; i < n; ++i) {
        int len = 2 * even[i];
        if (len > max_len) {
            max_len = len;
            start = i - even[i];
        }
    }

    return s.substr(start, max_len);
}

// 经典例题2：字符串匹配（KMP算法）
std::vector<int> kmp_search(const std::string& text, const std::string& pattern) {
    std::vector<int> result;
    if (pattern.empty() || text.size() < pattern.size()) return result;

    std::string s = pattern + "#" + text;
    std::vector<int> pi = StringAlgo::kmp_pi(s);
    int m = pattern.size();
    int n = s.size();

    for (int i = m + 1; i < n; ++i) {
        if (pi[i] == m) {
            result.push_back(i - 2 * m);
        }
    }

    return result;
}

int main() {
    // 设置控制台编码为UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::cout << "=== StringAlgo 算法全家桶演示 ===\n\n";

    // 演示Manacher算法
    std::string s1 = "abbaabba";
    std::pair<std::vector<int>, std::vector<int>> manacher_result1 = StringAlgo::manacher(s1);
    const std::vector<int>& odd1 = manacher_result1.first;
    const std::vector<int>& even1 = manacher_result1.second;
    std::cout << "1. Manacher算法演示（字符串：\"" << s1 << "\"）：\n";
    std::cout << "   奇数回文半径数组：";
    for (int x : odd1) std::cout << x << " ";
    std::cout << "\n   偶数回文半径数组：";
    for (int x : even1) std::cout << x << " ";
    std::cout << "\n\n";

    // 演示Z函数
    std::string s2 = "abacabaabacaba";
    std::vector<int> z2 = StringAlgo::z_function(s2);
    std::cout << "2. Z函数演示（字符串：\"" << s2 << "\"）：\n";
    std::cout << "   Z数组：";
    for (int x : z2) std::cout << x << " ";
    std::cout << "\n\n";

    // 演示KMP π数组
    std::string s3 = "ABABCABAB";
    std::vector<int> pi3 = StringAlgo::kmp_pi(s3);
    std::cout << "3. KMP π数组演示（字符串：\"" << s3 << "\"）：\n";
    std::cout << "   π数组：";
    for (int x : pi3) std::cout << x << " ";
    std::cout << "\n\n";

    // 演示所有回文子串
    std::string s4 = "abcba";
    std::vector<std::string> palindromes4;
    StringAlgo::all_palindromic_substrings(s4, palindromes4);
    std::cout << "4. 所有回文子串演示（字符串：\"" << s4 << "\"）：\n";
    std::cout << "   回文子串总数：" << palindromes4.size() << "\n";
    std::cout << "   回文子串列表：";
    for (const std::string& p : palindromes4) {
        std::cout << "\"" << p << "\" ";
    }
    std::cout << "\n\n";

    // 演示最小字典序旋转
    std::string s5 = "cbaab";
    int min_rot5 = StringAlgo::minimal_rotation(s5);
    std::cout << "5. 最小字典序旋转演示（字符串：\"" << s5 << "\"）：\n";
    std::cout << "   最小旋转起点：" << min_rot5 << "\n";
    std::cout << "   最小旋转结果：\"" << s5.substr(min_rot5) + s5.substr(0, min_rot5) << "\"\n\n";

    // 经典例题1：最长回文子串
    std::string s6 = "babad";
    std::string longest_pal6 = longest_palindromic_substring(s6);
    std::cout << "=== 经典例题1：最长回文子串 ===\n";
    std::cout << "输入字符串：\"" << s6 << "\"\n";
    std::cout << "最长回文子串：\"" << longest_pal6 << "\"\n\n";

    // 经典例题2：字符串匹配（KMP算法）
    std::string text7 = "ABABDABACDABABCABAB";
    std::string pattern7 = "ABABCABAB";
    std::vector<int> matches7 = kmp_search(text7, pattern7);
    std::cout << "=== 经典例题2：字符串匹配（KMP算法） ===\n";
    std::cout << "文本字符串：\"" << text7 << "\"\n";
    std::cout << "模式字符串：\"" << pattern7 << "\"\n";
    std::cout << "匹配位置总数：" << matches7.size() << "\n";
    std::cout << "匹配位置列表：";
    for (int pos : matches7) {
        std::cout << pos << " ";
    }
    std::cout << "\n\n";

    std::cout << "=== 所有算法演示完成！ ===\n";

    return 0;
}