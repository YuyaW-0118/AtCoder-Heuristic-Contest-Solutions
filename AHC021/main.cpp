#pragma GCC optimize("Ofast")
#pragma GCC target("avx2")
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
using namespace std;

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

struct op
{
	int x1, y1, x2, y2;
};
ostream &operator<<(ostream &os, const op &o)
{
	os << o.x1 << " " << o.y1 << " " << o.x2 << " " << o.y2;
	return os;
}

// variables
constexpr int N = 30, M = N * (N + 1) / 2;
vector<vector<int>> b;

vector<op> ans;

const int dx[6] = {-1, -1, 0, 0, 1, 1};
const int dy[6] = {-1, 0, -1, 1, 0, 1};

vector<pair<int, int>> place(M);

void input();
void solve();
void output();

int main()
{
	TimeKeeperDouble ALLTIME(10000);
	input();
	solve();
	output();
	// ALLTIME.setNowTime(); cerr << setprecision(2) << "Time : " << ALLTIME.getNowTime() << "ms" << endl;
	return 0;
}

void input()
{
	b.resize(N);
	for (int i = 0; i < N; i++)
	{
		b[i].resize(i + 1);
		for (int j = 0; j <= i; j++)
			cin >> b[i][j];
	}
	return;
}

void output()
{
	cout << ans.size() << endl;
	for (auto &a : ans)
		cout << a << endl;
	return;
}

void make_first_ans()
{
	vector<vector<int>> bb = b;
	for (int i = 0; i < N; i++)
		for (int j = 0; j <= i; j++)
			place[bb[i][j]] = {i, j};
	for (int n = 0; n < M; n++)
	{
		while (true)
		{
			bool ok = false;
			int x = place[n].first, y = place[n].second;
			if (place[n] == make_pair(0, 0))
				break;
			if (y - 1 >= 0 && y < x)
			{
				int to1 = bb[x - 1][y - 1], to2 = bb[x - 1][y];
				if (n > to1 && n > to2)
					ok = true;
				else if (to1 > to2)
				{
					ans.push_back({x, y, x - 1, y - 1});
					swap(bb[x][y], bb[x - 1][y - 1]);
					place[to1] = {x, y};
					place[n] = {x - 1, y - 1};
				}
				else
				{
					ans.push_back({x, y, x - 1, y});
					swap(bb[x][y], bb[x - 1][y]);
					place[to2] = {x, y};
					place[n] = {x - 1, y};
				}
			}
			else if (y - 1 >= 0)
			{
				int to = bb[x - 1][y - 1];
				if (n > to)
					ok = true;
				else
				{
					ans.push_back({x, y, x - 1, y - 1});
					swap(bb[x][y], bb[x - 1][y - 1]);
					place[to] = {x, y};
					place[n] = {x - 1, y - 1};
				}
			}
			else if (y < x)
			{
				int to = bb[x - 1][y];
				if (n > to)
					ok = true;
				else
				{
					ans.push_back({x, y, x - 1, y});
					swap(bb[x][y], bb[x - 1][y]);
					place[to] = {x, y};
					place[n] = {x - 1, y};
				}
			}
			if (ok)
				break;
		}
	}
}

void make_rand_and()
{
	vector<vector<int>> bb = b;
	vector<op> new_ans;
	for (int i = 0; i < N; i++)
		for (int j = 0; j <= i; j++)
			place[bb[i][j]] = {i, j};
	for (int n = 0; n < M; n++)
	{
		while (true)
		{
			bool ok = false;
			int x = place[n].first, y = place[n].second;
			if (place[n] == make_pair(0, 0))
				break;
			if (y - 1 >= 0 && y < x)
			{
				int to1 = bb[x - 1][y - 1], to2 = bb[x - 1][y];
				if (n > to1 && n > to2)
					ok = true;
				else if (n < to1 && n < to2)
				{
					if ((to1 > to2 && rnd.nextDouble() < 0.99) || (to1 < to2 && rnd.nextDouble() < 0.01))
					{
						new_ans.push_back({x, y, x - 1, y - 1});
						swap(bb[x][y], bb[x - 1][y - 1]);
						place[to1] = {x, y};
						place[n] = {x - 1, y - 1};
					}
					else
					{
						new_ans.push_back({x, y, x - 1, y});
						swap(bb[x][y], bb[x - 1][y]);
						place[to2] = {x, y};
						place[n] = {x - 1, y};
					}
				}
				else if (to1 > to2)
				{
					new_ans.push_back({x, y, x - 1, y - 1});
					swap(bb[x][y], bb[x - 1][y - 1]);
					place[to1] = {x, y};
					place[n] = {x - 1, y - 1};
				}
				else
				{
					new_ans.push_back({x, y, x - 1, y});
					swap(bb[x][y], bb[x - 1][y]);
					place[to2] = {x, y};
					place[n] = {x - 1, y};
				}
			}
			else if (y - 1 >= 0)
			{
				int to = bb[x - 1][y - 1];
				if (n > to)
					ok = true;
				else
				{
					new_ans.push_back({x, y, x - 1, y - 1});
					swap(bb[x][y], bb[x - 1][y - 1]);
					place[to] = {x, y};
					place[n] = {x - 1, y - 1};
				}
			}
			else if (y < x)
			{
				int to = bb[x - 1][y];
				if (n > to)
					ok = true;
				else
				{
					new_ans.push_back({x, y, x - 1, y});
					swap(bb[x][y], bb[x - 1][y]);
					place[to] = {x, y};
					place[n] = {x - 1, y};
				}
			}
			if (ok)
				break;
		}
	}
	if (new_ans.size() < ans.size())
	{
		ans = new_ans;
	}
}

void solve()
{
	TimeKeeperDouble tick(1995);
	make_first_ans();
	int iter = 0;
	while (true)
	{
		tick.setNowTime();
		if (tick.isTimeOver())
			break;
		make_rand_and();
		iter++;
	}
	// cerr << "ITER : " << iter << endl;
	return;
}