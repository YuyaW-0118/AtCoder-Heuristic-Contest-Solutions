#include <iostream>
#include <cassert>
#include <chrono>
#include <algorithm>
#include <random>
#include <vector>
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

class Solver
{
	const int N = 20;
	int left;

	vector<vector<char>> _C, C;

	Random rnd;
	TimeKeeperDouble timekeeper;

	vector<pair<char, int>> ans, best_ans;

	void input()
	{
		int _;
		cin >> _;
		C.resize(N, vector<char>(N));
		_C.resize(N, vector<char>(N));
		for (int i = 0; i < N; i++)
			for (int j = 0; j < N; j++)
				cin >> _C[i][j];
		return;
	}

	void _solve()
	{
		left = 2 * N;
		C = _C;
		ans.clear();
		search();
		int iter = 0;
		while (exist_oni())
		{
			if (iter++ > 100)
				return;
			vector<pair<int, int>> oni;
			for (int i = 0; i < N; i++)
				for (int j = 0; j < N; j++)
					if (C[i][j] == 'x')
						oni.push_back({i, j});
			for (auto [i, j] : oni)
			{
				if (C[i][j] != 'x')
					continue;
				vector<int> idx(4);
				iota(idx.begin(), idx.end(), 0);
				shuffle(idx.begin(), idx.end(), rnd.mt_);
				for (int id : idx)
				{
					if (id == 0 && C[i][0] != 'o')
					{
						ans.push_back({'L', i});
						for (int k = 0; k < N - 1; k++)
							C[i][k] = C[i][k + 1];
						C[i][N - 1] = '.';
						break;
					}
					if (id == 1 && C[i][N - 1] != 'o')
					{
						ans.push_back({'R', i});
						for (int k = N - 1; k > 0; k--)
							C[i][k] = C[i][k - 1];
						C[i][0] = '.';
						break;
					}
					if (id == 2 && C[0][j] != 'o')
					{
						ans.push_back({'U', j});
						for (int k = 0; k < N - 1; k++)
							C[k][j] = C[k + 1][j];
						C[N - 1][j] = '.';
						break;
					}
					if (id == 3 && C[N - 1][j] != 'o')
					{
						ans.push_back({'D', j});
						for (int k = N - 1; k > 0; k--)
							C[k][j] = C[k - 1][j];
						C[0][j] = '.';
						break;
					}
				}
			}
			search();
		}
		if (best_ans.empty() || ans.size() < best_ans.size())
			best_ans = ans;
	}

	void search()
	{
		int border = rnd.nextInt(2) + 1;
		while (true)
		{
			vector<pair<pair<int, int>, pair<char, int>>> possibles;
			bool ex = false;
			{ // L
				for (int i = 0; i < N; i++)
				{
					int cnt = 0, len = -1;
					for (int j = 0; j < N; j++)
					{
						if (C[i][j] == 'o')
							break;
						else if (C[i][j] == 'x')
						{
							ex = true;
							cnt++;
							len = j + 1;
							possibles.push_back({{cnt, len}, {'L', i}});
						}
					}
				}
			}
			{ // R
				for (int i = 0; i < N; i++)
				{
					int cnt = 0, len = -1;
					for (int j = N - 1; j >= 0; j--)
					{
						if (C[i][j] == 'o')
							break;
						else if (C[i][j] == 'x')
						{
							ex = true;
							cnt++;
							len = N - j;
							possibles.push_back({{cnt, len}, {'R', i}});
						}
					}
				}
			}
			{ // U
				for (int j = 0; j < N; j++)
				{
					int cnt = 0, len = -1;
					for (int i = 0; i < N; i++)
					{
						if (C[i][j] == 'o')
							break;
						else if (C[i][j] == 'x')
						{
							ex = true;
							cnt++;
							len = i + 1;
							possibles.push_back({{cnt, len}, {'U', j}});
						}
					}
				}
			}
			{ // D
				for (int j = 0; j < N; j++)
				{
					int cnt = 0, len = -1;
					for (int i = N - 1; i >= 0; i--)
					{
						if (C[i][j] == 'o')
							break;
						else if (C[i][j] == 'x')
						{
							ex = true;
							cnt++;
							len = N - i;
							possibles.push_back({{cnt, len}, {'D', j}});
						}
					}
				}
			}
			if (!ex)
				return;
			sort(possibles.begin(), possibles.end(), [&](auto a, auto b)
				 { return (double)a.first.first * (double)a.first.first / a.first.second > (double)b.first.first * (double)b.first.first / b.first.second; });
			int idx = 0;
			if (left > border || rnd.nextDouble() < 0.1)
				while (rnd.nextDouble() < 0.33)
					idx++;
			if (idx >= possibles.size())
				idx = possibles.size() - 1;
			auto choice = possibles[idx];
			int time = choice.first.second;
			auto best = choice.second;
			time = (time != 1 && rnd.nextDouble() < 0.3) ? 2 : 1;
			for (int i = 0; i < time; i++)
				ans.push_back(best);
			if (best.first == 'L')
				for (int j = 0; j < N; j++)
				{
					if (j == 0 && C[best.second][j] == 'x')
						left--;
					if (j + time < N)
						C[best.second][j] = C[best.second][j + time];
					else
						C[best.second][j] = '.';
				}
			else if (best.first == 'R')
				for (int j = N - 1; j >= 0; j--)
				{
					if (j == N - 1 && C[best.second][j] == 'x')
						left--;
					if (j - time >= 0)
						C[best.second][j] = C[best.second][j - time];
					else
						C[best.second][j] = '.';
				}
			else if (best.first == 'U')
				for (int i = 0; i < N; i++)
				{
					if (i == 0 && C[i][best.second] == 'x')
						left--;
					if (i + time < N)
						C[i][best.second] = C[i + time][best.second];
					else
						C[i][best.second] = '.';
				}
			else if (best.first == 'D')
				for (int i = N - 1; i >= 0; i--)
				{
					if (i == N - 1 && C[i][best.second] == 'x')
						left--;
					if (i - time >= 0)
						C[i][best.second] = C[i - time][best.second];
					else
						C[i][best.second] = '.';
				}
		}
		return;
	}

	bool exist_oni()
	{
		for (int i = 0; i < N; i++)
			for (int j = 0; j < N; j++)
				if (C[i][j] == 'x')
					return true;
		return false;
	}

public:
	Solver() : rnd(Random(0)), timekeeper(TimeKeeperDouble(1995))
	{
		input();
	}

	void solve()
	{
		int iter = 0;
		while (true)
		{
			timekeeper.setNowTime();
			if (timekeeper.isTimeOver())
				break;
			_solve();
			iter++;
		}
		cerr << "iter = " << iter << endl;
		return;
	}

	void output()
	{
		for (auto p : best_ans)
			cout << p.first << " " << p.second << endl;
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