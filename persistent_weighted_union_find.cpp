#include "persistent_weighted_union_find.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

PersistentWeightedUnionFind::PersistentWeightedUnionFind(int size) {
    if (size <= 0) {
        throw invalid_argument("Size must be positive");
    }
    this->size = size;
    current_version = 0;
    vector<int> initial_parent(size);
    vector<int> initial_weight(size, 0);
    vector<int> initial_rank(size, 0);
    for (int i = 0; i < size; ++i) {
        initial_parent[i] = i;
    }
    parent = PersistentArray(size, initial_parent);
    weight = PersistentArray(size, initial_weight);
    rank = PersistentArray(size, initial_rank);
    version_stack.clear();
    version_stack.push_back(0); // 初始版本号为0
}

pair<int, int> PersistentWeightedUnionFind::find(int x, int version) const {
    // 调试信息
    cout << "Debug: find() version = " << version << endl;
    cout << "Debug: find() parent.get_version_count() = " << parent.get_version_count() << endl;
    if (version < 0 || version >= parent.get_version_count()) {
        throw invalid_argument("Invalid version");
    }
    if (x < 0 || x >= parent.size()) {
        throw invalid_argument("Invalid element");
    }
    int total_weight = 0;
    while (true) {
        int parent_x = parent.get(version, x);
        if (parent_x == x) {
            break;
        }
        total_weight = (total_weight + weight.get(version, x)) % 3;
        x = parent_x;
    }
    return {x, total_weight};
}

int PersistentWeightedUnionFind::union_sets(int a, int b, int rel, int version) {
    // 调试信息
    cout << "Debug: union_sets() version = " << version << endl;
    cout << "Debug: union_sets() parent.get_version_count() = " << parent.get_version_count() << endl;
    if (version < 0 || version >= parent.get_version_count()) {
        throw invalid_argument("Invalid version");
    }
    if (a < 0 || a >= parent.size() || b < 0 || b >= parent.size()) {
        throw invalid_argument("Invalid element");
    }
    pair<int, int> result_a = find(a, version);
    int root_a = result_a.first;
    int weight_a = result_a.second;

    pair<int, int> result_b = find(b, version);
    int root_b = result_b.first;
    int weight_b = result_b.second;

    // 如果a和b已经在同一个集合中，检查关系是否一致
    if (root_a == root_b) {
        int expected_rel = (weight_a - weight_b + 3) % 3;
        if (expected_rel != rel) {
            // 关系不一致，不进行合并
            version_stack.push_back(version);
            return version;
        } else {
            // 关系一致，返回当前版本
            version_stack.push_back(version);
            return version;
        }
    } else {
        // a和b不在同一个集合中，合并它们
        int new_version;
        int rank_a = rank.get(version, root_a);
        int rank_b = rank.get(version, root_b);

        // 将秩小的树合并到秩大的树上
        if (rank_a > rank_b) {
            // 更新b的父节点为a
            new_version = parent.set(version, root_b, root_a);
            // 更新b的权重
            int new_weight = (weight.get(version, a) - weight.get(version, b) + rel + 3) % 3;// 更新b的权重
            new_version = weight.set(new_version, root_b, new_weight);
            // 复制a的秩
            new_version = rank.set(new_version, root_a, rank_a);
        } else {
            // 更新a的父节点为b
            new_version = parent.set(version, root_a, root_b);
            // 更新a的权重
            int new_weight = (weight.get(version, b) - weight.get(version, a) - rel + 3) % 3;// 更新a的权重
            new_version = weight.set(new_version, root_a, new_weight);
            // 如果秩相等，更新b的秩
            if (rank_a == rank_b) {
                new_version = rank.set(new_version, root_b, rank_b + 1);
            } else {
                // 复制b的秩
                new_version = rank.set(new_version, root_b, rank_b);
            }
        }

        // 更新current_version变量
        current_version = new_version;
        version_stack.push_back(new_version);
        // 返回新的版本号
        return new_version;
    }
}

int PersistentWeightedUnionFind::undo() {
    if (version_stack.size() <= 1) {
        throw runtime_error("No more operations to undo");
    }
    version_stack.pop_back();
    return version_stack.back();
}

int PersistentWeightedUnionFind::time_travel(int version) {
    if (version < 0 || version >= parent.roots.size()) {
        throw invalid_argument("Invalid version");
    }
    version_stack.push_back(version);
    return version;
}

void PersistentWeightedUnionFind::save(const string& filename) const {
    // 保存parent数组
    parent.save(filename + ".parent");
    // 保存weight数组
    weight.save(filename + ".weight");
    // 保存version_stack
    ofstream file(filename + ".version", ios::binary);
    if (!file.is_open()) {
        throw runtime_error("Failed to open file for writing");
    }
    int version_count = version_stack.size();
    file.write(reinterpret_cast<const char*>(&version_count), sizeof(version_count));
    for (int v : version_stack) {
        file.write(reinterpret_cast<const char*>(&v), sizeof(v));
    }
    file.close();
}

void PersistentWeightedUnionFind::load(const string& filename) {
    // 加载parent数组
    parent.load(filename + ".parent");
    // 加载weight数组
    weight.load(filename + ".weight");
    // 加载version_stack
    ifstream file(filename + ".version", ios::binary);
    if (!file.is_open()) {
        throw runtime_error("Failed to open file for reading");
    }
    int version_count;
    file.read(reinterpret_cast<char*>(&version_count), sizeof(version_count));
    version_stack.resize(version_count);
    for (int i = 0; i < version_count; ++i) {
        file.read(reinterpret_cast<char*>(&version_stack[i]), sizeof(version_stack[i]));
    }
    file.close();
}

int PersistentWeightedUnionFind::get_current_version() const {
    if (version_stack.empty()) {
        throw runtime_error("UnionFind not initialized");
    }
    return version_stack.back();
}

int PersistentWeightedUnionFind::get_version_count() const {
    return parent.get_version_count();
}

int PersistentWeightedUnionFind::get_size() const {
    return parent.size();
}