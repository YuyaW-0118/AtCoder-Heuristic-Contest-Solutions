#pragma GCC optimize("Ofast")
#pragma GCC target("avx2")
#include <bits/stdc++.h>
using namespace std;
#define debug(x) cerr << #x << " = " << (x) << endl

#ifdef ONLINE_JUDGE
#define DEBUG 0
#else
#define DEBUG 1
#endif

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

////////////////////////////////////////////////////////////////

// variables
const int N = 6, M = 15, T = 10, seeds = 2 * N * (N - 1);
vector<vector<int>> x;
#ifdef DEBUG
vector yoko(10, vector(6, vector<string>(5))), tate(10, vector(5, vector<string>(6)));
#endif

void input();
void solve();

int main()
{
	TimeKeeperDouble ALLTIME(10000);
	input();
	solve();
	ALLTIME.setNowTime();
	cerr << "Time : " << ALLTIME.getNowTime() << "ms" << endl;
	return 0;
}

void input()
{
	int _;
	cin >> _ >> _ >> _;
	x.resize(seeds, vector<int>(M));
	for (int i = 0; i < seeds; i++)
		for (int j = 0; j < M; j++)
			cin >> x[i][j];
	if (DEBUG)
	{
		for (int t = 0; t < T; t++)
		{
			for (int i = 0; i < 6; i++)
				for (int j = 0; j < 5; j++)
					cin >> yoko[t][i][j];
			for (int i = 0; i < 5; i++)
				for (int j = 0; j < 6; j++)
					cin >> tate[t][i][j];
		}
	}
	return;
}

vector<vector<int>> query(vector<vector<int>> &A, int t)
{
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			cout << A[i][j] << " \n"[j == N - 1];
	cout.flush();
	vector<vector<int>> res(seeds, vector<int>(M));
	if (DEBUG)
	{
		for (int i = 0; i < N; i++)
			for (int j = 0; j < N - 1; j++)
			{
				int num = i * (N - 1) + j;
				for (int s = 0; s < M; s++)
					res[num][s] = yoko[t][i][j][s] == '0' ? x[A[i][j]][s] : x[A[i][j + 1]][s];
			}
		for (int i = 0; i < N - 1; i++)
			for (int j = 0; j < N; j++)
			{
				int num = N * (N - 1) + i * N + j;
				for (int s = 0; s < M; s++)
					res[num][s] = tate[t][i][j][s] == '0' ? x[A[i][j]][s] : x[A[i + 1][j]][s];
			}
	}
	else
		for (int i = 0; i < seeds; i++)
			for (int j = 0; j < M; j++)
				cin >> res[i][j];
	return res;
}

const int wx[36] = {0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 5, 5, 5, 5, 5, 4, 3, 2, 1, 1, 1, 1, 1, 2, 3, 4, 4, 4, 4, 3, 2, 2, 2, 3, 3};
const int wy[36] = {0, 1, 2, 3, 4, 5, 5, 5, 5, 5, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 1, 2, 3, 4, 4, 4, 4, 3, 2, 1, 1, 1, 2, 3, 3, 2};
const int b = 8;

void solve()
{
	for (int t = 0; t < b; t++)
	{
		vector<vector<int>> A(N, vector<int>(N));
		vector<int> idx(seeds);
		iota(idx.begin(), idx.end(), 0);
		vector<int> MAX(M);
		for (int i = 0; i < seeds; i++)
			for (int j = 0; j < M; j++)
				MAX[j] = max(MAX[j], x[i][j]);
		sort(idx.begin(), idx.end(), [&](int i, int j)
			 {
			double cnt1 = 0, cnt2 = 0;
			for(int k = 0; k < 15; k++)
			{
				cnt1 += ((double)x[i][k]/MAX[k])*((double)x[i][k]/MAX[k])*((double)x[i][k]/MAX[k])*((double)x[i][k]/MAX[k])*((double)x[i][k]/MAX[k])*((double)x[i][k]/MAX[k])*((double)x[i][k]/MAX[k]);
				cnt2 += ((double)x[j][k]/MAX[k])*((double)x[j][k]/MAX[k])*((double)x[j][k]/MAX[k])*((double)x[j][k]/MAX[k])*((double)x[j][k]/MAX[k])*((double)x[j][k]/MAX[k])*((double)x[j][k]/MAX[k]);
			}
			return cnt1 > cnt2; });
		for (int i = 0; i < 36; i++)
			A[wx[i]][wy[i]] = idx[35 - i];
		vector<vector<int>> res = query(A, t);
		for (int i = 0; i < seeds; i++)
			for (int j = 0; j < M; j++)
				x[i][j] = res[i][j];
	}
	for (int t = b; t < T; t++)
	{
		vector<vector<int>> A(N, vector<int>(N));
		vector<int> idx(seeds);
		iota(idx.begin(), idx.end(), 0);
		sort(idx.begin(), idx.end(), [&](int i, int j)
			 { return accumulate(x[i].begin(), x[i].end(), 0) > accumulate(x[j].begin(), x[j].end(), 0); });
		for (int i = 0; i < 36; i++)
			A[wx[i]][wy[i]] = idx[35 - i];
		vector<vector<int>> res = query(A, t);
		for (int i = 0; i < seeds; i++)
			for (int j = 0; j < M; j++)
				x[i][j] = res[i][j];
	}
	return;
}