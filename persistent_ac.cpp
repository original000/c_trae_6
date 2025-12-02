#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <functional>

using namespace std;

struct Node {
    array<Node*, 26> children;
    Node* fail;
    vector<int> output;
    int dfs_in;
    int dfs_out;

    Node() : fail(nullptr), dfs_in(-1), dfs_out(-1) {
        children.fill(nullptr);
    }

    Node(const Node& other) : fail(other.fail), output(other.output), dfs_in(other.dfs_in), dfs_out(other.dfs_out) {
        children = other.children;
    }
};



Node* insert(Node* root, const string& pattern, int id) {
    Node* current = new Node(*root);
    Node* new_root = current;

    for (char ch : pattern) {
        int c = ch - 'a';
        Node* child = current->children[c];

        Node* new_child;
        if (child != nullptr) {
            new_child = new Node(*child);
        } else {
            new_child = new Node();
            // Compute fail pointer for new_child
            Node* f = current->fail;
            while (f != nullptr && f->children[c] == nullptr) {
                f = f->fail;
            }
            if (f == nullptr) {
                new_child->fail = new_root;
            } else {
                new_child->fail = f->children[c];
            }
            // Compute output list for new_child
            new_child->output.insert(new_child->output.end(), new_child->fail->output.begin(), new_child->fail->output.end());
        }

        current->children[c] = new_child;
        current = new_child;
    }

    current->output.push_back(id);

    return new_root;
}

void build_fail(Node* root) {
    vector<Node*> nodes;
    queue<Node*> q;
    q.push(root);

    while (!q.empty()) {
        Node* u = q.front();
        q.pop();
        nodes.push_back(u);

        for (int c = 0; c < 26; ++c) {
            Node* child = u->children[c];
            if (child != nullptr) {
                q.push(child);
            }
        }
    }

    queue<Node*> bfs_q;

    root->fail = nullptr;

    for (int c = 0; c < 26; ++c) {
        Node* child = root->children[c];
        if (child != nullptr) {
            child->fail = root;
            child->output.insert(child->output.end(), root->output.begin(), root->output.end());
            bfs_q.push(child);
        }
    }

    while (!bfs_q.empty()) {
        Node* u = bfs_q.front();
        bfs_q.pop();

        for (int c = 0; c < 26; ++c) {
            Node* v = u->children[c];
            if (v != nullptr) {
                bfs_q.push(v);

                Node* f = u->fail;
                while (f != nullptr && f->children[c] == nullptr) {
                    f = f->fail;
                }

                if (f == nullptr) {
                    v->fail = root;
                } else {
                    v->fail = f->children[c];
                }

                v->output.insert(v->output.end(), v->fail->output.begin(), v->fail->output.end());
            }
        }
    }

    unordered_map<Node*, vector<Node*>> fail_tree_children;
    for (Node* u : nodes) {
        if (u == root) continue;
        Node* f = u->fail;
        if (f != nullptr) {
            fail_tree_children[f].push_back(u);
        }
    }

    int dfs_time = 0;
    function<void(Node*)> dfs_helper = [&](Node* u) {
        u->dfs_in = dfs_time++;

        for (Node* v : fail_tree_children[u]) {
            dfs_helper(v);
        }

        u->dfs_out = dfs_time++;
    };

    dfs_helper(root);
}

vector<int> match(const string& text, Node* root) {
    vector<int> result;
    Node* current = root;

    for (char ch : text) {
        int c = ch - 'a';

        while (current != nullptr && current->children[c] == nullptr) {
            current = current->fail;
        }

        if (current == nullptr) {
            current = root;
            continue;
        }

        current = current->children[c];

        result.insert(result.end(), current->output.begin(), current->output.end());
    }

    sort(result.begin(), result.end());
    auto last = unique(result.begin(), result.end());
    result.erase(last, result.end());

    return result;
}

Node* find_node_with_pattern_id(Node* root, int id) {
    queue<Node*> q;
    q.push(root);

    while (!q.empty()) {
        Node* u = q.front();
        q.pop();

        for (int pattern_id : u->output) {
            if (pattern_id == id) {
                return u;
            }
        }

        for (int c = 0; c < 26; ++c) {
            Node* child = u->children[c];
            if (child != nullptr) {
                q.push(child);
            }
        }
    }

    return nullptr;
}

Node* create_new_node_without_pattern_id(Node* u, int id) {
    Node* u_prime = new Node(*u);

    auto it = remove(u_prime->output.begin(), u_prime->output.end(), id);
    u_prime->output.erase(it, u_prime->output.end());

    return u_prime;
}

vector<pair<Node*, int>> find_path_from_root(Node* root, Node* u) {
    vector<pair<Node*, int>> path;
    unordered_map<Node*, pair<Node*, int>> parent_map;
    queue<Node*> q;
    q.push(root);
    parent_map[root] = {nullptr, -1};

    while (!q.empty()) {
        Node* current = q.front();
        q.pop();

        if (current == u) {
            Node* node = u;
            while (node != nullptr) {
                auto parent_entry = parent_map[node];
                if (parent_entry.second != -1) {
                    path.emplace_back(parent_entry.first, parent_entry.second);
                }
                node = parent_entry.first;
            }
            reverse(path.begin(), path.end());
            return path;
        }

        for (int c = 0; c < 26; ++c) {
            Node* child = current->children[c];
            if (child != nullptr && parent_map.find(child) == parent_map.end()) {
                parent_map[child] = {current, c};
                q.push(child);
            }
        }
    }

    return path;
}

Node* update_trie_with_new_node(Node* root, Node* u, Node* u_prime) {
    vector<pair<Node*, int>> path = find_path_from_root(root, u);

    if (path.empty()) {
        return root;
    }

    Node* current_new_node = nullptr;
    Node* new_root = nullptr;

    for (int i = 0; i < path.size(); ++i) {
        Node* original_parent = path[i].first;
        int c = path[i].second;
        Node* original_child = original_parent->children[c];

        Node* new_parent;
        if (i == 0) {
            new_parent = new Node(*original_parent);
            new_root = new_parent;
        } else {
            new_parent = new Node(*current_new_node);
        }

        Node* new_child;
        if (i == path.size() - 1) {
            new_child = u_prime;
        } else {
            new_child = new Node(*original_child);
        }

        new_parent->children[c] = new_child;
        current_new_node = new_child;
    }

    return new_root;
}

Node* delete_pattern(Node* root, int id) {
    Node* u = find_node_with_pattern_id(root, id);
    if (u == nullptr) {
        return root;
    }

    Node* u_prime = create_new_node_without_pattern_id(u, id);
    Node* new_root = update_trie_with_new_node(root, u, u_prime);
    build_fail(new_root);

    return new_root;
}

int main() {
    Node* root0 = new Node();

    Node* root1 = insert(root0, "he", 1);
    root1 = insert(root1, "she", 2);
    build_fail(root1);

    Node* root2 = insert(root1, "his", 3);
    build_fail(root2);

    vector<int> result1 = match("ushers", root1);
    cout << "Version 1 match results: ";
    for (int id : result1) {
        cout << id << " ";
    }
    cout << endl;

    vector<int> result2 = match("ushers", root2);
    cout << "Version 2 match results: ";
    for (int id : result2) {
        cout << id << " ";
    }
    cout << endl;

    Node* root3 = delete_pattern(root2, 1);
    vector<int> result3 = match("ushers", root3);
    cout << "Version 3 match results (after deleting ID 1): ";
    for (int id : result3) {
        cout << id << " ";
    }
    cout << endl;

    return 0;
}
