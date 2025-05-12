#include <iostream>
#include <iomanip>
#include <cassert>
#include <chrono>
#include <random>
#include <algorithm>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <queue>
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

struct Dir
{
	int d;
	Dir(int d) : d(d) {}
	Dir() : d(0) {}

	int operator-(const Dir &dir) const
	{
		return d - dir.d;
	}

	bool operator==(const Dir &dir) const
	{
		return d == dir.d;
	}

	int dist(const Dir &dir) const
	{
		return min(abs(d - dir.d), 4 - abs(d - dir.d));
	}

	void set_dir(int d)
	{
		this->d = d;
		return;
	}
	void rotate90()
	{
		d = (d + 1) % 4;
		return;
	}
	pair<int, int> vec()
	{
		if (d == 0)
			return {0, 1};
		if (d == 1)
			return {1, 0};
		if (d == 2)
			return {0, -1};
		if (d == 3)
			return {-1, 0};
		return {0, 0};
	}
	bool reachable(pair<int, int> p, int siz, int l)
	{
		if (d == 0)
			return p.second >= l;
		if (d == 1)
			return p.first >= l;
		if (d == 2)
			return p.second <= siz - 1 - l;
		if (d == 3)
			return p.first <= siz - 1 - l;
		return false;
	}
};

struct Solver
{
public:
	Solver() : rnd(Random(42)), timekeeper(TimeKeeperDouble(2900))
	{
		input();
		init();
	}

	void solve()
	{
		while (true)
		{
			timekeeper.setNowTime();
			if (timekeeper.isTimeOver())
				break;
			solve_init();
			_solve();
		}
		while (true)
		{ // 答えの圧縮
			bool changed = false;
			for (int i = 1; i < (int)ans.size(); i++)
				if (ans[i][0] == '.')
				{
					bool ok = true;
					for (int j = 0; j < (int)ans[i].size(); j++)
					{
						if (ans[i][j] == '.')
							continue;
						else if (ans[i][j] == 'P')
						{
							if (ans[i - 1][j] != '.')
								ok = false;
						}
						else
						{
							int pre_idx = -1;
							for (int k = i - 1; k >= 0; k--)
								if (ans[k][j + V] == 'P')
								{
									pre_idx = k;
									break;
								}
							for (int k = pre_idx + 1; k < i; k++)
								if (ans[k][j] == '.')
								{
									ans[k][j] = ans[i][j];
									ans[i][j] = '.';
									break;
								}
							if (ans[i][j] != '.' && (ans[i - 1][j] != '.' || ans[i - 1][j + V] != '.'))
								ok = false;
						}
					}
					if (ok)
					{
						for (int j = 0; j < (int)ans[i].size(); j++)
							if (ans[i][j] != '.')
								ans[i - 1][j] = ans[i][j];
						ans.erase(ans.begin() + i);
						changed = true;
						break;
					}
				}
			if (!changed)
				break;
		}
		return;
	}

	void output()
	{
		cout << V << endl;
		for (int i = 0; i < V - 1; i++)
			cout << 0 << " " << best_length[i] << endl;
		cout << best_sx << " " << best_sy << endl;
		for (string s : ans)
			cout << s << endl;
		return;
	}

private:
	int N, M, V;
	vector<string> f_s, f_t, s, t;
	int sx, sy, best_sx, best_sy;

	vector<string> ans, tmp;
	vector<pair<int, int>> f_place_s, f_place_t;
	vector<bool> f_carried;
	vector<int> f_holding;
	vector<Dir> f_dirs;

	vector<pair<int, int>> place_s, place_t;
	vector<bool> carried;
	vector<int> holding;
	vector<Dir> dirs;
	vector<int> length, best_length;

	Random rnd;
	TimeKeeperDouble timekeeper;

	void input()
	{
		cin >> N >> M >> V;
		f_s.resize(N);
		for (int i = 0; i < N; i++)
			cin >> f_s[i];
		f_t.resize(N);
		for (int i = 0; i < N; i++)
			cin >> f_t[i];
		return;
	}

	void init()
	{
		for (int i = 0; i < N; i++)
			for (int j = 0; j < N; j++)
			{
				if (f_s[i][j] == '1')
					f_place_s.push_back({i, j});
				if (f_t[i][j] == '1')
					f_place_t.push_back({i, j});
			}
		f_carried.resize(M, false), carried.resize(M, false);
		for (int i = 0; i < M; i++)
			for (int j = 0; j < M; j++)
				if (f_place_t[i] == f_place_s[j])
					f_carried[i] = true;
		f_holding.resize(V - 1, -1), holding.resize(V - 1, -1);
		f_dirs.resize(V - 1, Dir(0)), dirs.resize(V - 1, Dir(0));
		length.resize(V - 1, 1);
		return;
	}

	void solve_init()
	{
		s = f_s;
		t = f_t;
		place_s = f_place_s;
		place_t = f_place_t;
		carried = f_carried;
		holding = f_holding;
		dirs = f_dirs;
		for (int i = 0; i < V - 1; i++)
			length[i] = rnd.nextInt(2 * N / 3) + 1;
		// int idx = min_element(length.begin(), length.end()) - length.begin();
		// for(int i = 0; i < M; i++)if(dirs[idx].reachable(f_place_s[i], N, length[idx]))
		// {
		// 	sx = f_place_s[i].first, sy = f_place_s[i].second-length[idx];
		// 	break;
		// }
		int idx = 0;
		while (!f_dirs[0].reachable(f_place_s[idx], N, 1))
			idx++;
		sx = f_place_s[idx].first, sy = f_place_s[idx].second - 1;
		return;
	}

	int dist(pair<int, int> p1, pair<int, int> p2)
	{
		return abs(p1.first - p2.first) + abs(p1.second - p2.second);
	}

	pair<int, int> required_point(pair<int, int> p, Dir d, int L)
	{
		return {p.first - d.vec().first * L, p.second - d.vec().second * L};
	}

	string move(pair<int, int> from, pair<int, int> to)
	{
		string res = "";
		if (from.first < to.first)
			res += string(to.first - from.first, 'D');
		if (from.first > to.first)
			res += string(from.first - to.first, 'U');
		if (from.second < to.second)
			res += string(to.second - from.second, 'R');
		if (from.second > to.second)
			res += string(from.second - to.second, 'L');
		return res;
	}

	tuple<pair<int, int>, pair<int, int>, int, int, Dir> find_nearest_uncarried(pair<int, int> p)
	{
		set<int> usable;
		for (int i = 0; i < V - 1; i++)
			if (holding[i] == -1)
				usable.insert(length[i]);
		set<int> visited;
		queue<pair<pair<int, int>, int>> Q;
		Q.push({p, 0});
		visited.insert(p.first * N + p.second);
		pair<int, int> to = {-1, -1}, real_to = {-1, -1};
		int dist = 2e9, length = -1;
		Dir dir;
		while (!Q.empty())
		{
			auto [now, d] = Q.front();
			Q.pop();
			bool found = false;
			for (int k = 0; k < 4; k++)
				if (!found)
					for (int l : usable)
					{
						int tox = now.first + Dir(k).vec().first * l, toy = now.second + Dir(k).vec().second * l;
						if (!(0 <= tox && tox < N && 0 <= toy && toy < N))
							continue;
						if (s[tox][toy] == '1' && t[tox][toy] != '1')
						{
							to = {tox, toy};
							real_to = now;
							dist = d;
							length = l;
							dir = dirs[k];
							found = true;
							break;
						}
					}
			for (int k = 0; k < 4; k++)
			{
				int tox = now.first + Dir(k).vec().first, toy = now.second + Dir(k).vec().second;
				if (!(0 <= tox && tox < N && 0 <= toy && toy < N))
					continue;
				if (visited.find(tox * N + toy) != visited.end())
					continue;
				Q.push({{tox, toy}, d + 1});
				visited.insert(tox * N + toy);
			}
			if (found)
				break;
		}
		return make_tuple(to, real_to, dist, length, dir);
	}
	tuple<pair<int, int>, pair<int, int>, int, int, Dir> find_nearest_unput(pair<int, int> p)
	{
		set<int> usable;
		for (int i = 0; i < V - 1; i++)
			if (holding[i] != -1)
				usable.insert(length[i]);
		set<int> visited;
		queue<pair<pair<int, int>, int>> Q;
		Q.push({p, 0});
		visited.insert(p.first * N + p.second);
		pair<int, int> to = {-1, -1}, real_to = {-1, -1};
		int dist = 2e9, length = -1;
		Dir dir;
		while (!Q.empty())
		{
			auto [now, d] = Q.front();
			Q.pop();
			bool found = false;
			for (int k = 0; k < 4; k++)
				if (!found)
					for (int l : usable)
					{
						int tox = now.first + Dir(k).vec().first * l, toy = now.second + Dir(k).vec().second * l;
						if (!(0 <= tox && tox < N && 0 <= toy && toy < N))
							continue;
						if (t[tox][toy] == '1' && s[tox][toy] != '1')
						{
							to = {tox, toy};
							real_to = now;
							dist = d;
							length = l;
							dir = dirs[k];
							found = true;
							break;
						}
					}
			for (int k = 0; k < 4; k++)
			{
				int tox = now.first + Dir(k).vec().first, toy = now.second + Dir(k).vec().second;
				if (!(0 <= tox && tox < N && 0 <= toy && toy < N))
					continue;
				if (visited.find(tox * N + toy) != visited.end())
					continue;
				Q.push({{tox, toy}, d + 1});
				visited.insert(tox * N + toy);
			}
			if (found)
				break;
		}
		return make_tuple(to, real_to, dist, length, dir);
	}

	bool move_and_catch_or_put(int idx, pair<int, int> &now, pair<int, int> to)
	{
		int min_dist = 1e9;
		int best_d = -1;
		for (int d = 0; d < 4; d++)
		{
			Dir tmp = dirs[idx];
			for (int i = 0; i < d; i++)
				tmp.rotate90();
			if (!tmp.reachable(to, N, length[idx]))
				continue;
			if (dist(now, required_point(to, tmp, length[idx])) < min_dist)
			{
				min_dist = dist(now, required_point(to, tmp, length[idx]));
				best_d = d;
			}
		}
		if (best_d == -1)
			return false;
		string rotate = "";
		if (best_d == 0)
			rotate = "";
		else if (best_d == 1)
			rotate = "R";
		else if (best_d == 2)
			rotate = "RR";
		else if (best_d == 3)
			rotate = "L";
		for (int i = 0; i < best_d; i++)
			dirs[idx].rotate90();
		pair<int, int> real_to = required_point(to, dirs[idx], length[idx]);
		string res = move(now, real_to);
		if (rotate.size() < res.size())
			rotate += string(res.size() - rotate.size(), '.');
		else
			res += string(rotate.size() - res.size(), '.');
		for (int i = 0; i < (int)res.size(); i++)
		{
			string op = "";
			op += res[i];
			op += string(idx, '.') + rotate[i] + string(V - 2 - idx, '.');
			if (i + 1 == (int)res.size())
				op += string(idx + 1, '.') + 'P' + string(V - 2 - idx, '.');
			else
				op += string(V, '.');
			assert((int)op.size() == 2 * V);
			tmp.push_back(op);
		}
		if (res.size() == 0)
			tmp.push_back(string(V + idx + 1, '.') + 'P' + string(V - 2 - idx, '.'));
		now = real_to;
		return true;
	}

	void _solve()
	{
		tmp.clear();
		pair<int, int> now = {sx, sy};
		while (true)
		{
			auto [to1, real_to1, dist1, length1, dir1] = find_nearest_uncarried(now);
			auto [to2, real_to2, dist2, length2, dir2] = find_nearest_unput(now);
			if (to1.first == -1 && to2.first == -1)
				break;
			int hold = 0, unhold = 0;
			for (int i = 0; i < V - 1; i++)
				(holding[i] == -1 ? unhold : hold)++;
			int r = rnd.nextInt(100);
			if (unhold != 0 && dist1 != 2e9 && ((hold == 0 || dist1 < dist2 || r < 3)))
			{
				int takoyaki_idx = -1;
				for (int i = 0; i < M; i++)
					if (place_s[i] == to1)
					{
						takoyaki_idx = i;
						break;
					}
				int root_idx = -1, min_dis = 4;
				for (int i = 0; i < V - 1; i++)
					if (holding[i] == -1 && length[i] == length1)
					{
						if (dirs[i].dist(dir1) < min_dis)
						{
							min_dis = dirs[i].dist(dir1);
							root_idx = i;
						}
					}
				move_and_catch_or_put(root_idx, now, to1);
				holding[root_idx] = takoyaki_idx;
				s[place_s[takoyaki_idx].first][place_s[takoyaki_idx].second] = '0';
				place_s[takoyaki_idx] = make_pair(-2e9, -2e9);
			}
			else
			{
				int put_idx = -1;
				for (int i = 0; i < M; i++)
					if (place_t[i] == to2)
					{
						put_idx = i;
						break;
					}
				int root_idx = -1, min_dis = 4;
				for (int i = 0; i < V - 1; i++)
					if (holding[i] != -1 && length[i] == length2)
					{
						if (dirs[i].dist(dir2) < min_dis)
						{
							min_dis = dirs[i].dist(dir2);
							root_idx = i;
						}
					}
				move_and_catch_or_put(root_idx, now, to2);
				place_s[holding[root_idx]] = to2;
				s[to2.first][to2.second] = '1';
				holding[root_idx] = -1;
				carried[put_idx] = true;
			}
		}
		for (int i = 0; i < N; i++)
			for (int j = 0; j < N; j++)
				if (t[i][j] == '1' && s[i][j] != '1')
					return;
		if (ans.size() == 0 || tmp.size() < ans.size())
		{
			ans = tmp;
			best_length = length;
			best_sx = sx;
			best_sy = sy;
		}
		return;
	}
};

int main()
{
	// TimeKeeperDouble ALLTIME(10000);
	Solver solver;
	solver.solve();
	solver.output();
	// ALLTIME.setNowTime();
	// cerr << "Time: " << ALLTIME.getNowTime() << "ms" << endl;
	return 0;
}