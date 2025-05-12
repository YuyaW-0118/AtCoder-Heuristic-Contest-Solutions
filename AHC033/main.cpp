#pragma GCC optimize("Ofast")
#pragma GCC target("avx2")
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cassert>
#include <random>
#include <algorithm>
#include <queue>
#include <tuple>
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
};

////////////////////////////////////////////////////////////////

struct dot
{
	int x, y;
	dot(int x, int y) : x(x), y(y) {}
	dot() : x(0), y(0) {}
	const bool operator==(const dot &d) const { return x == d.x && y == d.y; }
	const bool operator!=(const dot &d) const { return x != d.x || y != d.y; }
};
ostream &operator<<(ostream &os, const dot &d) { return os << d.x << " " << d.y; }

constexpr int N = 5;
vector<vector<int>> A;
string ans[N];
int BEST_SIZE = 1e9;
vector<int> idx(N);

void input();
void solve();
void output();

int main()
{
	TimeKeeperDouble ALLTIME(10000);
	input();
	solve();
	output();
	// ALLTIME.setNowTime(); cerr << "Time : " << ALLTIME.getNowTime() << "ms" << endl;
	return 0;
}

void input()
{
	int _;
	cin >> _;
	A.resize(N);
	for (int i = 0; i < N; i++)
	{
		A[i].resize(N);
		for (int j = 0; j < N; j++)
			cin >> A[i][j];
	}
	for (int i = 0; i < N; i++)
		idx[i] = i;
	return;
}

void output()
{
	int score = 0;
	for (int i = 0; i < N; i++)
		score = max(score, (int)ans[i].size());
	cerr << "SCORE : " << score << endl;
	for (int i = 0; i < N; i++)
		cout << ans[i] << endl;
	return;
}

void initialize();
string move(dot, dot);
void carry(dot, dot, string &);
dot find_nearest_movable_container(dot);
dot find_nearest_open_space(dot);
void solve_for_worstcase();

vector<string> S(N);
constexpr int INQUEUE = -1e5, END = 1e5, IMPOSSIBLE = 2 << 17;
vector<dot> place(N *N);
vector<vector<int>> exist(N, vector<int>(N));
vector<int> required(N), queueIDX(N);
vector<dot> now(N);

void solve()
{
	initialize();
	for (int j = 3; j >= 1; j--)
		for (int i = 0; i < N; i++)
		{ // 全力でコンテナをqueueから出す
			S[i] += "P";
			S[i] += string(j, 'R');
			S[i] += "Q";
			if (j != 1)
				S[i] += string(j, 'L');
			int num = A[i][3 - j];
			place[num] = {i, j};
			exist[i][j] = num;
		}
	for (int i = 0; i < N; i++)
		place[A[i][3]] = {i, 0};
	for (int i = 0; i < N; i++)
		exist[i][0] = A[i][3];
	S[1] += 'B';
	S[2] += 'B';
	S[3] += 'B';
	now[0].x = 0, now[0].y = 1;
	for (int i = 1; i < N - 1; i++)
		now[i] = {-1, -1};
	now[N - 1].x = N - 1, now[N - 1].y = 1;

	int remain = N * N;
	while (remain--)
	{
		dot to = find_nearest_movable_container({now[0].x, now[0].y});
		string move_of_zero = "";
		if (to.x != -1)
		{ // 運べるものを運ぶ
			move_of_zero += move({now[0].x, now[0].y}, to);
			int num = exist[to.x][to.y];
			carry({now[0].x, now[0].y}, {num / N, 4}, move_of_zero);
			exist[to.x][to.y] = -1;
			place[num] = {END, END};
			required[num / N]++;
			if (to.y == 0 && queueIDX[to.x] != N)
			{
				int nxt = A[to.x][queueIDX[to.x]];
				place[nxt] = {to.x, 0};
				exist[to.x][0] = nxt;
				queueIDX[to.x]++;
			}
		}
		else
		{ // 運べるものがない場合、queueの先頭を動かす
			int choice = -1;
			for (int i = 0; i < N; i++)
			{ // まだ出すべきコンテナがある行を探す(now[0].xから近い場所から)
				int up = now[0].x + i, down = now[0].x - i;
				if (up < N && queueIDX[up] != N)
				{
					choice = up;
					break;
				}
				if (down >= 0 && queueIDX[down] != N)
				{
					choice = down;
					break;
				}
			}
			S[0] += move({now[0].x, now[0].y}, {choice, 0});
			int num = exist[choice][0];
			dot to = find_nearest_open_space({choice, 0});
			if (to == (dot){-1, -1})
			{
				solve_for_worstcase();
				return;
			}
			else
			{
				carry({now[0].x, now[0].y}, to, move_of_zero);
				place[num] = {to.x, to.y};
				exist[to.x][to.y] = num;
				remain++;
			}
			int nxt = A[choice][queueIDX[choice]];
			place[nxt] = {choice, 0};
			exist[choice][0] = nxt;
			queueIDX[choice]++;
		}
		S[0] += move_of_zero;
		S[4] += string(move_of_zero.size(), '.');
		/* ここに小アーム4の動きを入れる
		 *  - 大アーム0に当たらないようにしつつコンテを右に動かす
		 *  - carryのstartとtoを決め、S[4]に追加、S[0]と被るならS[4]を止める
		 */
	}

	int siz = 0;
	for (int i = 0; i < N; i++)
		siz = max(siz, (int)S[i].size());
	if (siz < BEST_SIZE)
	{
		BEST_SIZE = siz;
		for (int i = 0; i < N; i++)
			ans[i] = S[i];
	}
	return;
}

void initialize()
{
	for (int i = 0; i < N; i++)
		S[i] = "";
	for (int i = 0; i < N * N; i++)
		place[i] = {INQUEUE, INQUEUE};
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N - 1; j++)
			exist[i][j] = -1;
	for (int i = 0; i < N; i++)
		exist[i][N - 1] = IMPOSSIBLE;
	for (int i = 0; i < N; i++)
		required[i] = 0, queueIDX[i] = 4;
	return;
}

string move(dot start, dot to)
{
	// cerr << "MOVE " << start << " TO " << to << endl;
	if (to.x == -1 || to.y == -1)
	{
		for (int i = 0; i < N; i++)
			cout << S[i] << endl;
		cerr << "THERE IS NO SPACE TO MOVE." << endl;
	}
	string ret;
	if (to.x <= start.x && to.y >= start.y)
	{
		ret += string(to.y - start.y, 'R');
		ret += string(start.x - to.x, 'U');
	}
	else if (to.x <= start.x && to.y <= start.y)
	{
		ret += string(start.y - to.y, 'L');
		ret += string(start.x - to.x, 'U');
	}
	else if (to.x >= start.x && to.y >= start.y)
	{
		ret += string(to.x - start.x, 'D');
		ret += string(to.y - start.y, 'R');
	}
	else if (to.x >= start.x && to.y <= start.y)
	{
		ret += string(to.x - start.x, 'D');
		ret += string(start.y - to.y, 'L');
	}
	now[0].x = to.x, now[0].y = to.y;
	return ret;
}

void carry(dot start, dot to, string &S)
{
	assert(start.x == now[0].x && start.y == now[0].y);
	S += 'P';
	S += move(start, to);
	S += 'Q';
	return;
}

const int dx[4] = {0, 1, -1, 0}, dy[4] = {1, 0, 0, -1};

dot find_nearest_movable_container(dot start)
{
	vector<vector<bool>> visited(N, vector<bool>(N, false));
	queue<dot> Q;
	Q.push(start);
	while (!Q.empty())
	{
		auto [x, y] = Q.front();
		Q.pop();
		visited[x][y] = true;
		for (int k = 0; k < 4; k++)
		{
			int nx = x + dx[k], ny = y + dy[k];
			if (!(0 <= nx && nx < N && 0 <= ny && ny < N) || visited[nx][ny])
				continue;
			Q.push({nx, ny});
			if (exist[nx][ny] != -1 && exist[nx][ny] != IMPOSSIBLE)
			{
				int num = exist[nx][ny];
				if (required[num / N] == num % N)
					return {nx, ny};
			}
		}
	}
	return {-1, -1};
}

dot find_nearest_open_space(dot start)
{
	vector<vector<bool>> visited(N, vector<bool>(N, false));
	queue<dot> Q;
	Q.push(start);
	visited[start.x][start.y] = true;
	while (!Q.empty())
	{
		auto [x, y] = Q.front();
		Q.pop();
		visited[x][y] = true;
		for (int k = 0; k < 4; k++)
		{
			int nx = x + dx[k], ny = y + dy[k];
			if (!(0 <= nx && nx < N && 0 <= ny && ny < N))
				continue;
			if (visited[nx][ny])
				continue;
			Q.push({nx, ny});
			if (exist[nx][ny] == -1)
				return {nx, ny};
		}
	}
	return {-1, -1};
}

void solve_for_worstcase()
{ // requiredが求めるもののうち、すでにqueueから出たものか、まだqueueに入っているもののうち一番近いものを運ぶ
	cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
	initialize();
	for (int i = 0; i < N; i++)
	{
		queueIDX[i] = 1;
		int num = A[i][0];
		place[num] = {i, 0};
		exist[i][0] = num;
	}
	now[0].x = 0, now[0].y = 0;
	S[1] = S[2] = S[3] = S[4] = "B";
	int remain = N * N;
	while (remain--)
	{
		int best = -1, MIN = 10;
		for (int i = 0; i < N; i++)
		{
			if (required[i] == N)
				continue;
			int num = i * N + required[i];
			if (place[num] != (dot){INQUEUE, INQUEUE})
			{
				best = i;
				break;
			}
			int x, y;
			for (int i = 0; i < N; i++)
				for (int j = 0; j < N; j++)
					if (A[i][j] == num)
						x = i, y = j;
			int need = y - queueIDX[x] + 1;
			if (need < MIN)
				MIN = need, best = i;
		}
		int num = best * N + required[best];

		if (place[num] == (dot){INQUEUE, INQUEUE})
		{
			int x, y;
			for (int i = 0; i < N; i++)
				for (int j = 0; j < N; j++)
					if (A[i][j] == num)
						x = i, y = j;
			while (queueIDX[x] <= y)
			{
				int start = exist[x][0], nxt = A[x][queueIDX[x]];
				S[0] += move({now[0].x, now[0].y}, {x, 0});
				dot to = find_nearest_open_space({x, 0});
				carry({now[0].x, now[0].y}, to, S[0]);
				place[start] = {to.x, to.y};
				exist[to.x][to.y] = start;
				place[nxt] = {x, 0};
				exist[x][0] = nxt;
				queueIDX[x]++;
			}
		}
		S[0] += move({now[0].x, now[0].y}, place[num]);
		carry({now[0].x, now[0].y}, {num / N, 4}, S[0]);
		exist[place[num].x][place[num].y] = -1;
		if (place[num].y == 0 && queueIDX[place[num].x] != N)
		{
			int nxt = A[place[num].x][queueIDX[place[num].x]];
			place[nxt] = {place[num].x, 0};
			exist[place[num].x][0] = nxt;
			queueIDX[place[num].x]++;
		}
		place[num] = {END, END};
		required[best]++;
	}
	for (int i = 0; i < N; i++)
		ans[i] = S[i];
	return;
}
