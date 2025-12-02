#include "persistent_array.h"
#include <fstream>
#include <stdexcept>

int PersistentArray::build(int l, int r, const vector<int>& data) {
    int node_idx = nodes.size();
    nodes.emplace_back();
    if (l == r) {
        nodes[node_idx].value = data[l];
        return node_idx;
    }
    int mid = (l + r) / 2;
    int left_child = build(l, mid, data);
    int right_child = build(mid + 1, r, data);
    nodes[node_idx].left = left_child;
    nodes[node_idx].right = right_child;
    return node_idx;
}

int PersistentArray::update(int node_idx, int l, int r, int idx, int value) {
    if (l == r) {
        // 创建一个新的叶子节点
        Node new_node;
        new_node.left = -1;
        new_node.right = -1;
        new_node.value = value;
        nodes.push_back(new_node);
        return nodes.size() - 1;
    }
    int mid = (l + r) / 2;
    Node& node = nodes[node_idx];
    Node new_node;
    new_node.left = node.left;
    new_node.right = node.right;
    new_node.value = node.value;
    if (idx <= mid) {
        // 左子树需要更新
        new_node.left = update(node.left, l, mid, idx, value);
    } else {
        // 右子树需要更新
        new_node.right = update(node.right, mid + 1, r, idx, value);
    }
    nodes.push_back(new_node);
    return nodes.size() - 1;
}

int PersistentArray::query(int node_idx, int l, int r, int idx) const {
    if (l == r) {
        return nodes[node_idx].value;
    }
    int mid = (l + r) / 2;
    if (idx <= mid) {
        return query(nodes[node_idx].left, l, mid, idx);
    } else {
        return query(nodes[node_idx].right, mid + 1, r, idx);
    }
}

PersistentArray::PersistentArray(int size, const vector<int>& initial_data) : n(size) {
    if (initial_data.size() != size) {
        throw invalid_argument("Initial data size does not match array size");
    }
    int root = build(0, n - 1, initial_data);
    roots.push_back(root);
}

int PersistentArray::get(int version, int idx) const {
    if (version < 0 || version >= roots.size()) {
        throw invalid_argument("Invalid version");
    }
    if (idx < 0 || idx >= n) {
        throw invalid_argument("Invalid index");
    }
    return query(roots[version], 0, n - 1, idx);
}

int PersistentArray::set(int version, int idx, int value) {
    if (version < 0 || version >= roots.size()) {
        throw invalid_argument("Invalid version");
    }
    if (idx < 0 || idx >= n) {
        throw invalid_argument("Invalid index");
    }
    int new_root = update(roots[version], 0, n - 1, idx, value);
    roots.push_back(new_root);
    return roots.size() - 1;
}

int PersistentArray::size() const {
    return n;
}

int PersistentArray::get_version_count() const {
    return roots.size();
}

void PersistentArray::save(const string& filename) const {
    ofstream file(filename, ios::binary);
    if (!file.is_open()) {
        throw runtime_error("Failed to open file for writing");
    }
    // 保存数组大小
    file.write(reinterpret_cast<const char*>(&n), sizeof(n));
    // 保存节点数量
    int node_count = nodes.size();
    file.write(reinterpret_cast<const char*>(&node_count), sizeof(node_count));
    // 保存所有节点
    for (const auto& node : nodes) {
        file.write(reinterpret_cast<const char*>(&node.left), sizeof(node.left));
        file.write(reinterpret_cast<const char*>(&node.right), sizeof(node.right));
        file.write(reinterpret_cast<const char*>(&node.value), sizeof(node.value));
    }
    // 保存版本数量
    int version_count = roots.size();
    file.write(reinterpret_cast<const char*>(&version_count), sizeof(version_count));
    // 保存所有版本的根节点索引
    for (int root : roots) {
        file.write(reinterpret_cast<const char*>(&root), sizeof(root));
    }
    file.close();
}

void PersistentArray::load(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        throw runtime_error("Failed to open file for reading");
    }
    // 清空现有数据
    nodes.clear();
    roots.clear();
    // 读取数组大小
    file.read(reinterpret_cast<char*>(&n), sizeof(n));
    // 读取节点数量
    int node_count;
    file.read(reinterpret_cast<char*>(&node_count), sizeof(node_count));
    // 读取所有节点
    nodes.resize(node_count);
    for (int i = 0; i < node_count; ++i) {
        file.read(reinterpret_cast<char*>(&nodes[i].left), sizeof(nodes[i].left));
        file.read(reinterpret_cast<char*>(&nodes[i].right), sizeof(nodes[i].right));
        file.read(reinterpret_cast<char*>(&nodes[i].value), sizeof(nodes[i].value));
    }
    // 读取版本数量
    int version_count;
    file.read(reinterpret_cast<char*>(&version_count), sizeof(version_count));
    // 读取所有版本的根节点索引
    roots.resize(version_count);
    for (int i = 0; i < version_count; ++i) {
        file.read(reinterpret_cast<char*>(&roots[i]), sizeof(roots[i]));
    }
    file.close();
}