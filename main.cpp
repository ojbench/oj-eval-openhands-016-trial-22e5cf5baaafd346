#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    map<string, multiset<int>> db;

    while (n--) {
        string op, key;
        cin >> op >> key;
        if (op == "insert") {
            int v; cin >> v;
            db[key].insert(v);
        } else if (op == "delete") {
            int v; cin >> v;
            auto it = db.find(key);
            if (it != db.end()) {
                auto vit = it->second.find(v);
                if (vit != it->second.end()) {
                    it->second.erase(vit);
                    if (it->second.empty()) db.erase(it);
                }
            }
        } else if (op == "find") {
            auto it = db.find(key);
            if (it == db.end()) {
                cout << "null\n";
            } else {
                bool first = true;
                for (int v : it->second) {
                    if (!first) cout << ' ';
                    first = false;
                    cout << v;
                }
                cout << '\n';
            }
        }
    }
    return 0;
}
