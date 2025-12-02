#ifndef PERSISTENT_ARRAY_H
#define PERSISTENT_ARRAY_H

#include <vector>
#include <string>

using namespace std;

struct Node {
    int left, right;
    int value;
    Node() : left(-1), right(-1), value(0) {}
    Node(int l, int r, int v) : left(l), right(r), value(v) {}
};

class PersistentArray {
private:
    vector<Node> nodes;
    int n; // 数组的大小

    int build(int l, int r, const vector<int>& data);
    int update(int prev_node_idx, int l, int r, int idx, int value);
    int query(int node_idx, int l, int r, int idx) const;

public:
    vector<int> roots; // 存储每个版本的根节点索引

    PersistentArray() : n(0) {}

    // 初始化数组
    PersistentArray(int size, const vector<int>& initial_data);

    // 获取指定版本中指定索引的值
    int get(int version, int idx) const;

    // 设置指定版本中指定索引的值，并返回新的版本号
    int set(int version, int idx, int value);

    // 获取数组的大小
    int size() const;
    // 获取版本数量
    int get_version_count() const;

    // 保存数组到文件
    void save(const string& filename) const;

    // 从文件加载数组
    void load(const string& filename);
};

#endif // PERSISTENT_ARRAY_H