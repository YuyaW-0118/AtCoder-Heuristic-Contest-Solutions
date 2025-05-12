#pragma GCC optimize("O3")
#pragma GCC target("avx2")
#pragma GCC optimize("unroll-loops")
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include <algorithm>
#include <vector>
#include <set>
#include <map>
using namespace std;
#define debug(x) cerr << #x << " = " << (x) << endl

struct IOSetUp
{
	IOSetUp()
	{
		cin.tie(nullptr)->sync_with_stdio(false);
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

	Random(const long long seed = 0) : mt_(std::mt19937(seed)) {}

	inline long long nextInt(const long long m)
	{
		uniform_int_distribution<long long> di(0, m - 1);
		return di(mt_);
	}
	inline double nextDouble() { return dd_(mt_); }
	inline double nextLog() { return log(dd_(mt_)); }
};

////////////////////////////////////////////////////////////////

struct op
{
	long long x, y, xx, yy;
	bool operator==(const op &o) const
	{
		return x == o.x && y == o.y && xx == o.xx && yy == o.yy;
	}
	bool operator<(const op &o) const
	{
		return x == o.x ? y == o.y ? xx == o.xx ? yy < o.yy : xx < o.xx : y < o.y : x < o.x;
	}
};
ostream &operator<<(ostream &os, const op &o)
{
	os << o.x << " " << o.y << " " << o.xx << " " << o.yy;
	return os;
}

struct Solver
{
public:
	Random rnd;
	TimeKeeperDouble timekeeper;
	Solver() : rnd(Random(42)), timekeeper(TimeKeeperDouble(1900))
	{
		input();
	}

	void solve()
	{
		iota(idx.begin(), idx.end(), 0);
		sort(idx.begin(), idx.end(), [&](long long i, long long j)
			 { return A[i] + B[i] < A[j] + B[j]; });
		G.resize(N + 1);
		done.push_back({0, 0});
		A.push_back(0), B.push_back(0);
		for (long long i : idx)
		{
			long long min_cost = 2e9;
			long long min_idx = -1;
			for (long long j = 0; j < (long long)done.size(); j++)
			{
				if (!(done[j].first <= A[i] && done[j].second <= B[i]))
					continue;
				long long cost = A[i] - done[j].first + B[i] - done[j].second;
				if (cost < min_cost)
				{
					min_cost = cost;
					min_idx = j;
				}
			}
			long long idx = -1;
			for (long long j = 0; j < N + 1; j++)
				if (A[j] == done[min_idx].first && B[j] == done[min_idx].second)
					idx = j;
			done.push_back({A[i], B[i]});
			G[idx].push_back(i);
		}
		for (long long i = 0; i < N + 1; i++)
		{
			set<long long> s;
			for (long long j : G[i])
				s.insert(j);
			vector<pair<long long, long long>> tmp;
			tmp.push_back({A[i], B[i]});

			while (!s.empty())
			{
				long long MINA = 2e9, MINB = 2e9;
				for (long long j : s)
				{
					MINA = min(MINA, A[j]);
					MINB = min(MINB, B[j]);
				}
				if (MINA == 2e9)
					break;
				long long idx = -1, MIN = 2e9;
				for (long long j = 0; j < (long long)tmp.size(); j++)
				{
					long long a = tmp[j].first, b = tmp[j].second;
					if (!(a <= MINA && b <= MINB))
						continue;
					long long cost = MINA - a + MINB - b;
					if (cost < MIN)
					{
						MIN = cost;
						idx = j;
					}
				}
				if (tmp[idx].first != MINA && tmp[idx].second != MINB)
				{
					ans.push_back({tmp[idx].first, tmp[idx].second, MINA, tmp[idx].second});
					ans.push_back({MINA, tmp[idx].second, MINA, MINB});
					tmp.push_back({MINA, tmp[idx].second});
					tmp.push_back({MINA, MINB});
				}
				else
				{
					ans.push_back({tmp[idx].first, tmp[idx].second, MINA, MINB});
					tmp.push_back({MINA, MINB});
				}
				set<long long> t;
				for (long long j : s)
				{
					if (A[j] == MINA || B[j] == MINB)
					{
						ans.push_back({MINA, MINB, A[j], B[j]});
						tmp.push_back({A[j], B[j]});
					}
					else
						t.insert(j);
				}
				s = t;
			}
		}
		sort(ans.begin(), ans.end(), [&](op a, op b)
			 { return a < b; });
		int cnt = 0;
		while (true)
		{
			timekeeper.setNowTime();
			if (timekeeper.isTimeOver())
				break;
			if (!improve())
				return;
			sort(ans.begin(), ans.end(), [&](op a, op b)
				 { return a < b; });
			cnt++;
			if (cnt == 2)
				break;
		}
		output();
		return;
	}

	bool improve()
	{
		map<long long, vector<pair<long long, long long>>> x_line, y_line;
		for (long long i = 0; i < (long long)ans.size(); i++)
		{
			if (ans[i].x == ans[i].xx)
				x_line[ans[i].x].push_back({ans[i].y, ans[i].yy});
			else
				y_line[ans[i].y].push_back({ans[i].x, ans[i].xx});
		}
		for (long long i = 0; i < (long long)ans.size(); i++)
		{
			bool found = false;
			if (ans[i].x == ans[i].xx && ans[i].y == ans[i].yy)
				continue;
			else if (ans[i].x == ans[i].xx)
			{
				long long same_x = ans[i].xx, y = ans[i].yy;
				long long dist = ans[i].yy - ans[i].y;
				auto itr = x_line.begin();
				while (itr->first != same_x)
					itr++;
				while (itr != x_line.begin())
				{
					if (found)
						break;
					itr--;
					if (same_x - itr->first >= dist)
						break;
					if (itr->second.size() == 0)
						continue;
					for (pair<long long, long long> p : itr->second)
						if (p.first <= y && y <= p.second)
						{
							y_line[y].push_back({itr->first, same_x});
							x_line[itr->first].push_back({p.first, y});
							x_line[itr->first].push_back({y, p.second});
							for (auto it = x_line[same_x].begin(); it != x_line[same_x].end(); it++)
								if (it->first == ans[i].y && it->second == ans[i].yy)
								{
									x_line[same_x].erase(it);
									break;
								}
							for (auto it = x_line[itr->first].begin(); it != x_line[itr->first].end(); it++)
								if (it->first == p.first && it->second == p.second)
								{
									x_line[itr->first].erase(it);
									break;
								}
							// if(x_line[same_x].empty()) x_line.erase(same_x);
							found = true;
						}
				}
			}
			else
			{
				long long x = ans[i].xx, same_y = ans[i].yy;
				long long dist = ans[i].xx - ans[i].x;
				auto itr = y_line.begin();
				while (itr->first != same_y)
					itr++;
				while (itr != y_line.begin())
				{
					if (found)
						break;
					itr--;
					if (same_y - itr->first >= dist)
						break;
					if (itr->second.size() == 0)
						continue;
					for (pair<long long, long long> p : itr->second)
						if (p.first <= x && x <= p.second)
						{
							x_line[x].push_back({itr->first, same_y});
							y_line[itr->first].push_back({p.first, x});
							y_line[itr->first].push_back({x, p.second});
							for (auto it = y_line[same_y].begin(); it != y_line[same_y].end(); it++)
								if (it->first == ans[i].x && it->second == ans[i].xx)
								{
									y_line[same_y].erase(it);
									break;
								}
							for (auto it = y_line[itr->first].begin(); it != y_line[itr->first].end(); it++)
								if (it->first == p.first && it->second == p.second)
								{
									y_line[itr->first].erase(it);
									break;
								}
							// if(y_line[same_y].empty()) y_line.erase(same_y);
							found = true;
						}
				}
			}
		}
		vector<op> tmp;
		for (auto &p : x_line)
			if (p.second.size() != 0)
				for (auto q : p.second)
					tmp.push_back({p.first, q.first, p.first, q.second});
		for (auto &p : y_line)
			if (p.second.size() != 0)
				for (auto q : p.second)
					tmp.push_back({q.first, p.first, q.second, p.first});
		if (tmp.size() > 5 * N)
		{
			output();
			return false;
		}
		else
		{
			ans.clear();
			for (auto &p : tmp)
				ans.push_back(p);
			return true;
		}
	}

	void input()
	{
		long long _;
		cin >> _;
		A.resize(N);
		B.resize(N);
		for (long long i = 0; i < N; i++)
			cin >> A[i] >> B[i];
		idx.resize(N);
		return;
	}

	void output()
	{
		cout << ans.size() << endl;
		for (auto &a : ans)
			cout << a << endl;
		return;
	}

private:
	// variables
	const long long N = 1'000;
	vector<long long> A, B;

	vector<op> ans;

	vector<long long> idx;
	vector<pair<long long, long long>> done;
	vector<vector<long long>> G;
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