#ifndef SUFFIX_AUTOMATON_H
#define SUFFIX_AUTOMATON_H

#include <cstddef>
#include <vector>
#include <string>
#include <algorithm>

class SuffixAutomaton {
public:
    struct State {
        int len; // 状态表示的最长子串的长度
        int link; // 后缀链接
        std::vector<int> next; // 转移函数

        State(int len = 0, int link = -1) : len(len), link(link), next(256, -1) {}
    };

    SuffixAutomaton() : size(1), last(0) {
        states.emplace_back(0, -1);
    }

    explicit SuffixAutomaton(const std::string& s) : SuffixAutomaton() {
        build(s);
    }

    void build(const std::string& s) {
        for (char c : s) {
            extend(c);
        }
    }

    void extend(char c) {
        int curr = size++;
        states.emplace_back(states[last].len + 1);
        int p = last;

        while (p != -1 && states[p].next[c] == -1) {
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
                states.emplace_back(states[p].len + 1, states[q].link);
                states[clone].next = states[q].next;

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

    bool contains(const std::string& s) const {
        int curr = 0;
        for (char c : s) {
            if (states[curr].next[c] == -1) {
                return false;
            }
            curr = states[curr].next[c];
        }
        return true;
    }

    std::vector<int> find_all_occurrences(const std::string& s) const {
        std::vector<int> occurrences;
        int curr = 0;

        for (char c : s) {
            if (states[curr].next[c] == -1) {
                return occurrences;
            }
            curr = states[curr].next[c];
        }

        std::vector<int> order(size);
        std::vector<int> cnt(size);

        for (int i = 0; i < size; ++i) {
            cnt[states[i].len]++;
        }

        for (int i = 1; i <= states[last].len; ++i) {
            cnt[i] += cnt[i - 1];
        }

        for (int i = size - 1; i >= 0; --i) {
            order[--cnt[states[i].len]] = i;
        }

        std::vector<int> endpos(size);
        int curr_state = curr;

        while (curr_state != -1) {
            endpos[curr_state] = 1;
            curr_state = states[curr_state].link;
        }

        for (int i : order) {
            if (states[i].link != -1) {
                endpos[states[i].link] += endpos[i];
            }
        }

        curr_state = curr;
        while (curr_state != -1) {
            if (endpos[curr_state] > 0 && states[curr_state].len >= s.size()) {
                occurrences.push_back(states[curr_state].len - s.size());
            }
            curr_state = states[curr_state].link;
        }

        std::sort(occurrences.begin(), occurrences.end());
        return occurrences;
    }

    const std::vector<State>& get_states() const {
        return states;
    }

    int get_size() const {
        return size;
    }

private:
    std::vector<State> states;
    int size;
    int last;
};

#endif // SUFFIX_AUTOMATON_H
