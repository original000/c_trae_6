#ifndef SUFFIX_AUTOMATON_H
#define SUFFIX_AUTOMATON_H

#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <numeric>
#include <climits>

struct State {
    int len;                // 状态的长度
    int link;               // 后缀链接
    std::unordered_map<char, int> next;  // 转移映射
    std::vector<int> endpos;  // 结束位置集合
    int cnt;                // 出现次数

    State() : len(0), link(-1), cnt(0) {}
};

class SuffixAutomaton {
public:
    SuffixAutomaton() : size(1), last(0) {
        states.push_back(State());
    }

    // 添加一个字符
    void extend(char c) {
        int p = last;
        int curr = size++;
        states.push_back(State());
        states[curr].len = states[last].len + 1;
        states[curr].endpos.push_back(states[curr].len - 1);  // 记录结束位置

        while (p != -1 && states[p].next.find(c) == states[p].next.end()) {
            states[p].next[c] = curr;
            p = states[p].link;
        }

        if (p == -1) {
            states[curr].link = 0;
        } else {
            int q = states[p].next[c];
            if (states[p].len + 1 == states[q].len) {
                states[curr].link = q;
            } else {
                int clone = size++;
                states.push_back(State());
                states[clone].len = states[p].len + 1;
                states[clone].next = states[q].next;
                states[clone].link = states[q].link;
                states[clone].endpos = states[q].endpos;

                while (p != -1 && states[p].next[c] == q) {
                    states[p].next[c] = clone;
                    p = states[p].link;
                }

                states[q].link = clone;
                states[curr].link = clone;
            }
        }

        last = curr;
    }

    // 添加一个字符串
    void add_string(const std::string& s) {
        for (char c : s) {
            extend(c);
        }
        // 添加一个特殊字符作为分隔符，避免不同字符串之间的干扰
        if (!s.empty()) {
            extend('$');
        }
    }

    // 构建后缀自动机（在添加所有字符串后调用）
    void build() {
        // 按长度排序状态
        order.resize(size);
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(), [this](int a, int b) {
            return states[a].len > states[b].len;
        });

        // 统计每个状态的出现次数
        count_occurrences();
    }

    // 统计每个状态的出现次数
    void count_occurrences() {
        // 初始化cnt数组
        for (int i = 0; i < size; ++i) {
            states[i].cnt = 0;
        }

        // 遍历所有状态，将endpos的大小作为初始cnt
        for (int i = 1; i < size; ++i) {  // 跳过初始状态
            if (!states[i].endpos.empty()) {
                states[i].cnt = 1;
            }
        }

        // 按长度从大到小更新cnt
        for (int v : order) {
            if (states[v].link != -1 && states[v].cnt > 0) {
                states[states[v].link].cnt += states[v].cnt;
            }
        }
    }

    // 查找模式串的所有出现位置
    std::vector<int> find_all_occurrences(const std::string& pattern) {
        std::vector<int> result;
        int current = 0;

        // 遍历模式串，找到对应的状态
        for (char c : pattern) {
            if (states[current].next.find(c) == states[current].next.end()) {
                return result;  // 模式串不存在
            }
            current = states[current].next[c];
        }

        // 收集所有出现位置
        collect_occurrences(current, pattern.length(), result);

        // 排序并去重
        std::sort(result.begin(), result.end());
        auto last = std::unique(result.begin(), result.end());
        result.erase(last, result.end());

        return result;
    }

    // 收集所有出现位置（递归辅助函数）
    void collect_occurrences(int state, int pattern_length, std::vector<int>& result) {
        // 添加当前状态的endpos
        for (int pos : states[state].endpos) {
            result.push_back(pos - pattern_length + 1);
        }

        // 遍历所有子状态（通过后缀链接）
        // 注意：这里不能直接遍历后缀链接，因为后缀链接可能指向父状态
        // 正确的做法是遍历所有通过next转移可以到达当前状态的状态
        // 但这会比较复杂，所以我们可以使用BFS或DFS来遍历所有相关状态

        // 使用BFS遍历所有相关状态
        std::vector<int> queue;
        queue.push_back(state);

        while (!queue.empty()) {
            int current = queue.back();
            queue.pop_back();

            // 遍历所有后缀链接
            int link = states[current].link;
            while (link != -1) {
                // 添加后缀链接状态的endpos
                for (int pos : states[link].endpos) {
                    result.push_back(pos - pattern_length + 1);
                }

                // 将后缀链接状态加入队列
                queue.push_back(link);

                // 继续遍历后缀链接
                link = states[link].link;
            }
        }
    }

    // 查找两个字符串的最长公共子串
    std::string longest_common_substring(const std::string& s1, const std::string& s2) {
        // 构建s1的后缀自动机
        SuffixAutomaton sa;
        sa.add_string(s1);
        sa.build();

        int current = 0;
        int max_len = 0;
        int end_pos = 0;
        int current_len = 0;

        // 遍历s2，找到最长公共子串
        for (int i = 0; i < s2.length(); ++i) {
            char c = s2[i];

            // 如果当前状态有c的转移，则转移到该状态
            if (sa.states[current].next.find(c) != sa.states[current].next.end()) {
                current = sa.states[current].next[c];
                current_len++;
            } else {
                // 否则，沿着后缀链接回溯
                while (current != -1 && sa.states[current].next.find(c) == sa.states[current].next.end()) {
                    current = sa.states[current].link;
                }

                if (current == -1) {
                    current = 0;
                    current_len = 0;
                } else {
                    current_len = sa.states[current].len + 1;
                    current = sa.states[current].next[c];
                }
            }

            // 更新最长公共子串
            if (current_len > max_len) {
                max_len = current_len;
                end_pos = i;
            }
        }

        // 返回最长公共子串
        if (max_len == 0) {
            return "";
        } else {
            return s2.substr(end_pos - max_len + 1, max_len);
        }
    }

    // 获取模式串的总出现次数
    int get_total_occurrences(const std::string& pattern) {
        int current = 0;

        // 遍历模式串，找到对应的状态
        for (char c : pattern) {
            if (states[current].next.find(c) == states[current].next.end()) {
                return 0;  // 模式串不存在
            }
            current = states[current].next[c];
        }

        // 返回该状态的出现次数
        return states[current].cnt;
    }

private:
    std::vector<State> states;
    int size;
    int last;
    std::vector<int> order;  // 按长度排序的状态索引
};

#endif // SUFFIX_AUTOMATON_H