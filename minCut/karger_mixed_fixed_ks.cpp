// karger_mixed_fixed_ks.cpp
// Mixed graph experiment: Erdős & Clique
// Includes: Karger (single-run), Karger–Stein (recursive), and Stoer-Wagner (exact)

#include <bits/stdc++.h>
using namespace std;
using Edge = pair<int,int>;

// ------------------------------------------------------------
// DSU
// ------------------------------------------------------------
struct DSU {
    vector<int> p, r;
    int comp;
    DSU(int n=0){ init(n); }
    void init(int n){
        p.resize(n);
        r.assign(n,0);
        iota(p.begin(), p.end(), 0);
        comp = n;
    }
    int find(int x){ return p[x]==x ? x : p[x] = find(p[x]); }
    bool unite(int a,int b){
        a = find(a); b = find(b);
        if(a==b) return false;
        if(r[a] < r[b]) swap(a,b);
        p[b] = a;
        if(r[a]==r[b]) r[a]++;
        comp--;
        return true;
    }
};

// ------------------------------------------------------------
// Karger (single run)
// ------------------------------------------------------------
int karger_once(int n, const vector<Edge>& original_edges, mt19937 &rng){
    vector<Edge> edges = original_edges;
    DSU dsu(n);

    while(dsu.comp > 2 && !edges.empty()){
        uniform_int_distribution<size_t> dist(0, edges.size()-1);
        size_t idx = dist(rng);

        int u = edges[idx].first;
        int v = edges[idx].second;
        int ru = dsu.find(u), rv = dsu.find(v);

        if(ru == rv){
            edges[idx] = edges.back();
            edges.pop_back();
            continue;
        }

        dsu.unite(ru, rv);

        vector<Edge> new_edges;
        new_edges.reserve(edges.size());
        for(const auto &e : edges){
            int a = dsu.find(e.first);
            int b = dsu.find(e.second);
            if(a != b) new_edges.emplace_back(a,b);
        }
        edges.swap(new_edges);
    }

    int cut = 0;
    for(const auto &e : original_edges){
        if(dsu.find(e.first) != dsu.find(e.second)) cut++;
    }
    return cut;
}

// ------------------------------------------------------------
// Stoer–Wagner (warning-free)
// ------------------------------------------------------------
int stoer_wagner(int N, const vector<Edge>& edges){
    if(N <= 1) return 0;

    vector<vector<int>> w(N, vector<int>(N, 0));
    for(const auto &e : edges){
        w[e.first][e.second]++;
        w[e.second][e.first]++;
    }

    vector<int> v(N);
    iota(v.begin(), v.end(), 0);

    int best = INT_MAX;
    int n = N;

    while(n > 1){
        vector<int> added(n, 0), weights(n, 0);
        int prev = -1, last = -1;

        for(int i=0;i<n;i++){
            int sel = -1;
            for(int j=0;j<n;j++)
                if(!added[j] && (sel == -1 || weights[j] > weights[sel]))
                    sel = j;

            added[sel] = 1;
            prev = last;
            last = sel;

            if(i == n-1) break;

            for(int j=0;j<n;j++)
                if(!added[j])
                    weights[j] += w[v[sel]][v[j]];
        }

        int cut = 0;
        for(int j=0;j<n;j++)
            if(j != last) cut += w[v[last]][v[j]];
        best = min(best, cut);

        if(prev < 0) break;

        int s = v[prev], t = v[last];
        for(int j=0;j<n;j++){
            int node = v[j];
            w[s][node] += w[t][node];
            w[node][s] = w[s][node];
        }

        v[last] = v[n-1];
        n--;
    }

    return best;
}

// ------------------------------------------------------------
// Karger–Stein: contract-until
// ------------------------------------------------------------
pair<int, vector<Edge>>
contract_until(int n, const vector<Edge>& original, int target, mt19937 &rng){
    if(target >= n) return {n, original};

    vector<Edge> edges = original;
    DSU dsu(n);

    while(dsu.comp > target){
        uniform_int_distribution<size_t> dist(0, edges.size()-1);
        size_t idx = dist(rng);

        int u = edges[idx].first;
        int v = edges[idx].second;

        int ru = dsu.find(u), rv = dsu.find(v);
        if(ru == rv){
            edges[idx] = edges.back();
            edges.pop_back();
            continue;
        }

        dsu.unite(ru, rv);

        vector<Edge> filtered;
        filtered.reserve(edges.size());
        for(const auto &e : edges){
            int a = dsu.find(e.first);
            int b = dsu.find(e.second);
            if(a != b) filtered.emplace_back(a,b);
        }
        edges.swap(filtered);
    }

    unordered_map<int,int> mp;
    int newn = 0;
    for(int i=0;i<n;i++){
        int r = dsu.find(i);
        if(!mp.count(r)) mp[r] = newn++;
    }

    vector<Edge> new_edges;
    for(const auto &e : edges){
        int a = mp[ dsu.find(e.first) ];
        int b = mp[ dsu.find(e.second) ];
        if(a != b) new_edges.emplace_back(a,b);
    }

    return {newn, new_edges};
}

// ------------------------------------------------------------
// Karger–Stein recursive
// ------------------------------------------------------------
int karger_stein_rec(int n, const vector<Edge>& edges, mt19937 &rng){
    if(n <= 6)
        return stoer_wagner(n, edges);

    int t = (int)ceil(n / sqrt(2.0));

    uint32_t s1 = rng();
    uint32_t s2 = rng();
    mt19937 rng1(s1), rng2(s2);

    auto [n1, e1] = contract_until(n, edges, t, rng1);
    auto [n2, e2] = contract_until(n, edges, t, rng2);

    int c1 = karger_stein_rec(n1, e1, rng1);
    int c2 = karger_stein_rec(n2, e2, rng2);

    return min(c1, c2);
}

int karger_stein(int n, const vector<Edge>& edges, mt19937 &rng){
    return karger_stein_rec(n, edges, rng);
}

// ------------------------------------------------------------
// Graph Generators
// ------------------------------------------------------------
vector<Edge> gen_erdos(int n, double p, mt19937 &rng){
    vector<Edge> edges;
    uniform_real_distribution<double> ud(0, 1);

    for(int i=0;i<n;i++)
        for(int j=i+1;j<n;j++)
            if(ud(rng) < p) edges.emplace_back(i,j);

    return edges;
}

vector<Edge> gen_clique(int n, int k, mt19937 &rng){
    vector<Edge> edges;
    int half = n/2;

    for(int i=0;i<half;i++)
        for(int j=i+1;j<half;j++)
            edges.emplace_back(i,j);

    for(int i=half;i<n;i++)
        for(int j=i+1;j<n;j++)
            edges.emplace_back(i,j);

    uniform_int_distribution<int> A(0, half-1);
    uniform_int_distribution<int> B(half, n-1);

    for(int i=0;i<k;i++)
        edges.emplace_back(A(rng), B(rng));

    return edges;
}

// ------------------------------------------------------------
// MAIN — Mixed Experiments
// ------------------------------------------------------------
int main(int argc, char** argv){
    if(argc != 7){
        cerr << "Usage: ./karger_mixed_fixed_ks n p k trials seed out.csv\n";
        return 1;
    }

    int n = stoi(argv[1]);
    double p = stod(argv[2]);
    int k = stoi(argv[3]);
    int trials = stoi(argv[4]);
    unsigned long long seed = stoull(argv[5]);
    string out = argv[6];

    ofstream fout(out);
    fout << "trial_id,graph_type,true_mincut,"
            "karger_cut,karger_ms,karger_correct,"
            "ks_cut,ks_ms,ks_correct\n";

    mt19937 exp_rng(seed);

    int karger_corr_erdos=0, karger_corr_clique=0;
    int ks_corr_erdos=0, ks_corr_clique=0;
    int ned=0, ncl=0;

    double karger_time_erdos=0, karger_time_clique=0;
    double ks_time_erdos=0, ks_time_clique=0;

    for(int t=1;t<=trials;t++){
        uint32_t run_seed = exp_rng();
        mt19937 local(run_seed);

        bool is_erdos = (local() & 1);
        vector<Edge> edges = is_erdos ?
            gen_erdos(n, p, local) : gen_clique(n, k, local);

        int true_cut = stoer_wagner(n, edges);

        // KARGER
        auto t0 = chrono::high_resolution_clock::now();
        int kc = karger_once(n, edges, local);
        auto t1 = chrono::high_resolution_clock::now();
        double k_ms = chrono::duration<double,milli>(t1-t0).count();
        bool kc_ok = (kc == true_cut);

        // KARGER-STEIN
        auto s0 = chrono::high_resolution_clock::now();
        int ks = karger_stein(n, edges, local);
        auto s1 = chrono::high_resolution_clock::now();
        double ks_ms = chrono::duration<double,milli>(s1-s0).count();
        bool ks_ok = (ks == true_cut);

        // logging
        fout << t << "," << (is_erdos?"erdos":"clique") << ","
             << true_cut << ","
             << kc << "," << k_ms << "," << kc_ok << ","
             << ks << "," << ks_ms << "," << ks_ok << "\n";

        // stats
        if(is_erdos){
            ned++;
            karger_time_erdos += k_ms;
            ks_time_erdos += ks_ms;
            if(kc_ok) karger_corr_erdos++;
            if(ks_ok) ks_corr_erdos++;
        } else {
            ncl++;
            karger_time_clique += k_ms;
            ks_time_clique += ks_ms;
            if(kc_ok) karger_corr_clique++;
            if(ks_ok) ks_corr_clique++;
        }
    }

    fout.close();
    cout << "Wrote CSV: " << out << "\n\n";

    cout << "Summary over " << trials << " trials:\n";

    if(ned){
        cout << "Erdos:\n";
        cout << "  Karger:       acc=" << (double)karger_corr_erdos/ned
             << " avg_ms=" << karger_time_erdos/ned << "\n";
        cout << "  Karger-Stein: acc=" << (double)ks_corr_erdos/ned
             << " avg_ms=" << ks_time_erdos/ned << "\n";
    }

    if(ncl){
        cout << "Clique:\n";
        cout << "  Karger:       acc=" << (double)karger_corr_clique/ncl
             << " avg_ms=" << karger_time_clique/ncl << "\n";
        cout << "  Karger-Stein: acc=" << (double)ks_corr_clique/ncl
             << " avg_ms=" << ks_time_clique/ncl << "\n";
    }

    return 0;
}
