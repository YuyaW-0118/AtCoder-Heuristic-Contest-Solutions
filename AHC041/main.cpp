#include <iostream>
#include <cassert>
#include <chrono>
#include <algorithm>
#include <random>
#include <vector>
#include <queue>
#include <tuple>
#include <set>
using namespace std;
#define debug(x) cerr << #x << " = " << (x) << endl

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
	std::uniform_real_distribution<double> dd_{0, 1.0};

	Random(const int seed = 0) : mt_(std::mt19937(seed)) {}

	inline int nextInt(const int m)
	{
		std::uniform_int_distribution<int> di(0, m - 1);
		return di(mt_);
	}
	inline double nextDouble() { return dd_(mt_); }
	inline double nextLog() { return log(dd_(mt_)); }
};

struct Point
{
	int x, y;
};
istream &operator>>(istream &is, Point &p)
{
	is >> p.x >> p.y;
	return is;
}

class Solver
{
	const int N = 1000, H = 10;
	int M;
	vector<int> A;
	vector<vector<int>> G;
	vector<Point> P;

	vector<int> ans;
	vector<int> cur;
	vector<vector<bool>> G2;
	vector<vector<int>> child;

	Random rnd;
	TimeKeeperDouble timekeeper1, timekeeper2;

	void input()
	{
		int _;
		cin >> _ >> M >> _;
		A.resize(N);
		for (int i = 0; i < N; i++)
			cin >> A[i];
		G.resize(N);
		for (int i = 0; i < M; i++)
		{
			int u, v;
			cin >> u >> v;
			G[u].push_back(v);
			G[v].push_back(u);
		}
		P.resize(N);
		for (int i = 0; i < N; i++)
			cin >> P[i];
		return;
	}

	void rand_solve(int &best_score)
	{
		vector<int> dist(N, -1);
		vector<int> prev(N, -1);
		set<pair<int, int>> left;
		for (int i = 0; i < N; i++)
			left.insert(make_pair(A[i], i));

		while (!left.empty())
		{
			vector<pair<int, int>> skipped;
			while (left.size() > 1 && rnd.nextDouble() < 0.8)
			{
				skipped.push_back(*left.begin());
				left.erase(left.begin());
			}
			int s = left.begin()->second;
			dist[s] = 1;

			for (auto sk : skipped)
				left.insert(sk);

			auto dfs = [&](auto &self, int now, int par) -> void
			{
				left.erase(make_pair(A[now], now));
				if (dist[now] > H)
					return;
				for (int to : G[now])
				{
					if (to == par)
						continue;
					if (dist[to] != -1)
						continue;
					dist[to] = dist[now] + 1;
					prev[to] = now;
					self(self, to, now);
					if (dist[now] < 3)
						break;
				}
			};

			dfs(dfs, s, -1);
		}
		int score = 0;
		for (int i = 0; i < N; i++)
			score += A[i] * dist[i];
		if (score > best_score)
		{
			best_score = score;
			ans = prev;
			cur = dist;
		}
		return;
	}

	void merge_solo_point()
	{
		for (int i = 0; i < N; i++)
		{
			vector<bool> visited(N, false);
			if (ans[i] != -1)
				continue;
			int mx = -1;
			queue<int> Q;
			set<int> group;
			int sum = 0;
			Q.push(i);
			visited[i] = true;
			while (!Q.empty())
			{
				int now = Q.front();
				Q.pop();
				group.insert(now);
				sum += A[now];
				for (int j = 0; j < N; j++)
				{
					if (!G2[now][j])
						continue;
					int to = j;
					if (visited[to])
						continue;
					visited[to] = true;
					Q.push(to);
					mx = max(mx, cur[to]);
				}
			}
			if (mx == H + 1)
				continue;
			int Max = -1, idx = -1;
			for (int to : G[i])
			{
				if (G2[i][to] || group.find(to) != group.end())
					continue;
				if (cur[to] <= H - mx - 1 && cur[to] > Max)
				{
					Max = cur[to];
					idx = to;
				}
			}
			if (Max != -1)
			{
				G[i].push_back(idx);
				G[idx].push_back(i);
				G2[i][idx] = G2[idx][i] = true;
				ans[i] = idx;
				cur[i] = Max + 1;
				continue;
			}
			int inc = 0, best_to = -1, best_to2 = -1;
			for (int to : G[i])
			{
				if (G2[i][to] || group.find(to) != group.end())
					continue;
				for (int to2 : G[to])
				{
					if (group.find(to2) != group.end())
						continue;
					if (cur[to2] >= H - mx - 1)
						continue;
					int diff = sum * (cur[to2] + 2 - cur[i]) + A[to] * (cur[to2] + 1 - cur[to]);
					if (diff > inc)
					{
						inc = diff;
						best_to = to;
						best_to2 = to2;
					}
				}
			}
			if (inc > 0)
			{
				G2[best_to][ans[best_to]] = G2[ans[best_to]][best_to] = false;
				G2[best_to][best_to2] = G2[best_to2][best_to] = true;
				G2[i][best_to] = G2[best_to][i] = true;
				ans[best_to] = best_to2;
				ans[i] = best_to;
				cur[best_to] = cur[best_to2] + 1;
				int base = cur[best_to2] + 2;
				for (int g : group)
				{
					if (g == i)
						continue;
					cur[g] = base + (cur[g] - cur[i]);
				}
				cur[i] = base;
			}
		}
	}

	void annealing(double temp)
	{
		int idx = rnd.nextInt(N);
		if (child[idx].size() != 0)
			return;
		if (cur[idx] == H + 1)
			return;
		for (int to : G[idx])
		{
			if (G2[idx][to])
				continue;
			if (child[to].size() != 0)
				continue;
			for (int to2 : G[to])
			{
				if (G2[to][to2])
					continue;
				if (to2 == idx)
					continue;
				if (cur[to2] + 2 >= H + 1)
					continue;
				int diff = A[idx] * (cur[to2] + 2 - cur[idx]) + A[to] * (cur[to2] + 1 - cur[to]);
				if (diff > 0 || rnd.nextDouble() < exp(diff / temp))
				{
					if (ans[idx] != -1)
					{
						G2[idx][ans[idx]] = G2[ans[idx]][idx] = false;
						child[ans[idx]].erase(find(child[ans[idx]].begin(), child[ans[idx]].end(), idx));
					}
					if (ans[to] != -1)
					{
						G2[to][ans[to]] = G2[ans[to]][to] = false;
						child[ans[to]].erase(find(child[ans[to]].begin(), child[ans[to]].end(), to));
					}
					G2[to][to2] = G2[to2][to] = true;
					child[to2].push_back(to);
					G2[idx][to] = G2[to][idx] = true;
					child[to].push_back(idx);
					ans[to] = to2;
					ans[idx] = to;
					cur[to] = cur[to2] + 1;
					cur[idx] = cur[to2] + 2;
					return;
				}
			}
		}
	}

public:
	Solver() : rnd(Random(0)), timekeeper1(TimeKeeperDouble(1700)), timekeeper2(TimeKeeperDouble(1990))
	{
		input();
		ans.resize(N, -1);
	}

	void solve()
	{
		for (int i = 0; i < N; i++)
			sort(G[i].begin(), G[i].end(), [&](int u, int v)
				 { return A[u] < A[v]; });
		int best_score = 0;
		while (true)
		{
			timekeeper1.setNowTime();
			if (timekeeper1.isTimeOver())
				break;
			rand_solve(best_score);
		}
		G2.resize(N, vector<bool>(N, false));
		for (int i = 0; i < N; i++)
			if (ans[i] != -1)
				G2[i][ans[i]] = G2[ans[i]][i] = true;
		merge_solo_point();
		child.resize(N);
		G2.resize(N, vector<bool>(N, false));
		for (int i = 0; i < N; i++)
			if (ans[i] != -1)
				G2[i][ans[i]] = G2[ans[i]][i] = true;
		for (int i = 0; i < N; i++)
			if (ans[i] != -1)
				child[ans[i]].push_back(i);
		for (int i = 0; i < N; i++)
		{
			if (ans[i] != -1)
				continue;
			queue<int> Q;
			Q.push(i);
			vector<bool> visited(N, false);
			cur[i] = 1;
			visited[i] = true;
			while (!Q.empty())
			{
				int now = Q.front();
				Q.pop();
				for (int j = 0; j < N; j++)
				{
					if (!G2[now][j])
						continue;
					int to = j;
					if (visited[to])
						continue;
					visited[to] = true;
					cur[to] = cur[now] + 1;
					Q.push(to);
				}
			}
		}
		double first_temp = 1e-1, last_temp = 1e-3;
		while (true)
		{
			timekeeper2.setNowTime();
			if (timekeeper2.isTimeOver())
				break;
			double temp = first_temp + (last_temp - first_temp) * timekeeper2.getNowTime() / 1990;
			annealing(temp);
		}
		return;
	}

	void output()
	{
		for (int i = 0; i < N; i++)
			cout << (cur[i] > H + 1 ? -1 : ans[i]) << " \n"[i == N - 1];
		return;
	}
};

int main()
{
	Solver solver;
	solver.solve();
	solver.output();
	return 0;
}