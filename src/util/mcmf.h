#ifndef ALKAID_SD_SRC_UTIL_MCMF_H_
#define ALKAID_SD_SRC_UTIL_MCMF_H_

#include <queue>

namespace alkaid_sd {

struct MCMF {
  const int inf = static_cast<int>(1e9);

  struct Node {
    int from, to, nxt, cap, flow, cost;
  };

  std::vector<int> dis, g, pre, vis;
  std::vector<Node> e;
  int n;

  MCMF(int n) : n(n), dis(n), g(n, -1), pre(n), vis(n) {}

  int Link(int u, int v, int f, int c) {
    int m = static_cast<int>(e.size());
    e.push_back(Node{u, v, g[u], f, 0, c});
    g[u] = m;
    e.push_back(Node{v, u, g[v], 0, 0, -c});
    g[v] = m + 1;
    return m;
  }

  bool Extend(int s, int t) {
    std::fill(vis.begin(), vis.end(), 0);
    std::fill(dis.begin(), dis.end(), inf);
    std::queue<int> queue;
    dis[s] = 0;
    queue.push(s);
    while (!queue.empty()) {
      int u = queue.front();
      queue.pop();
      vis[u] = false;
      for (int it = g[u]; ~it; it = e[it].nxt) {
        int v = e[it].to;
        if (e[it].cap > e[it].flow && dis[v] > dis[u] + e[it].cost) {
          dis[v] = dis[u] + e[it].cost;
          pre[v] = it;
          if (!vis[v]) {
            queue.push(v);
          }
          vis[v] = true;
        }
      }
    }
    return dis[t] < inf;
  }

  std::pair<int, int> Solve(int s, int t) {
    int max_flow = 0;
    int min_cost = 0;
    while (Extend(s, t)) {
      int delta = inf;
      for (int u = t; u != s; u = e[pre[u]].from) {
        delta = std::min(delta, e[pre[u]].cap - e[pre[u]].flow);
      }
      min_cost += delta * dis[t];
      max_flow += delta;
      for (int u = t; u != s; u = e[pre[u]].from) {
        e[pre[u]].flow += delta;
        e[pre[u] ^ 1].flow -= delta;
      }
    }
    return {max_flow, min_cost};
  }
};

} // namespace alkaid_sd

#endif // ALKAID_SD_SRC_UTIL_MCMF_H_
