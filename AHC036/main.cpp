#pragma GCC optimize("O3")
#pragma GCC target("avx2")
#pragma GCC optimize("unroll-loops")
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include <algorithm>
#include <cassert>
#include <queue>
#include <vector>
using namespace std;

#define debug(x) cerr << #x << " = " << (x) << endl

struct IOSetUp
{
	IOSetUp()
	{
		ios::sync_with_stdio(false);
		cin.tie(nullptr);
		cout << fixed << setprecision(16);
	}
} iosetup;

class TimeKeeperDouble
{
private:
	std::chrono::high_resolution_clock::time_point start_time_;
	double time_threshold_;

	double now_time_ = 0;

public:
	TimeKeeperDouble(const double time_threshold)
		: start_time_(std::chrono::high_resolution_clock::now()),
		  time_threshold_(time_threshold) {}

	void setNowTime()
	{
		auto diff = std::chrono::high_resolution_clock::now() - this->start_time_;
		this->now_time_ =
			std::chrono::duration_cast<std::chrono::microseconds>(diff).count() *
			1e-3;
	}

	double getNowTime() const { return this->now_time_; }
	bool isTimeOver() const { return now_time_ >= time_threshold_; }
};

class Random
{
public:
	std::mt19937 mt_;
	uniform_real_distribution<double> dd_{0, 1.0};

	Random(const int seed = 0) : mt_(std::mt19937(seed)) {}

	inline int nextInt(const int m)
	{
		uniform_int_distribution<int> di(0, m - 1);
		return di(mt_);
	}
	inline double nextDouble() { return dd_(mt_); }
	inline double nextLog() { return log(dd_(mt_)); }
};

struct Point
{
	int x, y;
	int dist(Point b) { return (x - b.x) * (x - b.x) + (y - b.y) * (y - b.y); }
};

struct Query
{
public:
	int signal_cnt = 0;
	vector<vector<int>> buffer;

	void push(vector<int> v)
	{
		if (v[0] == 0)
		{
			signal_cnt++;
			buffer.push_back({0, v[1], v[2], v[3]});
		}
		else
			buffer.push_back({1, v[1]});
	}

	void output()
	{
		for (auto &v : buffer)
		{
			if (v[0] == 0)
				signal(v[1], v[2], v[3]);
			else
				move(v[1]);
		}
		buffer.clear();
		return;
	}

private:
	void signal(int l, int PA, int PB)
	{
		cout << "s " << l << " " << PA << " " << PB << endl;
		return;
	}
	void move(int v)
	{
		cout << "m " << v << endl;
		return;
	}
};

class Solver
{
public:
	Random rnd;
	TimeKeeperDouble timekeeper;
	Query query;

	Solver() : rnd(Random(42)), timekeeper(TimeKeeperDouble(2950))
	{
		input();
	}

	void input()
	{
		int _;
		cin >> _ >> M >> _ >> LA >> LB;
		G.resize(N);
		GG.resize(N, vector<bool>(N, false));
		t.resize(T);
		xy.resize(N);
		for (int i = 0; i < M; i++)
		{
			int u, v;
			cin >> u >> v;
			G[u].push_back(v);
			G[v].push_back(u);
			GG[u][v] = GG[v][u] = true;
		}
		for (int i = 0; i < T; i++)
			cin >> t[i];
		for (int i = 0; i < N; i++)
			cin >> xy[i].x >> xy[i].y;

		A.resize(LA, 0);
		B.resize(LB, -1);
		sig.resize(N, false);
		belong.resize(N, -1);
		graph.resize(N, vector<pair<int, int>>(N, make_pair(-1, -1)));
		return;
	}

	void solve()
	{
		grouped_graph_init();
		A_init();
		tour();
		query.output();
		return;
	}

	void grouped_graph_init()
	{
		groups = divide_graph();
		groups_siz = groups.size();
		for (int i = 0; i < groups_siz; i++)
			for (int v : groups[i])
				belong[v] = i;
		for (int i = 0; i < N; i++)
		{
			for (int to : G[i])
			{
				if (belong[i] == belong[to])
					continue;
				graph[belong[i]][belong[to]] = make_pair(i, to);
				graph[belong[to]][belong[i]] = make_pair(to, i);
			}
		}
	}

	vector<vector<int>> divide_graph()
	{
		vector<vector<int>> res;
		vector<bool> visited(N, false);
		vector<int> group;

		auto dfs = [&](auto &self, int cur) -> void
		{
			if (group.size() == LB)
			{
				res.push_back(group);
				group.clear();
				return;
			}
			group.push_back(cur);
			visited[cur] = true;
			bool all_visited = true;
			vector<int> search_order = G[cur];
			sort(search_order.begin(), search_order.end(), [&](const int &a, const int &b)
				 {
					 double suma = 0, sumb = 0;
					 for (int pre : group)
						 suma += sqrt(xy[pre].dist(xy[a])), sumb += sqrt(xy[pre].dist(xy[b]));
					 return suma > sumb; // to be optimized
				 });
			for (int v : search_order)
				if (!visited[v])
				{
					self(self, v);
					all_visited = false;
				}
			if (all_visited)
			{
				res.push_back(group);
				group.clear();
			}
			return;
		};

		vector<int> idx(N);
		iota(idx.begin(), idx.end(), 0);
		sort(idx.begin(), idx.end(), [&](const int &l, const int &r)
			 { return xy[l].x < xy[r].x; });

		for (int i = 0; i < N; i++)
			if (!visited[idx[i]])
			{
				dfs(dfs, idx[i]);
			}

		return res;
	}

	void improve_group_size()
	{
		// 焼きなまし法を用いて、LB個以下毎のグループ数を最小化する
		// 初期解はdivide_graph()で求めたものを用いる
		// 近傍操作は、頂点iとその連結な頂点jを選び、iがjのグループに連結になる辺が存在する場合、iをjのグループに移動する
		// jについては、jの近傍の頂点を選び、頂点数がLBを超えないようにする

		TimeKeeperDouble TIME(1500);

		double start_temp = 1e-1, end_temp = 1e-3;
		int now_size = groups_siz;

		while (true)
		{
			TIME.setNowTime();
			if (TIME.isTimeOver())
				break;
			double temp = start_temp + (end_temp - start_temp) * TIME.getNowTime() / 1500;
			// 近傍操作
		}
	}

	int neighborhood_search()
	{
	}

	void A_init()
	{
		group_idx_A.resize(groups_siz, -1);
		int now = 0;
		for (int i = 0; i < groups_siz; i++)
		{
			group_idx_A[i] = now;
			for (int v : groups[i])
				A[now++] = v;
		}
		for (int i = 0; i < LA; i++)
			cout << A[i] << " \n"[i == LA - 1];
	}

	void tour()
	{
		int now = 0, now_group = belong[0];
		// 0. 頂点0のグループの信号を付ける
		query.push({0, (int)groups[belong[0]].size(), group_idx_A[belong[0]], 0});
		for (int time = 0; time < T; time++)
		{
			int to = t[time], to_group = belong[to];
			// 1. now -> toへの最短信号変更回数をbfsで計算
			vector<int> dist(groups_siz, -1);
			vector<int> prev(groups_siz, -1);
			queue<int> Q;
			Q.push(now_group);
			dist[now_group] = 0;
			while (!Q.empty())
			{
				int cur = Q.front();
				Q.pop();
				for (int i = 0; i < groups_siz; i++)
				{
					if (graph[cur][i].first == -1)
						continue;
					if (dist[i] != -1)
						continue;
					dist[i] = dist[cur] + 1;
					prev[i] = cur;
					Q.push(i);
				}
			}

			// 2. bfsを復元する
			vector<int> route;
			for (int i = to_group; i != -1; i = prev[i])
				route.push_back(i);
			reverse(route.begin(), route.end());

			// 3.group毎に下記を行う
			//     3.1 次のgroupに行くための頂点に移動する。
			// 　　    この移動はgroup内で行い、bfsで経路を求める。
			//     3.2 信号を変更して、次のgroupに行く。
			for (int i = 0; i + 1 < (int)route.size(); i++)
			{
				int from = route[i], next = route[i + 1];
				int bridge = graph[from][next].first;
				move_in_group(from, now, bridge);
				query.push({0, (int)groups[next].size(), group_idx_A[next], 0});
				query.push({1, graph[from][next].second});
				now = graph[from][next].second;
			}

			// 4. toに移動する
			move_in_group(to_group, now, to);
			now = to, now_group = to_group;
		}
	}

	void move_in_group(int group_idx, int now, int to)
	{
		// group内でnow -> toに移動する
		vector<int> dist(groups[group_idx].size(), -1);
		vector<int> prev(groups[group_idx].size(), -1);
		queue<int> Q;
		for (int i = 0; i < (int)groups[group_idx].size(); i++)
		{
			if (groups[group_idx][i] == now)
				dist[i] = 0, Q.push(i);
		}
		while (!Q.empty())
		{
			int cur = Q.front();
			Q.pop();
			if (groups[group_idx][cur] == to)
				break;
			for (int i = 0; i < (int)groups[group_idx].size(); i++)
			{
				if (GG[groups[group_idx][cur]][groups[group_idx][i]] && dist[i] == -1)
				{
					dist[i] = dist[cur] + 1;
					prev[i] = cur;
					Q.push(i);
				}
			}
		}
		vector<int> route;
		int to_idx = -1;
		for (int i = 0; i < (int)groups[group_idx].size(); i++)
			if (groups[group_idx][i] == to)
				to_idx = i;
		for (int i = to_idx; i != -1; i = prev[i])
			route.push_back(i);
		reverse(route.begin(), route.end());
		for (int i = 1; i < (int)route.size(); i++)
			query.push({1, groups[group_idx][route[i]]});
		return;
	}

private:
	// variables
	const int N = 600, T = 600;
	int M, LA, LB;
	vector<vector<int>> G;
	vector<vector<bool>> GG;
	vector<int> t;
	vector<Point> xy;

	vector<int> A, B;
	vector<bool> sig;
	vector<vector<int>> groups;
	int groups_siz;
	vector<int> belong;
	vector<vector<pair<int, int>>> graph;
	vector<int> group_idx_A;
};

int main()
{
	TimeKeeperDouble ALLTIME(10000);
	Solver solver;
	solver.solve();
	ALLTIME.setNowTime();
	// cerr << "Time: " << ALLTIME.getNowTime() << "ms" << endl;
	return 0;
}