#pragma GCC optimize("Ofast")
#pragma GCC target("avx2")
#include <bits/stdc++.h>
using namespace std;
#define all(a) (a).begin(), (a).end()

struct IOSetUp
{
	IOSetUp()
	{
		ios::sync_with_stdio(false);
		cin.tie(nullptr);
		cout << fixed << setprecision(16);
		cerr << fixed << setprecision(16);
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
} rnd;

////////////////////////////////////////////////////////////////

// variables
const int N = 50;
int sx, sy;
vector<vector<int>> t(N, vector<int>(N)), p(N, vector<int>(N));

const int dx[4] = {1, 0, -1, 0}, dy[4] = {0, 1, 0, -1};
const char dir[4] = {'D', 'R', 'U', 'L'};

vector<vector<vector<pair<int, int>>>> nxt(N, vector<vector<pair<int, int>>>(N));
vector<bool> visited(N *N);

vector<pair<int, int>> ans_path;

void input();
void solve();
void output();

int main()
{
	TimeKeeperDouble ALLTIME(10000);
	input();
	solve();
	output();
	ALLTIME.setNowTime();
	cerr << "Time : " << ALLTIME.getNowTime() << "ms" << endl;
	return 0;
}

void input()
{
	cin >> sx >> sy;
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			cin >> t[i][j];
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			cin >> p[i][j];
	return;
}

void output()
{
	for (int i = 0; i < ans_path.size() - 1; i++)
	{
		int x = ans_path[i].first, y = ans_path[i].second;
		int nx = ans_path[i + 1].first, ny = ans_path[i + 1].second;
		if (x == nx)
		{
			if (y < ny)
				cout << 'R';
			else
				cout << 'L';
		}
		else
		{
			if (x < nx)
				cout << 'D';
			else
				cout << 'U';
		}
	}
	cout << endl;
	return;
}

void make_first_ans()
{
	TimeKeeperDouble tick2(100);
	vector<pair<int, int>> now;
	int score = 0, best_score = 0;

	auto dfs = [&](auto &self, pair<int, int> point) -> void
	{
		now.push_back(point);
		score += p[point.first][point.second];
		visited[t[point.first][point.second]] = true;
		if (score > best_score)
		{
			best_score = score;
			ans_path = now;
		}
		tick2.setNowTime();
		if (tick2.isTimeOver())
			return;

		for (auto np : nxt[point.first][point.second])
		{
			if (visited[t[np.first][np.second]])
				continue;
			self(self, np);
		}

		now.pop_back();
		score -= p[point.first][point.second];
		visited[t[point.first][point.second]] = false;
		return;
	};

	dfs(dfs, {sx, sy});
	visited = vector<bool>(N * N, false);
	for (int i = 0; i < ans_path.size(); i++)
	{
		visited[t[ans_path[i].first][ans_path[i].second]] = true;
	}
	return;
}

void hill_climb(double temp)
{
	int siz = ans_path.size();
	int del = rnd.nextInt((int)(1 + 0.05 * siz));
	int start = rnd.nextInt(siz - del), end = start + del;

	int currenct_score = 0;
	vector<bool> new_visited = visited;
	int cnt = del * 4;
	for (int i = start + 1; i <= end - 1; i++)
	{
		currenct_score += p[ans_path[i].first][ans_path[i].second];
		new_visited[t[ans_path[i].first][ans_path[i].second]] = false;
	}

	int fx = ans_path[start].first, fy = ans_path[start].second;
	int gx = ans_path[end].first, gy = ans_path[end].second;
	int new_score = 0, best_score = 0;
	vector<pair<int, int>> new_path, best_path;

	auto dfs = [&](auto &self, pair<int, int> point) -> void
	{
		if (!new_visited[t[point.first][point.second]])
		{
			new_path.push_back(point);
			new_score += p[point.first][point.second];
			new_visited[t[point.first][point.second]] = true;
		}
		cnt--;
		if (cnt <= 0)
			return;
		vector<pair<int, int>> ok;
		if (point != make_pair(gx, gy))
		{
			for (auto np : nxt[point.first][point.second])
			{
				if (!new_visited[t[np.first][np.second]])
					ok.push_back(np);
				else if (np == make_pair(gx, gy))
				{
					best_score = new_score + p[np.first][np.second];
					best_path = new_path;
					cnt = 0;
					return;
				}
			}
		}
		shuffle(all(ok), rnd.mt_);
		for (auto np : ok)
			if (!new_visited[t[np.first][np.second]])
			{
				self(self, np);
				if (cnt <= 0)
					return;
			}

		new_path.pop_back();
		new_score -= p[point.first][point.second];
		new_visited[t[point.first][point.second]] = false;
		return;
	};

	dfs(dfs, {fx, fy});
	if (best_path.size() > 0 && exp((best_score - currenct_score) / temp) > rnd.nextDouble())
	{
		visited = vector<bool>(N * N, false);

		vector<pair<int, int>> tmp;
		for (int i = 0; i <= start; i++)
		{
			tmp.push_back(ans_path[i]);
			visited[t[ans_path[i].first][ans_path[i].second]] = true;
		}
		for (int i = 0; i < best_path.size(); i++)
		{
			tmp.push_back(best_path[i]);
			visited[t[best_path[i].first][best_path[i].second]] = true;
		}
		for (int i = end; i < siz; i++)
		{
			tmp.push_back(ans_path[i]);
			visited[t[ans_path[i].first][ans_path[i].second]] = true;
		}
		ans_path = tmp;
	}
	return;
}

void solve()
{
	TimeKeeperDouble tick(1990);
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				int nx = i + dx[k], ny = j + dy[k];
				if (nx < 0 || nx >= N || ny < 0 || ny >= N)
					continue;
				if (t[i][j] != t[nx][ny])
					nxt[i][j].push_back({nx, ny});
			}
		}

	make_first_ans();

	double start_temp = 150, end_temp = 0;
	while (true)
	{
		tick.setNowTime();
		if (tick.isTimeOver())
			break;
		double temp = start_temp + (end_temp - start_temp) * tick.getNowTime() / 1990.0;
		hill_climb(temp);
	}

	return;
}