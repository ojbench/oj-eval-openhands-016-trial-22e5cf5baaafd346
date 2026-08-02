#include <bits/stdc++.h>
using namespace std;

static const char* DB_FILE = "db_state.bin";

static void write_u32(ofstream& out, uint32_t x) { out.write(reinterpret_cast<const char*>(&x), sizeof(x)); }
static void write_i32(ofstream& out, int32_t x) { out.write(reinterpret_cast<const char*>(&x), sizeof(x)); }
static uint32_t read_u32(ifstream& in) { uint32_t x; in.read(reinterpret_cast<char*>(&x), sizeof(x)); return x; }
static int32_t read_i32(ifstream& in) { int32_t x; in.read(reinterpret_cast<char*>(&x), sizeof(x)); return x; }

static void save_db(const map<string, multiset<int>>& db) {
    ofstream out(DB_FILE, ios::binary | ios::trunc);
    write_u32(out, (uint32_t)db.size());
    for (const auto& [k, ms] : db) {
        write_u32(out, (uint32_t)k.size());
        out.write(k.data(), k.size());
        write_u32(out, (uint32_t)ms.size());
        for (int v : ms) write_i32(out, v);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    map<string, multiset<int>> db;
    {
        ifstream in(DB_FILE, ios::binary);
        if (in) {
            uint32_t m = read_u32(in);
            for (uint32_t i = 0; i < m; ++i) {
                uint32_t len = read_u32(in);
                string key(len, '\0');
                in.read(key.data(), len);
                uint32_t c = read_u32(in);
                auto& ms = db[key];
                for (uint32_t j = 0; j < c; ++j) ms.insert(read_i32(in));
            }
        }
    }

    int n;
    if (!(cin >> n)) return 0;
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
        } else {
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

    save_db(db);
    return 0;
}
