#include <bits/stdc++.h>
using namespace std;

static constexpr int MAX_KEYS = 128;
static constexpr int MAX_CHILDREN = MAX_KEYS + 1;
static constexpr int MIN_LEAF = (MAX_KEYS + 1) / 2;
static constexpr int MIN_INTERNAL = (MAX_CHILDREN + 1) / 2 - 1;

struct Node {
    bool leaf;
    int cnt;
    string keys[MAX_KEYS + 1];
    Node* parent;
    Node* next;
    Node* child[MAX_CHILDREN + 2];
    vector<int> vals[MAX_KEYS + 1];
    Node(bool is_leaf = false) : leaf(is_leaf), cnt(0), parent(nullptr), next(nullptr) {
        memset(child, 0, sizeof(child));
    }
};

Node* root = nullptr;

int lower_key(Node* node, const string& key) {
    int l = 0, r = node->cnt;
    while (l < r) {
        int m = (l + r) >> 1;
        if (node->keys[m] < key) l = m + 1; else r = m;
    }
    return l;
}

Node* find_leaf(const string& key) {
    Node* cur = root;
    while (cur && !cur->leaf) {
        int i = lower_key(cur, key);
        cur = cur->child[i];
    }
    return cur;
}

void insert_in_parent(Node* left, const string& key, Node* right);

void split_leaf(Node* leaf) {
    int mid = leaf->cnt / 2;
    Node* right = new Node(true);
    right->parent = leaf->parent;
    right->cnt = leaf->cnt - mid;
    for (int i = 0; i < right->cnt; ++i) {
        right->keys[i] = std::move(leaf->keys[mid + i]);
        right->vals[i] = std::move(leaf->vals[mid + i]);
    }
    leaf->cnt = mid;
    right->next = leaf->next;
    leaf->next = right;
    insert_in_parent(leaf, right->keys[0], right);
}

void split_internal(Node* node) {
    int mid = node->cnt / 2;
    string up = node->keys[mid];
    Node* right = new Node(false);
    right->parent = node->parent;
    int right_keys = node->cnt - mid - 1;
    right->cnt = right_keys;
    for (int i = 0; i < right_keys; ++i) right->keys[i] = std::move(node->keys[mid + 1 + i]);
    for (int i = 0; i < right_keys + 1; ++i) {
        right->child[i] = node->child[mid + 1 + i];
        if (right->child[i]) right->child[i]->parent = right;
    }
    node->cnt = mid;
    insert_in_parent(node, up, right);
}

void insert_in_parent(Node* left, const string& key, Node* right) {
    if (!left->parent) {
        Node* nr = new Node(false);
        nr->keys[0] = key;
        nr->child[0] = left;
        nr->child[1] = right;
        nr->cnt = 1;
        left->parent = right->parent = nr;
        root = nr;
        return;
    }
    Node* parent = left->parent;
    int pos = 0;
    while (pos <= parent->cnt && parent->child[pos] != left) ++pos;
    for (int i = parent->cnt; i > pos; --i) parent->keys[i] = std::move(parent->keys[i - 1]);
    for (int i = parent->cnt + 1; i > pos + 1; --i) parent->child[i] = parent->child[i - 1];
    parent->keys[pos] = key;
    parent->child[pos + 1] = right;
    right->parent = parent;
    parent->cnt++;
    if (parent->cnt > MAX_KEYS) split_internal(parent);
}

void insert_entry(const string& key, int val) {
    if (!root) root = new Node(true);
    Node* leaf = find_leaf(key);
    int pos = lower_key(leaf, key);
    if (pos < leaf->cnt && leaf->keys[pos] == key) {
        auto& v = leaf->vals[pos];
        auto it = lower_bound(v.begin(), v.end(), val);
        v.insert(it, val);
        return;
    }
    for (int i = leaf->cnt; i > pos; --i) {
        leaf->keys[i] = std::move(leaf->keys[i - 1]);
        leaf->vals[i] = std::move(leaf->vals[i - 1]);
    }
    leaf->keys[pos] = key;
    leaf->vals[pos] = vector<int>{val};
    leaf->cnt++;
    if (leaf->cnt > MAX_KEYS) split_leaf(leaf);
}

void update_parent_key(Node* node, const string& old_first) {
    Node* p = node->parent;
    if (!p) return;
    int idx = 0;
    while (idx <= p->cnt && p->child[idx] != node) ++idx;
    if (idx > 0 && node->cnt > 0 && p->keys[idx - 1] == old_first) p->keys[idx - 1] = node->keys[0];
}

void merge_children(Node* left, Node* right, int parent_key_idx) {
    if (left->leaf) {
        for (int i = 0; i < right->cnt; ++i) {
            left->keys[left->cnt + i] = std::move(right->keys[i]);
            left->vals[left->cnt + i] = std::move(right->vals[i]);
        }
        left->cnt += right->cnt;
        left->next = right->next;
    } else {
        int start = left->cnt;
        left->keys[start] = left->parent->keys[parent_key_idx];
        for (int i = 0; i < right->cnt; ++i) left->keys[start + 1 + i] = std::move(right->keys[i]);
        for (int i = 0; i < right->cnt + 1; ++i) {
            left->child[start + 1 + i] = right->child[i];
            if (right->child[i]) right->child[i]->parent = left;
        }
        left->cnt += right->cnt + 1;
    }
}

void fix_delete(Node* node);

void borrow_or_merge(Node* node) {
    if (!node->parent) {
        if (!node->leaf && node->cnt == 0) {
            root = node->child[0];
            if (root) root->parent = nullptr;
        }
        return;
    }
    Node* p = node->parent;
    int idx = 0;
    while (idx <= p->cnt && p->child[idx] != node) ++idx;
    Node* left = idx > 0 ? p->child[idx - 1] : nullptr;
    Node* right = idx < p->cnt ? p->child[idx + 1] : nullptr;
    if (left && ((left->leaf && left->cnt > MIN_LEAF) || (!left->leaf && left->cnt > MIN_INTERNAL))) {
        if (node->leaf) {
            for (int i = node->cnt; i > 0; --i) {
                node->keys[i] = std::move(node->keys[i - 1]);
                node->vals[i] = std::move(node->vals[i - 1]);
            }
            node->keys[0] = std::move(left->keys[left->cnt - 1]);
            node->vals[0] = std::move(left->vals[left->cnt - 1]);
            left->cnt--;
            node->cnt++;
            p->keys[idx - 1] = node->keys[0];
        } else {
            for (int i = node->cnt; i > 0; --i) node->keys[i] = std::move(node->keys[i - 1]);
            for (int i = node->cnt + 1; i > 0; --i) node->child[i] = node->child[i - 1];
            node->child[0] = left->child[left->cnt];
            if (node->child[0]) node->child[0]->parent = node;
            node->keys[0] = p->keys[idx - 1];
            p->keys[idx - 1] = left->keys[left->cnt - 1];
            left->cnt--;
            node->cnt++;
        }
        return;
    }
    if (right && ((right->leaf && right->cnt > MIN_LEAF) || (!right->leaf && right->cnt > MIN_INTERNAL))) {
        if (node->leaf) {
            node->keys[node->cnt] = std::move(right->keys[0]);
            node->vals[node->cnt] = std::move(right->vals[0]);
            node->cnt++;
            for (int i = 0; i + 1 < right->cnt; ++i) {
                right->keys[i] = std::move(right->keys[i + 1]);
                right->vals[i] = std::move(right->vals[i + 1]);
            }
            right->cnt--;
            p->keys[idx] = right->keys[0];
        } else {
            node->keys[node->cnt] = p->keys[idx];
            node->child[node->cnt + 1] = right->child[0];
            if (node->child[node->cnt + 1]) node->child[node->cnt + 1]->parent = node;
            p->keys[idx] = right->keys[0];
            for (int i = 0; i + 1 < right->cnt; ++i) right->keys[i] = std::move(right->keys[i + 1]);
            for (int i = 0; i + 1 <= right->cnt; ++i) right->child[i] = right->child[i + 1];
            right->cnt--;
            node->cnt++;
        }
        return;
    }
    if (left) {
        merge_children(left, node, idx - 1);
        for (int i = idx - 1; i < p->cnt - 1; ++i) {
            p->keys[i] = std::move(p->keys[i + 1]);
            p->child[i + 1] = p->child[i + 2];
        }
        p->cnt--;
        fix_delete(p);
    } else if (right) {
        merge_children(node, right, idx);
        for (int i = idx; i < p->cnt - 1; ++i) {
            p->keys[i] = std::move(p->keys[i + 1]);
            p->child[i + 1] = p->child[i + 2];
        }
        p->cnt--;
        fix_delete(p);
    }
}

void fix_delete(Node* node) {
    if (node == root) {
        if (!node->leaf && node->cnt == 0) {
            root = node->child[0];
            if (root) root->parent = nullptr;
        }
        return;
    }
    int min_req = node->leaf ? MIN_LEAF : MIN_INTERNAL;
    if (node->cnt >= min_req) return;
    borrow_or_merge(node);
}

void delete_entry(const string& key, int val) {
    Node* leaf = find_leaf(key);
    if (!leaf) return;
    int pos = lower_key(leaf, key);
    if (pos == leaf->cnt || leaf->keys[pos] != key) return;
    auto& v = leaf->vals[pos];
    auto it = lower_bound(v.begin(), v.end(), val);
    if (it == v.end() || *it != val) return;
    v.erase(it);
    if (!v.empty()) return;
    string old_first = leaf->keys[pos];
    for (int i = pos; i + 1 < leaf->cnt; ++i) {
        leaf->keys[i] = std::move(leaf->keys[i + 1]);
        leaf->vals[i] = std::move(leaf->vals[i + 1]);
    }
    leaf->cnt--;
    if (leaf == root) {
        if (leaf->cnt == 0) {
            delete root;
            root = nullptr;
        }
        return;
    }
    if (pos == 0 && leaf->cnt > 0) update_parent_key(leaf, old_first);
    fix_delete(leaf);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    while (n--) {
        string op;
        cin >> op;
        if (op == "insert") {
            string k; int v; cin >> k >> v; insert_entry(k, v);
        } else if (op == "delete") {
            string k; int v; cin >> k >> v; delete_entry(k, v);
        } else if (op == "find") {
            string k; cin >> k;
            Node* leaf = find_leaf(k);
            if (!leaf) { cout << "null\n"; continue; }
            int pos = lower_key(leaf, k);
            if (pos == leaf->cnt || leaf->keys[pos] != k) { cout << "null\n"; continue; }
            auto& v = leaf->vals[pos];
            for (size_t i = 0; i < v.size(); ++i) {
                if (i) cout << ' ';
                cout << v[i];
            }
            cout << '\n';
        }
    }
    return 0;
}
