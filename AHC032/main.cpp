#include <iostream>
#include <cassert>
#include <vector>

#include <atcoder/modint>
using mint = atcoder::modint998244353;

struct Index
{
	Index() : x(0), y(0) {}
	Index(int x, int y) : x(x), y(y) { assert(0 <= x && x < 9 && 0 <= y && y < 9); }
	Index(const Index &index) : x(index.x), y(index.y) {}

	Index operator+(const Index &other) const
	{
		return Index(x + other.x, y + other.y);
	}

	int x, y;
};

struct Stamp
{
	Stamp() : stamp(3, std::vector<int>(3, 0)) {}
	Stamp(std::vector<std::vector<int>> stamp) : stamp(stamp) { assert(stamp.size() == 3 && stamp[0].size() == 3); }

	inline int get(Index idx) const
	{
		assert(0 <= idx.x && idx.x < 3 && 0 <= idx.y && idx.y < 3);
		return stamp[idx.x][idx.y];
	}
	inline void set(Index idx, int val)
	{
		assert(0 <= idx.x && idx.x < 3 && 0 <= idx.y && idx.y < 3);
		stamp[idx.x][idx.y] = val;
	}

private:
	std::vector<std::vector<int>> stamp;
};
std::istream &operator>>(std::istream &is, Stamp &stamp)
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
		{
			int val;
			is >> val;
			stamp.set(Index(i, j), val);
		}
	return is;
}

struct Field
{
	Field() : field(9, std::vector<mint>(9, 0)) {}

	inline mint get(Index idx) const
	{
		assert(0 <= idx.x && idx.x < 9 && 0 <= idx.y && idx.y < 9);
		return field[idx.x][idx.y];
	}
	inline void set(Index idx, int val)
	{
		assert(0 <= idx.x && idx.x < 9 && 0 <= idx.y && idx.y < 9);
		field[idx.x][idx.y] = val;
	}

	inline long long get_sum() const
	{
		long long sum = 0;
		for (int i = 0; i < 9; i++)
			for (int j = 0; j < 9; j++)
				sum += field[i][j].val();
		return sum;
	}

	inline void put_stamp(Index &idx, const Stamp &stamp)
	{
		assert(0 <= idx.x && idx.x <= 7 && 0 <= idx.y && idx.y <= 7);
		for (int x = 0; x < 3; x++)
			for (int y = 0; y < 3; y++)
			{
				Index new_idx(idx.x + x, idx.y + y);
				field[new_idx.x][new_idx.y] += stamp.get(Index(x, y));
			}
	}

private:
	std::vector<std::vector<mint>> field;
};
std::istream &operator>>(std::istream &is, Field &field)
{
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
		{
			int val;
			is >> val;
			field.set(Index(i, j), val);
		}
	return is;
}

struct Solver
{
	Solver() { input(); }

	void solve()
	{
		auto find_best_stamps = [&](Index &point, std::vector<Index> &target) -> std::vector<std::pair<int, Index>>
		{
			int size = target.size();
			long long max_val = -1;
			std::vector<int> max_stamps;

			auto dfs = [&](auto &self, int cnt, int start, std::vector<mint> &vals, std::vector<int> &stamp_idxs) -> void
			{
				long long sum = 0;
				for (int i = 0; i < size; i++)
					sum += vals[i].val();
				if (sum > max_val)
				{
					max_val = sum;
					max_stamps = stamp_idxs;
				}

				if (stamp_idxs.size() == target.size())
					return;

				for (int i = start; i < M; i++)
				{
					for (int j = 0; j < size; j++)
						vals[j] += stamps[i].get(target[j]);
					stamp_idxs.push_back(i);

					self(self, cnt + 1, i, vals, stamp_idxs);

					for (int j = 0; j < size; j++)
						vals[j] -= stamps[i].get(target[j]);
					stamp_idxs.pop_back();
				}
			};

			std::vector<mint> vals(target.size(), 0);
			for (int i = 0; i < (int)target.size(); i++)
				vals[i] = field.get(point + target[i]);
			std::vector<int> stamps;
			dfs(dfs, 0, 0, vals, stamps);

			std::vector<std::pair<int, Index>> best_stamps;
			for (int idx : max_stamps)
				best_stamps.push_back({idx, point});
			return best_stamps;
		};

		for (int i = 0; i < N - 2; i++)
			for (int j = 0; j < N - 2; j++)
			{
				Index point = Index(i, j);
				std::vector<Index> idxs;
				if (i + 3 == N && j + 3 == N)
					for (int x = 0; x < 3; x++)
						for (int y = 0; y < 3; y++)
							idxs.push_back(Index(x, y));
				else if (i + 3 == N)
					for (int x = 0; x < 3; x++)
						idxs.push_back(Index(x, 0));
				else if (j + 3 == N)
					for (int y = 0; y < 3; y++)
						idxs.push_back(Index(0, y));
				else
					idxs.push_back(Index(0, 0));

				auto best_stamps = find_best_stamps(point, idxs);
				for (auto &a : best_stamps)
				{
					ans.push_back(a);
					field.put_stamp(a.second, stamps[a.first]);
				}
			}
		return;
	}

	void output()
	{
		assert((int)ans.size() <= K);
		std::cout << ans.size() << "\n";
		for (auto &a : ans)
			std::cout << a.first << " " << a.second.x << " " << a.second.y << "\n";
		return;
	}

private:
	const int N = 9, M = 20, K = 81;
	Field field;
	std::vector<Stamp> stamps;

	std::vector<std::pair<int, Index>> ans;

	void input()
	{
		int _;
		std::cin >> _ >> _ >> _;
		std::cin >> field;
		for (int i = 0; i < M; i++)
		{
			Stamp stamp;
			std::cin >> stamp;
			stamps.push_back(stamp);
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
