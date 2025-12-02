#ifndef PERSISTENT_WEIGHTED_UNION_FIND_H
#define PERSISTENT_WEIGHTED_UNION_FIND_H

#include "persistent_array.h"
#include <vector>
#include <string>
#include <utility>

using namespace std;

class PersistentWeightedUnionFind {
private:
    int size;
    int current_version;
    PersistentArray parent;
    PersistentArray rank;
    PersistentArray weight; // 存储每个节点与父节点的关系：0-同类，1-吃父节点，2-被父节点吃
    vector<int> version_stack; // 存储版本号的栈，用于undo操作

public:
    PersistentWeightedUnionFind() {}

    // 初始化并查集，每个元素的parent都是自己，weight都是0
    PersistentWeightedUnionFind(int size);

    // 查找指定版本中x的根节点，并返回根节点以及x到根节点的权值
    pair<int, int> find(int x, int version) const;

    // 合并指定版本中的集合a和b，并返回新的版本号
    // rel表示a和b的关系：0-同类，1-吃，2-被吃
    int union_sets(int a, int b, int rel, int version);

    // 撤销上一次的修改，返回上一个版本号
    int undo();

    // 时间旅行到指定版本，返回该版本号
    int time_travel(int version);

    // 保存并查集到文件
    void save(const string& filename) const;

    // 从文件加载并查集
    void load(const string& filename);

    // 获取当前版本号
    int get_current_version() const;
    int get_version_count() const;

    // 获取并查集的大小
    int get_size() const;
};

#endif // PERSISTENT_WEIGHTED_UNION_FIND_H