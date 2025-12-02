#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <utility>

#include "persistent_array.h"
#include "persistent_weighted_union_find.h"

using namespace std;

int main() {
    try {
        int N, K;
        cin >> N >> K;
        if (N < 1 || N > 50000 || K < 0 || K > 100000) {
            throw invalid_argument("Invalid N or K");
        }
        PersistentWeightedUnionFind uf(N);
        vector<int> false_statements(K + 1, 0); // false_statements[0]表示初始状态，false_statements[i]表示处理完第i句话后的假话总数
        vector<int> versions(K + 1, 0); // versions[0]表示初始状态，versions[i]表示处理完第i句话后的版本号
        int current_false_statements = 0;
        int current_version = 0;
        for (int i = 1; i <= K; ++i) {
            int D, X, Y;
            cin >> D >> X >> Y;
            X--; // 将X转换为0-based索引
            Y--; // 将Y转换为0-based索引
            bool is_false = false;
            if (X >= N || Y >= N || (D == 2 && X == Y)) {
                is_false = true;
            } else {
                // 调试信息
                cout << "Debug: current_version = " << current_version << endl;
                cout << "Debug: version_count = " << uf.get_version_count() << endl;
                int rel;
                if (D == 1) {
                    rel = 0; // 同类
                } else {
                    rel = 1; // X吃Y
                }
                int new_version = uf.union_sets(X, Y, rel, current_version);
                if (new_version == current_version) {
                    // 关系不一致，这是一句假话
                    is_false = true;
                } else {
                    current_version = new_version;
                }
            }
            if (is_false) {
                current_false_statements++;
            }
            false_statements[i] = current_false_statements;
            versions[i] = current_version;
        }
        // 输出最终结果
        cout << current_false_statements << endl;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}