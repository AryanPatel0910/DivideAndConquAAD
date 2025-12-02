// karger_batch_ks.cpp
// Batch experiments for: Karger (single-run), Karger–Stein (recursive), Stoer–Wagner exact.
// Runs for multiple values of n and trials.
// Outputs ONE summary row per (n, trials).

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
// Karger single-run
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

        vector<Edge> filtered;
        filtered.reserve(edges.size());
        for(const auto &e : edges){
            int a = dsu.find(e.first);
            int b = dsu.find(e.second);
            if(a != b) filtered.emplace_back(a,b);
        }
        edges.swap(filtered);
    }

    int cut = 0;
    for(const auto &e : original_edges){
        if(dsu.find(e.first) != dsu.find(e.second)) cut++;
    }
    return cut;
}

// ------------------------------------------------------------
// Stoer–Wagner (warning-free version)
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
        vector<int> added(n, 0);
        vector<int> weights(n, 0);

        int prev = -1;
        int last = -1;

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

        int s = v[prev];
        int t = v[last];

        for(int j=0;j<n;j++){
            int node = v[j];
            w[s][node] += w[t][node];
            w[node][s] = w[s][node];
        }

        v[last] = v[n - 1];
        n--;
    }

    return best;
}

// ------------------------------------------------------------
// Karger–Stein: contract-until stage
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
    int id = 0;
    for(int i=0;i<n;i++){
        int r = dsu.find(i);
        if(!mp.count(r)) mp[r] = id++;
    }

    vector<Edge> new_edges;
    new_edges.reserve(edges.size());
    for(const auto &e : edges){
        int a = mp[ dsu.find(e.first) ];
        int b = mp[ dsu.find(e.second) ];
        if(a != b) new_edges.emplace_back(a,b);
    }

    return {id, new_edges};
}

// ------------------------------------------------------------
// Karger–Stein recursive
// ------------------------------------------------------------
int karger_stein_rec(int n, const vector<Edge>& edges, mt19937 &rng){
    if(n <= 20)
        return stoer_wagner(n, edges);

    int t = (int)ceil(n / sqrt(2.0));

    uint32_t s1 = rng();
    uint32_t s2 = rng();
    mt19937 r1(s1), r2(s2);

    auto [n1, e1] = contract_until(n, edges, t, r1);
    auto [n2, e2] = contract_until(n, edges, t, r2);

    int c1 = karger_stein_rec(n1, e1, r1);
    int c2 = karger_stein_rec(n2, e2, r2);

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
    int half = n / 2;

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
// Summary structure
// ------------------------------------------------------------
struct Summary {
    double erdos_k_acc,    clique_k_acc;
    double erdos_ks_acc,   clique_ks_acc;
    double combined_k_acc, combined_ks_acc;

    double erdos_k_ms,     clique_k_ms;
    double erdos_ks_ms,    clique_ks_ms;
};

// ------------------------------------------------------------
// Run ONE experiment configuration (n, trials)
// ------------------------------------------------------------
Summary run_experiment(int n, int trials, int k, double p, unsigned seed){
    mt19937 rng(seed);

    int ke = 0, kc = 0;
    int kse = 0, ksc = 0;
    int ned = 0, ncl = 0;

    double ke_time = 0, kc_time = 0;
    double kse_time = 0, ksc_time = 0;

    for(int t=0;t<trials;t++){
        uint32_t s = rng();
        mt19937 local(s);

        bool isErdos = (local() & 1);
        vector<Edge> edges =
            isErdos ? gen_erdos(n, p, local) : gen_clique(n, k, local);

        int true_cut = stoer_wagner(n, edges);

        // ----- KARGER -----
        auto t0 = chrono::high_resolution_clock::now();
        int c1 = karger_once(n, edges, local);
        auto t1 = chrono::high_resolution_clock::now();
        double ms1 = chrono::duration<double,milli>(t1-t0).count();

        // ----- KARGER–STEIN -----
        auto t2 = chrono::high_resolution_clock::now();
        int c2 = karger_stein(n, edges, local);
        auto t3 = chrono::high_resolution_clock::now();
        double ms2 = chrono::duration<double,milli>(t3-t2).count();

        if(isErdos){
            ned++;
            ke_time += ms1;
            kse_time += ms2;
            if(c1 == true_cut) ke++;
            if(c2 == true_cut) kse++;
        } else {
            ncl++;
            kc_time += ms1;
            ksc_time += ms2;
            if(c1 == true_cut) kc++;
            if(c2 == true_cut) ksc++;
        }
    }

    Summary S;

    S.erdos_k_acc  = ned ? (double)ke / ned : 0;
    S.erdos_ks_acc = ned ? (double)kse / ned : 0;
    S.clique_k_acc = ncl ? (double)kc / ncl : 0;
    S.clique_ks_acc= ncl ? (double)ksc / ncl : 0;

    S.combined_k_acc  = (double)(ke + kc)  / trials;
    S.combined_ks_acc = (double)(kse + ksc)/ trials;

    S.erdos_k_ms  = ned ? ke_time / ned : 0;
    S.erdos_ks_ms = ned ? kse_time / ned : 0;
    S.clique_k_ms = ncl ? kc_time / ncl : 0;
    S.clique_ks_ms= ncl ? ksc_time / ncl : 0;

    return S;
}

// ------------------------------------------------------------
// MAIN — Batch Runner
// ------------------------------------------------------------
int main(int argc, char** argv){
    if(argc != 2){
        cerr << "Usage: ./karger_batch_ks output.csv\n";
        return 1;
    }

    string out = argv[1];
    ofstream fout(out);

    fout
    << "n,trials,"
    << "erdos_k_acc,clique_k_acc,combined_k_acc,"
    << "erdos_ks_acc,clique_ks_acc,combined_ks_acc,"
    << "erdos_k_ms,clique_k_ms,"
    << "erdos_ks_ms,clique_ks_ms,"
    << "seed\n";

    vector<int> Ns = {10, 20, 50, 75, 100, 150};
    vector<int> Trials = {100, 1000, 5000, 10000, 20000};

    int k = 5;
    double p = 0.1;

    for(int n : Ns){
        for(int T : Trials){
            unsigned seed = 100000 + n * 1000 + T;

            cout << "Running n=" << n
                 << " trials=" << T
                 << " seed=" << seed << "...\n";

            Summary S = run_experiment(n, T, k, p, seed);

            fout
            << n << "," << T << ","
            << S.erdos_k_acc << ","
            << S.clique_k_acc << ","
            << S.combined_k_acc << ","
            << S.erdos_ks_acc << ","
            << S.clique_ks_acc << ","
            << S.combined_ks_acc << ","
            << S.erdos_k_ms << ","
            << S.clique_k_ms << ","
            << S.erdos_ks_ms << ","
            << S.clique_ks_ms << ","
            << seed << "\n";
        }
    }

    fout.close();
    cout << "Saved: " << out << "\n";
    return 0;
}
