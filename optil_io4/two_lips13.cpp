#include <iostream>
#include <vector>
#include <queue>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <chrono>
#include <limits>
#include <functional>
//92.89

using namespace std;

static int N_global;
static vector<vector<int>> g;
static vector<unordered_set<int>> adj;

struct InputGraph {
    int N, M, A, B;
    vector<pair<int,int>> edges;
    int base;
};

static InputGraph readAndDetectBase() {
    InputGraph ig;
    cin >> ig.N >> ig.M >> ig.A >> ig.B;
    ig.edges.reserve(ig.M);

    int minId = min(ig.A, ig.B);
    for (int i = 0; i < ig.M; i++) {
        int u, v;
        cin >> u >> v;
        ig.edges.push_back({u, v});
        minId = min(minId, min(u, v));
    }
    ig.base = (minId == 0 ? 0 : 1);
    return ig;
}

static bool isInducedPath(const vector<int> &path, int A, int B) {
    int k = (int)path.size();
    if (k == 0) return false;
    if (path[0] != A || path.back() != B) return false;
    if (A == B && k == 1) return true;
    if (k < 2) return false;

    int N = N_global;
    vector<int> pos(N + 1, -1);
    for (int i = 0; i < k; i++) {
        if (pos[path[i]] != -1) return false;
        pos[path[i]] = i;
    }

    for (int i = 0; i + 1 < k; i++)
        if (adj[path[i]].count(path[i + 1]) == 0)
            return false;

    for (int i = 0; i < k; i++)
        for (int nb : g[path[i]]) {
            int p = pos[nb];
            if (p != -1 && abs(p - i) > 1)
                return false;
        }

    return true;
}

static bool disjointExceptAB(const vector<int> &p1, const vector<int> &p2, int A, int B) {
    if (p1 == p2) return false;

    int N = N_global;
    vector<char> used(N + 1, 0);

    for (int v : p1)
        if (v != A && v != B)
            used[v] = 1;

    for (int v : p2)
        if (v != A && v != B && used[v])
            return false;

    return true;
}

static vector<int> bfsPath(int A, int B, const vector<char> &forbidden) {
    int N = N_global;
    vector<int> parent(N + 1, -1);
    queue<int> q;

    if (forbidden[A] || forbidden[B]) return {};

    parent[A] = A;
    q.push(A);

    while (!q.empty()) {
        int v = q.front();
        q.pop();

        if (v == B) {
            vector<int> path;
            int cur = B;
            while (cur != parent[cur]) {
                path.push_back(cur);
                cur = parent[cur];
            }
            path.push_back(A);
            reverse(path.begin(), path.end());
            return path;
        }

        for (int to : g[v]) {
            if (forbidden[to] || parent[to] != -1) continue;
            parent[to] = v;
            q.push(to);
        }
    }
    return {};
}

static vector<int> bfsDistFrom(int src, const vector<char> &forbidden) {
    int N = N_global;
    const int INF = numeric_limits<int>::max();
    vector<int> dist(N + 1, INF);

    if (forbidden[src]) return dist;

    queue<int> q;
    dist[src] = 0;
    q.push(src);

    while (!q.empty()) {
        int v = q.front();
        q.pop();

        for (int to : g[v]) {
            if (forbidden[to] || dist[to] != INF) continue;
            dist[to] = dist[v] + 1;
            q.push(to);
        }
    }
    return dist;
}

static vector<vector<int>> findMultipleShortPaths(int A, int B,
    const vector<char> &forbidden, int limit)
{
    int N = N_global;
    const int INF = numeric_limits<int>::max();
    vector<int> dist(N + 1, INF);
    vector<vector<int>> parents(N + 1);

    if (forbidden[A] || forbidden[B]) return {};

    queue<int> q;
    dist[A] = 0;
    q.push(A);

    while (!q.empty()) {
        int v = q.front();
        q.pop();

        for (int to : g[v]) {
            if (forbidden[to]) continue;

            if (dist[to] > dist[v] + 1) {
                dist[to] = dist[v] + 1;
                parents[to].clear();
                parents[to].push_back(v);
                q.push(to);
            } else if (dist[to] == dist[v] + 1 && (int)parents[to].size() < 5) {
                parents[to].push_back(v);
            }
        }
    }

    if (dist[B] == INF) return {};

    vector<vector<int>> results;
    vector<int> current = {B};
    vector<char> used(N + 1, 0);
    used[B] = 1;

    function<void()> enumerate = [&]() {
        if ((int)results.size() >= limit) return;

        int v = current.back();
        if (v == A) {
            results.push_back(vector<int>(current.rbegin(), current.rend()));
            return;
        }

        for (int p : parents[v]) {
            if (used[p] && p != A) continue;
            current.push_back(p);
            used[p] = 1;
            enumerate();
            current.pop_back();
            used[p] = 0;
            if ((int)results.size() >= limit) return;
        }
    };

    enumerate();
    return results;
}

static vector<int> shortcutToInduced(const vector<int> &path, int A, int B) {
    if (path.size() <= 2) return path;

    int N = N_global;
    vector<int> p = path;
    bool changed = true;

    while (changed) {
        changed = false;
        int k = (int)p.size();
        vector<int> pos(N + 1, -1);

        for (int i = 0; i < k; i++)
            pos[p[i]] = i;

        for (int i = 0; i < k && !changed; i++) {
            for (int nb : g[p[i]]) {
                int j = pos[nb];
                if (j <= i + 1 || j == -1) continue;

                vector<int> np(p.begin(), p.begin() + i + 1);
                np.insert(np.end(), p.begin() + j, p.end());
                p = np;
                changed = true;
                break;
            }
        }
    }

    return isInducedPath(p, A, B) ? p : path;
}

struct GrowState {
    vector<int> path;
    vector<char> inPath;
    vector<int> adjCount;
    int N;

    void init(int _N, int s) {
        N = _N;
        path.clear();
        inPath.assign(N + 1, 0);
        adjCount.assign(N + 1, 0);
        push(s);
    }

    void push(int v) {
        path.push_back(v);
        inPath[v] = 1;
        for (int nb : g[v])
            adjCount[nb]++;
    }

    void pop() {
        int v = path.back();
        path.pop_back();
        inPath[v] = 0;
        for (int nb : g[v])
            adjCount[nb]--;
    }

    bool canAdd(int to) {
        if (inPath[to]) return false;
        if (adj[path.back()].count(to) == 0) return false;
        return adjCount[to] == 1;
    }
};

static void sortCand(vector<int> &c, int s, int B,
    const vector<int> &ac, const vector<int> &dB,
    const vector<int> &dA, mt19937 &rng)
{
    const int INF = numeric_limits<int>::max();

    if (s == 0) {
        sort(c.begin(), c.end(), [&](int a, int b) {
            if (a == B) return false;
            if (b == B) return true;
            return dB[a] > dB[b];
        });
    } else if (s == 1) {
        sort(c.begin(), c.end(), [&](int a, int b) {
            if (a == B) return false;
            if (b == B) return true;
            return (int)g[a].size() < (int)g[b].size();
        });
    } else if (s == 2) {
        sort(c.begin(), c.end(), [&](int a, int b) {
            if (a == B) return false;
            if (b == B) return true;
            return (dA[a] < INF ? dA[a] : 0) > (dA[b] < INF ? dA[b] : 0);
        });
    } else if (s == 3) {
        sort(c.begin(), c.end(), [&](int a, int b) {
            if (a == B) return false;
            if (b == B) return true;
            int sa = (dA[a] < INF ? dA[a] : 0) + (dB[a] < INF ? dB[a] : 0);
            int sb = (dA[b] < INF ? dA[b] : 0) + (dB[b] < INF ? dB[b] : 0);
            return sa > sb;
        });
    } else if (s == 4) {
        sort(c.begin(), c.end(), [&](int a, int b) {
            if (a == B) return false;
            if (b == B) return true;
            return ac[a] < ac[b];
        });
    } else if (s == 5) {
        sort(c.begin(), c.end(), [&](int a, int b) {
            if (a == B) return false;
            if (b == B) return true;
            return (int)g[a].size() > (int)g[b].size();
        });
    } else {
        shuffle(c.begin(), c.end(), rng);
        for (int i = 0; i < (int)c.size(); i++)
            if (c[i] == B) {
                swap(c[i], c.back());
                break;
            }
    }
}

struct DFSBuilder {
    GrowState *gs;
    int B;
    const vector<char> *forb;
    const vector<int> *dB, *dA;
    mt19937 *rng;
    int strategy;
    vector<int> best;
    int bestLen;
    const function<double()> *elapsed;
    double tl;
    int calls, minKeep;
    bool timeout;
    int maxBr;
    int cutoff;

    void run() {
        calls = 0;
        bestLen = 0;
        timeout = false;
        dfs();
    }

    void dfs() {
        if (timeout) return;

        if (++calls % 200 == 0 && (*elapsed)() > tl) {
            timeout = true;
            return;
        }

        int cur = gs->path.back();

        if (cur == B) {
            if ((int)gs->path.size() > bestLen) {
                bestLen = (int)gs->path.size();
                best = gs->path;
            }
            return;
        }

        const int INF = numeric_limits<int>::max();
        vector<int> c;
        for (int to : g[cur]) {
            if ((*forb)[to] && to != B) continue;
            if ((*dB)[to] == INF) continue;
            if (!gs->canAdd(to)) continue;
            c.push_back(to);
        }

        if (c.empty()) return;

        sortCand(c, strategy, B, gs->adjCount, *dB, *dA, *rng);

        int br = maxBr;
        if ((int)gs->path.size() > cutoff)
            br = min(br, 2);

        int tc = min((int)c.size(), br);
        for (int i = 0; i < tc; i++) {
            if (timeout) return;
            if (c[i] == B && (int)gs->path.size() < minKeep) continue;

            gs->push(c[i]);
            dfs();
            gs->pop();

            if (timeout) return;
        }
    }
};

static vector<int> buildPath(int A, int B,
    const vector<char> &forb, const vector<int> &dB,
    const vector<int> &dA, mt19937 &rng, int strat,
    int maxBT, double tl, const function<double()> &elapsed)
{
    int N = N_global;
    if (forb[A] || forb[B] || dB[A] == numeric_limits<int>::max())
        return {};

    GrowState gs;
    gs.init(N, A);

    vector<int> best;
    int bt = 0, ct = 0;
    vector<int> c;

    while (true) {
        if (++ct % 300 == 0 && elapsed() > tl)
            break;

        int cur = gs.path.back();

        if (cur == B) {
            if (best.empty() || (int)gs.path.size() > (int)best.size())
                best = gs.path;

            if (bt >= maxBT) break;

            int bs = 2 + rng() % min(8, max(1, (int)gs.path.size() - 1));
            for (int b = 0; b < bs && gs.path.size() > 1; b++)
                gs.pop();

            bt++;
            continue;
        }

        c.clear();
        for (int to : g[cur]) {
            if (forb[to] && to != B) continue;
            if (dB[to] == numeric_limits<int>::max()) continue;
            if (!gs.canAdd(to)) continue;
            c.push_back(to);
        }

        if (c.empty()) {
            if (bt >= maxBT) break;

            int bs = 2 + rng() % min(15, max(1, (int)gs.path.size() - 1));
            for (int b = 0; b < bs && gs.path.size() > 1; b++)
                gs.pop();

            bt++;
            continue;
        }

        int ls = strat;
        if (bt > 0 && rng() % 3 == 0)
            ls = rng() % 7;

        sortCand(c, ls, B, gs.adjCount, dB, dA, rng);

        int pi = 0;
        if ((int)c.size() > 1 && c[0] != B)
            pi = rng() % min((int)c.size(), 5);

        gs.push(c[pi]);
    }

    return best;
}

static vector<int> buildDFS(int A, int B,
    const vector<char> &forb, const vector<int> &dB,
    const vector<int> &dA, mt19937 &rng, int strat,
    double tl, const function<double()> &elapsed,
    int minK, int maxBr, int cutoff)
{
    int N = N_global;
    if (forb[A] || forb[B] || dB[A] == numeric_limits<int>::max())
        return {};

    GrowState gs;
    gs.init(N, A);

    DFSBuilder d;
    d.gs = &gs;
    d.B = B;
    d.forb = &forb;
    d.dB = &dB;
    d.dA = &dA;
    d.rng = &rng;
    d.strategy = strat;
    d.elapsed = &elapsed;
    d.tl = tl;
    d.minKeep = minK;
    d.maxBr = maxBr;
    d.cutoff = cutoff;
    d.run();

    return d.best;
}

static void printPath(const vector<int> &p, int base) {
    cout << (int)p.size() << "\n";
    for (size_t i = 0; i < p.size(); i++) {
        if (i) cout << " ";
        cout << (base == 0 ? p[i] - 1 : p[i]);
    }
    cout << "\n";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    InputGraph ig = readAndDetectBase();
    int N = ig.N;
    N_global = N;

    auto toI = [&](int v) { return ig.base == 0 ? v + 1 : v; };
    int A = toI(ig.A), B = toI(ig.B);

    g.assign(N + 1, {});
    adj.assign(N + 1, {});

    for (auto &e : ig.edges) {
        int u = toI(e.first), v = toI(e.second);
        if (u < 1 || u > N || v < 1 || v > N || u == v) continue;
        if (adj[u].count(v)) continue;

        g[u].push_back(v);
        g[v].push_back(u);
        adj[u].insert(v);
        adj[v].insert(u);
    }

    double avgDeg = (2.0 * ig.M) / N;
    int dfsBr = avgDeg > 15 ? 2 : 3;
    int dfsCutoff = N * 2 / 3;

    unsigned seed = (unsigned)chrono::high_resolution_clock::now().time_since_epoch().count();
    mt19937 rng(seed);

    auto startTime = chrono::high_resolution_clock::now();
    auto elapsed = [&]() {
        return chrono::duration<double>(
            chrono::high_resolution_clock::now() - startTime
        ).count();
    };

    vector<char> forbNone(N + 1, 0);
    vector<int> fullDB = bfsDistFrom(B, forbNone);
    vector<int> fullDA = bfsDistFrom(A, forbNone);

    if (fullDB[A] == numeric_limits<int>::max())
        return 0;

    auto shortPaths = findMultipleShortPaths(A, B, forbNone, 50);
    if (shortPaths.empty())
        return 0;

    vector<vector<int>> candP1s;

    if (adj[A].count(B))
        candP1s.push_back({A, B});

    for (auto &sp : shortPaths) {
        auto ip = shortcutToInduced(sp, A, B);
        if (isInducedPath(ip, A, B))
            candP1s.push_back(ip);
    }

    sort(candP1s.begin(), candP1s.end(),
        [](auto &a, auto &b) { return a.size() < b.size(); });

    {
        vector<vector<int>> u;
        for (auto &p : candP1s) {
            bool d = false;
            for (auto &q : u)
                if (p == q) { d = true; break; }
            if (!d) u.push_back(p);
        }
        candP1s = u;
    }

    if (candP1s.empty())
        return 0;

    int gBS = -1;
    vector<int> gBP1, gBP2;

    auto tryUp = [&](const vector<int> &p1, const vector<int> &p2) -> bool {
        if (p1.empty() || p2.empty()) return false;

        int s = abs((int)p1.size() - (int)p2.size());
        if (s <= gBS) return false;
        if (!disjointExceptAB(p1, p2, A, B)) return false;
        if (!isInducedPath(p1, A, B) || !isInducedPath(p2, A, B)) return false;

        gBS = s;
        gBP1 = p1;
        gBP2 = p2;
        return true;
    };

    double tpc = min(1.5, 4.0 / max(1, (int)candP1s.size()));

    for (auto &pS : candP1s) {
        if (elapsed() > 5.0) break;

        double ps = elapsed();

        vector<char> fL(N + 1, 0);
        for (int v : pS)
            if (v != A && v != B)
                fL[v] = 1;

        auto dB2 = bfsDistFrom(B, fL);
        auto dA2 = bfsDistFrom(A, fL);

        if (dB2[A] == numeric_limits<int>::max())
            continue;

        vector<int> bL;

        auto sp = bfsPath(A, B, fL);
        if (!sp.empty()) {
            auto ip = shortcutToInduced(sp, A, B);
            if (isInducedPath(ip, A, B) && disjointExceptAB(pS, ip, A, B))
                bL = ip;
        }

        double tl = ps + tpc;

        while (elapsed() < tl) {
            vector<int> c;

            if (rng() % 2 == 0) {
                c = buildPath(A, B, fL, dB2, dA2, rng, rng() % 7, 60, tl, elapsed);
            } else {
                int mk = bL.empty() ? 1 : (int)bL.size();
                c = buildDFS(A, B, fL, dB2, dA2, rng, rng() % 7,
                    min(elapsed() + 0.5, tl), elapsed, mk, dfsBr, dfsCutoff);
            }

            if (!c.empty() && disjointExceptAB(pS, c, A, B))
                if (bL.empty() || (int)c.size() > (int)bL.size())
                    bL = c;
        }

        if (!bL.empty())
            tryUp(pS, bL);
    }

    while (elapsed() < 13.0) {
        vector<int> lP;

        if (rng() % 2 == 0) {
            lP = buildPath(A, B, forbNone, fullDB, fullDA,
                rng, rng() % 7, 70, 13.0, elapsed);
        } else {
            lP = buildDFS(A, B, forbNone, fullDB, fullDA,
                rng, rng() % 7, min(elapsed() + 1.0, 13.0),
                elapsed, N / 4, dfsBr, dfsCutoff);
        }

        if (lP.empty()) continue;

        vector<char> fS(N + 1, 0);
        for (int v : lP)
            if (v != A && v != B)
                fS[v] = 1;

        auto sP = bfsPath(A, B, fS);
        if (!sP.empty()) {
            auto ip = shortcutToInduced(sP, A, B);
            tryUp(lP, ip);
        }
    }

    if (gBS <= 0) return 0;

    vector<int> *pSF = (gBP1.size() <= gBP2.size()) ? &gBP1 : &gBP2;
    vector<int> *pLF = (gBP1.size() <= gBP2.size()) ? &gBP2 : &gBP1;

    vector<char> fF(N + 1, 0);
    for (int v : *pSF)
        if (v != A && v != B)
            fF[v] = 1;

    auto dBF = bfsDistFrom(B, fF);
    auto dAF = bfsDistFrom(A, fF);

    double lastRepair = elapsed();

    while (elapsed() < 29.0) {

        if (elapsed() - lastRepair > 3.0 && elapsed() < 27.0) {
            lastRepair = elapsed();
            int prevScore = gBS;

            vector<int> newLong;
            if (rng() % 2 == 0) {
                newLong = buildPath(A, B, forbNone, fullDB, fullDA,
                    rng, rng() % 7, 70, min(elapsed() + 1.0, 27.0), elapsed);
            } else {
                newLong = buildDFS(A, B, forbNone, fullDB, fullDA,
                    rng, rng() % 7, min(elapsed() + 1.0, 27.0),
                    elapsed, N / 4, dfsBr, dfsCutoff);
            }

            if (!newLong.empty()) {
                vector<char> fS2(N + 1, 0);
                for (int v : newLong)
                    if (v != A && v != B)
                        fS2[v] = 1;

                auto sP2 = bfsPath(A, B, fS2);
                if (!sP2.empty()) {
                    auto ip2 = shortcutToInduced(sP2, A, B);
                    tryUp(ip2, newLong);
                }
            }

            if (gBS > prevScore) {
                pSF = (gBP1.size() <= gBP2.size()) ? &gBP1 : &gBP2;
                pLF = (gBP1.size() <= gBP2.size()) ? &gBP2 : &gBP1;

                fF.assign(N + 1, 0);
                for (int v : *pSF)
                    if (v != A && v != B)
                        fF[v] = 1;

                dBF = bfsDistFrom(B, fF);
                dAF = bfsDistFrom(A, fF);
            }

            continue;
        }

        vector<int> c;
        double ds = 0.3 + (rng() % 15) * 0.1;
        int r = rng() % 3;

        if (r == 0) {
            c = buildPath(A, B, fF, dBF, dAF, rng, rng() % 7, 70,
                min(elapsed() + ds, 29.0), elapsed);
        } else if (r == 1) {
            int mk = (int)pLF->size() * 3 / 4;
            c = buildDFS(A, B, fF, dBF, dAF, rng, rng() % 7,
                min(elapsed() + ds, 29.0), elapsed, mk, dfsBr, dfsCutoff);
        } else {
            int mk = (int)pLF->size();
            c = buildDFS(A, B, fF, dBF, dAF, rng, rng() % 7,
                min(elapsed() + ds, 29.0), elapsed, mk, dfsBr, dfsCutoff);
        }

        if (c.empty() || !disjointExceptAB(*pSF, c, A, B))
            continue;

        if ((int)c.size() > (int)pLF->size() && isInducedPath(c, A, B)) {
            *pLF = c;
            gBS = abs((int)pLF->size() - (int)pSF->size());
        }
    }

    if (!isInducedPath(*pSF, A, B) ||
        !isInducedPath(*pLF, A, B) ||
        !disjointExceptAB(*pSF, *pLF, A, B))
        return 0;

    if (gBP1.size() <= gBP2.size()) {
        printPath(gBP1, ig.base);
        printPath(gBP2, ig.base);
    } else {
        printPath(gBP2, ig.base);
        printPath(gBP1, ig.base);
    }

    return 0;
}