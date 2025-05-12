#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")
#include <iostream>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <random>
using namespace std;
#define debug(x) cerr << #x << " = " << (x) << endl

struct Timer
{
	Timer(int timeLimit) : timeLimit(timeLimit) { start = chrono::steady_clock::now(); }

	inline void reset() { start = chrono::steady_clock::now(); }
	inline bool isTimeOver() { return chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start).count() >= timeLimit; }
	inline double getTime() { return chrono::duration<double>(chrono::steady_clock::now() - start).count() * 1000; }

private:
	chrono::steady_clock::time_point start;
	int timeLimit;
};

struct Random
{
	Random(int seed) { init(seed); }

	inline int nextInt() { return rand(); }
	inline int nextInt(int n) { return rand() % n; }
	inline int nextInt(int l, int r) { return l + rand() % (r - l); }
	inline double nextDouble() { return (rand() + 0.5) / 4294967296.0; }
	inline double nextDouble(double r) { return r * nextDouble(); }
	inline double nextDouble(double l, double r) { return l + (r - l) * nextDouble(); }

private:
	unsigned int MT[624];
	int index;
	void init(int seed)
	{
		MT[0] = seed;
		for (int i = 1; i < 624; i++)
			MT[i] = 1812433253UL * (MT[i - 1] ^ (MT[i - 1] >> 30)) + i;
		index = 0;
	}
	void gen()
	{
		const unsigned int mask[2] = {0x0UL, 0x9908b0dfUL};
		for (int i = 0; i < 227; i++)
		{
			unsigned int y = (MT[i] & 0x80000000UL) + (MT[i + 1] & 0x7fffffffUL);
			MT[i] = MT[i + 397] ^ (y >> 1);
			MT[i] ^= mask[y & 1];
		}
		for (int i = 227; i < 623; i++)
		{
			unsigned int y = (MT[i] & 0x80000000UL) + (MT[i + 1] & 0x7fffffffUL);
			MT[i] = MT[i - 227] ^ (y >> 1);
			MT[i] ^= mask[y & 1];
		}
		unsigned int y = (MT[623] & 0x80000000UL) + (MT[0] & 0x7fffffffUL);
		MT[623] = MT[396] ^ (y >> 1);
		MT[623] ^= mask[y & 1];
	}
	unsigned int rand()
	{
		if (index == 0)
			gen();
		unsigned int y = MT[index];
		y ^= y >> 11;
		y ^= (y << 7) & 2636928640UL;
		y ^= (y << 15) & 4022730752UL;
		y ^= y >> 18;
		index = index == 623 ? 0 : index + 1;
		return y;
	}
};

struct Solver
{
	Solver() : rnd(Random(0)), timer(Timer(1950))
	{
		input();
		ans.resize(N);
		best_score = -1;
		sum.resize(N + 1);
		for (int i = 0; i < N; i++)
			sum[i + 1] = sum[i] + sqrt(T[i]);
		rough_cnt.resize(N);
		init_ans();
	}

	void solve()
	{
		double first_temp = 0, last_temp = 0;
		int iter = 0;
		while (true)
		{
			iter++;
			if (timer.isTimeOver())
				break;
			// double temp = first_temp * pow(last_temp / first_temp, timer.getTime() / 2000.0);
			double temp = first_temp + (last_temp - first_temp) * timer.getTime() / 2000;
			simulated_annealing(temp);
		}
		return;
	}

	void output()
	{
#ifdef LOCAL
		cerr << "SCORE: " << calc_score(ans) << '\n';
#endif
		for (auto [a, b] : ans)
			cout
				<< a << ' ' << b << '\n';
		return;
	}

private:
	Random rnd;
	Timer timer;

	const int N = 100, L = 500'000;
	vector<int> T;

	vector<int> sum;
	vector<int> rough_cnt;

	vector<pair<int, int>> ans;
	int best_score;

	void input()
	{
		int _;
		cin >> _ >> _;
		T.resize(N);
		for (int i = 0; i < N; i++)
			cin >> T[i];
		return;
	}

	// !! O(L/300)
	int roughly_calc_socre(const vector<pair<int, int>> &table)
	{
		vector<int> cnt(N, 0);
		vector<bool> use_a(N, true);
		int now = 0;
		for (int i = 0; i < L / 300; i++)
		{
			cnt[now]++;
			use_a[now] = !use_a[now];
			now = use_a[now] ? table[now].first : table[now].second;
		}
		int score = 0;
		for (int i = 0; i < N; i++)
		{
			score += abs(cnt[i] * 300 - T[i]);
			rough_cnt[i] = cnt[i] * 300;
		}
		return 1'000'000 - score;
	}

	// !! O(L)
	int calc_score(const vector<pair<int, int>> &table)
	{
		vector<int> cnt(N, 0);
		vector<bool> use_a(N, true);
		int now = 0;
		for (int i = 0; i < L; i++)
		{
			cnt[now]++;
			use_a[now] = !use_a[now];
			now = use_a[now] ? table[now].first : table[now].second;
		}
		int score = 0;
		for (int i = 0; i < N; i++)
			score += abs(cnt[i] - T[i]);
		return 1'000'000 - score;
	}

	void recalc_sum()
	{
		sum[0] = 0;
		for (int i = 0; i < N; i++)
			sum[i + 1] = sum[i] + abs(T[i] - rough_cnt[i]);
		return;
	}

	int rand_choice()
	{
		int val = rnd.nextInt(sum[N]);
		return upper_bound(sum.begin(), sum.end(), val) - sum.begin() - 1;
	}

	void init_ans()
	{
		vector<int> idx(N);
		iota(idx.begin(), idx.end(), 0);
		sort(idx.begin(), idx.end(), [&](int i, int j)
			 { return T[i] > T[j]; });
		vector<int> p(N);
		for (int i = 0; i < N / 2; i++)
			p[i] = i + 1;
		p[N / 2] = 0;
		for (int i = N / 2 + 1; i < N; i++)
			p[i] = i;
		vector<int> p_inv(N);
		for (int i = 0; i < N; i++)
			p_inv[p[i]] = i;
		vector<pair<int, int>> G(N);
		{
			G[0] = {1, 1};
			for (int i = 1; i < N; i++)
				G[i] = {(i + 1) % N, i - 1};
		}
		for (int i = 0; i < N; i++)
		{
			auto [u, v] = G[i];
			ans[idx[p_inv[i]]] = {idx[p_inv[u]], idx[p_inv[v]]};
		}
		best_score = calc_score(ans);
		return;
	}

	void simulated_annealing(double temp)
	{
		int choice = rnd.nextInt(4);
		vector<pair<int, int>> new_ans = ans;
		int idx = rnd.nextInt(N);
		if (choice <= 1)
		{
			int ab = rnd.nextInt(2);
			if (ab)
				new_ans[idx].first = rand_choice();
			else
				new_ans[idx].second = rand_choice();
		}
		else if (choice == 2)
		{
			int idx2 = rnd.nextInt(N);
			swap(new_ans[idx], new_ans[idx2]);
		}
		else
		{
			swap(new_ans[idx].first, new_ans[idx].second);
		}
		int new_score = calc_score(new_ans);
		if (new_score > best_score)
		{
			recalc_sum();
			best_score = new_score;
			ans = new_ans;
		}
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