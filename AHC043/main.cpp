#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")
#include <iostream>
#include <cassert>
#include <chrono>
#include <random>
#include <vector>
#include <unordered_set>
#include <queue>
using namespace std;
#include <atcoder/dsu>
#define debug(x) cerr << #x << " = " << (x) << endl

#define COST_STATION 5000
#define COST_RAIL 100

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

struct Point
{
	int x, y;
	Point() : x(-1), y(-1) {}
	Point(int x, int y) : x(x), y(y) {}

	inline bool operator==(const Point &p) const { return x == p.x && y == p.y; }
	inline bool operator!=(const Point &p) const { return x != p.x || y != p.y; }
	inline bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
	inline bool operator>(const Point &p) const { return x != p.x ? x > p.x : y > p.y; }
	inline bool operator<=(const Point &p) const { return x != p.x ? x < p.x : y <= p.y; }
	inline bool operator>=(const Point &p) const { return x != p.x ? x > p.x : y >= p.y; }

	inline Point operator+(const Point &p) const { return Point(x + p.x, y + p.y); }
	inline Point operator-(const Point &p) const { return Point(x - p.x, y - p.y); }
	inline Point operator+=(const Point &p) { return *this = *this + p; }
	inline Point operator-=(const Point &p) { return *this = *this - p; }

	inline int dist(const Point &p) const { return abs(x - p.x) + abs(y - p.y); }
};
istream &operator>>(istream &is, Point &p)
{
	is >> p.x >> p.y;
	return is;
}

const Point neighbors[] = {
	Point(-2, 0),
	Point(-1, -1),
	Point(-1, 0),
	Point(-1, 1),
	Point(0, -2),
	Point(0, -1),
	Point(0, 0),
	Point(0, 1),
	Point(0, 2),
	Point(1, -1),
	Point(1, 0),
	Point(1, 1),
	Point(2, 0),
};

const Point delta[] = {
	Point(0, 1),
	Point(1, 0),
	Point(0, -1),
	Point(-1, 0),
};

enum class Operation
{
	DO_NOTHING = -1,
	STATION = 0,
	RAIL_HORIZONTAL = 1,
	RAIL_VERTICAL = 2,
	RAIL_LEFT_DOWN = 3,
	RAIL_LEFT_UP = 4,
	RAIL_RIGHT_UP = 5,
	RAIL_RIGHT_DOWN = 6,
};

struct Action
{
	Action(Operation op) : op(op), p(Point()) { assert(op == Operation::DO_NOTHING); }
	Action(Operation op, Point p) : op(op), p(p) { assert(op != Operation::DO_NOTHING); }

	Operation op;
	Point p;
};
ostream &operator<<(ostream &os, const Action &act)
{
	if (act.op == Operation::DO_NOTHING)
		os << -1;
	else
		os << static_cast<int>(act.op) << " " << act.p.x << " " << act.p.y;
	return os;
}

enum class State
{
	EMPTY = -1,
	STATION = 0,
	RAIL_HORIZONTAL = 1,
	RAIL_VERTICAL = 2,
	RAIL_LEFT_DOWN = 3,
	RAIL_LEFT_UP = 4,
	RAIL_RIGHT_UP = 5,
	RAIL_RIGHT_DOWN = 6,
};

struct Field
{
	Field()
	{
		field.assign(N, vector<State>(N, State::EMPTY));
		uf = atcoder::dsu(N * N);
		ex_home.assign(N, vector<vector<int>>(N));
		ex_workspace.assign(N, vector<vector<int>>(N));
		ex_station.assign(N, vector<bool>(N, false));
		home_and_workspace_cnt.assign(N, vector<int>(N));
		cur_score.assign(N, vector<int>(N, 0));
		cur_unconnected_cnt.assign(N, vector<int>(N, 0));
	}
	Field(const vector<Point> &home, const vector<Point> &workspace) : Field()
	{
		assert(home.size() == workspace.size());
		this->home = home;
		this->workspace = workspace;
		for (int i = 0; i < (int)home.size(); i++)
		{
			ex_home[home[i].x][home[i].y].push_back(i);
			ex_workspace[workspace[i].x][workspace[i].y].push_back(i);
		}
		for (int i = 0; i < (int)home.size(); i++)
		{
			for (const Point &d : neighbors)
			{
				Point np = home[i] + d;
				if (is_inside(np))
					home_and_workspace_cnt[np.x][np.y]++;
				np = workspace[i] + d;
				if (is_inside(np))
					home_and_workspace_cnt[np.x][np.y]++;
			}
		}
		score.assign(home.size(), 0);
		for (int i = 0; i < (int)home.size(); i++)
			score[i] = home[i].dist(workspace[i]);
		cur_estimated_income.assign(N, vector<int>(N, 0));
		for (int i = 0; i < (int)home.size(); i++)
		{
			for (const Point &d : neighbors)
			{
				Point np = home[i] + d;
				if (is_inside(np))
					cur_estimated_income[np.x][np.y] += score[i];
				np = workspace[i] + d;
				if (is_inside(np))
					cur_estimated_income[np.x][np.y] += score[i];
			}
		}
	}

	inline Field &operator=(const Field &other)
	{
		if (this != &other)
		{
			this->field = other.field;
			this->uf = other.uf;
			this->home = other.home;
			this->workspace = other.workspace;
			this->stations = other.stations;
			this->ex_home = other.ex_home;
			this->ex_workspace = other.ex_workspace;
			this->ex_station = other.ex_station;
			this->home_and_workspace_cnt = other.home_and_workspace_cnt;
			this->score = other.score;
			this->near_station_unconnected_home_idx = other.near_station_unconnected_home_idx;
			this->near_station_unconnected_workspace_idx = other.near_station_unconnected_workspace_idx;
			this->connected_idx = other.connected_idx;
			this->cur_score = other.cur_score;
			this->cur_unconnected_cnt = other.cur_unconnected_cnt;
			this->cur_estimated_income = other.cur_estimated_income;
		}
		return *this;
	}

	inline State get_state(const Point &p) const { return field[p.x][p.y]; }

	inline Point get_home(int idx) const { return home[idx]; }
	inline Point get_workspace(int idx) const { return workspace[idx]; }

	inline vector<Point> get_stations() const { return stations; }

	inline vector<int> get_home_index(const Point &p) const { return ex_home[p.x][p.y]; }
	inline vector<int> get_workspace_index(const Point &p) const { return ex_workspace[p.x][p.y]; }
	inline bool exist_station(const Point &p) const { return ex_station[p.x][p.y]; }

	inline int get_neighbor_cnt(const Point &p) const { return home_and_workspace_cnt[p.x][p.y]; }

	inline int get_score(int idx) const { return score[idx]; }

	inline vector<int> get_connected_idx() const { return connected_idx; }

	inline int point_to_index(const Point &p) const { return p.x * N + p.y; }
	inline Point index_to_point(int idx) const { return Point(idx / N, idx % N); }

	inline bool exist_unconnected_home(int idx) const { return near_station_unconnected_home_idx.count(idx); }
	inline bool exist_unconnected_workspace(int idx) const { return near_station_unconnected_workspace_idx.count(idx); }

	inline int get_cur_score(const Point &p) const { return cur_score[p.x][p.y]; }
	inline int get_cur_unconnected_cnt(const Point &p) const { return cur_unconnected_cnt[p.x][p.y]; }
	inline int get_cur_estimated_income(const Point &p) const { return cur_estimated_income[p.x][p.y]; }

	inline bool is_inside(const Point &p) const { return 0 <= p.x && p.x < N && 0 <= p.y && p.y < N; }

	inline void build(const State &state, const Point &p)
	{
		assert(is_inside(p));
		assert(field[p.x][p.y] != State::STATION);
		if (1 <= static_cast<int>(state) && static_cast<int>(state) <= 6)
			assert(field[p.x][p.y] == State::EMPTY);
		field[p.x][p.y] = state;

		if (state == State::STATION)
			stations.push_back(p), ex_station[p.x][p.y] = true;

		// Connect to the adjacent cells
		// up
		if (p.x > 0 && exist_up(state))
			if (exist_down(field[p.x - 1][p.y]))
				uf.merge(point_to_index(p), point_to_index(Point(p.x - 1, p.y)));
		// down
		if (p.x < N - 1 && exist_down(state))
			if (exist_up(field[p.x + 1][p.y]))
				uf.merge(point_to_index(p), point_to_index(Point(p.x + 1, p.y)));
		// left
		if (p.y > 0 && exist_left(state))
			if (exist_right(field[p.x][p.y - 1]))
				uf.merge(point_to_index(p), point_to_index(Point(p.x, p.y - 1)));
		// right
		if (p.y < N - 1 && exist_right(state))
			if (exist_left(field[p.x][p.y + 1]))
				uf.merge(point_to_index(p), point_to_index(Point(p.x, p.y + 1)));

		return;
	}

	inline void update_corresponce(const Point &station)
	{
		for (const Point &d : neighbors)
		{
			Point np = station + d;
			if (!is_inside(np))
				continue;
			vector<pair<int, Point>> dels, adds, alls;
			for (int idx : ex_home[np.x][np.y])
			{
				if (near_station_unconnected_workspace_idx.count(idx))
				{
					connected_idx.push_back(idx);
					near_station_unconnected_workspace_idx.erase(idx);
					dels.push_back(make_pair(get_score(idx), get_home(idx)));
				}
				else
				{
					near_station_unconnected_home_idx.insert(idx);
					adds.push_back(make_pair(get_score(idx), get_workspace(idx)));
				}
				alls.push_back(make_pair(get_score(idx), get_home(idx)));
			}
			for (int idx : ex_workspace[np.x][np.y])
			{
				if (near_station_unconnected_home_idx.count(idx))
				{
					connected_idx.push_back(idx);
					near_station_unconnected_home_idx.erase(idx);
					dels.push_back(make_pair(get_score(idx), get_workspace(idx)));
				}
				else
				{
					near_station_unconnected_workspace_idx.insert(idx);
					adds.push_back(make_pair(get_score(idx), get_home(idx)));
				}
				alls.push_back(make_pair(get_score(idx), get_workspace(idx)));
			}
			for (auto [score, p] : dels)
			{
				for (const Point &d2 : neighbors)
				{
					Point nnp = p + d2;
					if (is_inside(nnp))
					{
						cur_score[nnp.x][nnp.y] -= score;
						cur_unconnected_cnt[nnp.x][nnp.y]--;
					}
				}
			}
			for (auto [score, p] : adds)
			{
				for (const Point &d2 : neighbors)
				{
					Point nnp = p + d2;
					if (is_inside(nnp))
					{
						cur_score[nnp.x][nnp.y] += score;
						cur_unconnected_cnt[nnp.x][nnp.y]++;
					}
				}
			}
			for (auto [score, p] : alls)
			{
				for (const Point &d2 : neighbors)
				{
					Point nnp = p + d2;
					if (is_inside(nnp))
						cur_estimated_income[nnp.x][nnp.y] -= score;
				}
			}
			int hw_cnt = ex_home[np.x][np.y].size() + ex_workspace[np.x][np.y].size();
			for (const Point &d2 : neighbors)
			{
				Point nnp = np + d2;
				if (is_inside(nnp))
					home_and_workspace_cnt[nnp.x][nnp.y] -= hw_cnt;
			}
			ex_home[np.x][np.y].clear();
			ex_workspace[np.x][np.y].clear();
		}
	}

	inline bool is_connected(const Point &p1, const Point &p2)
	{
		vector<Point> stations1 = collect_stations(p1);
		vector<Point> stations2 = collect_stations(p2);
		for (Point &station1 : stations1)
			for (Point &station2 : stations2)
				if (uf.same(point_to_index(station1), point_to_index(station2)))
					return true;
		return false;
	}

private:
	const int N = 50;

	vector<vector<State>> field;
	atcoder::dsu uf;

	vector<Point> home, workspace, stations;
	vector<vector<vector<int>>> ex_home, ex_workspace;
	vector<vector<bool>> ex_station;
	vector<int> score;
	vector<vector<int>> home_and_workspace_cnt;
	vector<vector<int>> cur_score, cur_unconnected_cnt, cur_estimated_income;

	unordered_set<int> near_station_unconnected_home_idx, near_station_unconnected_workspace_idx;
	vector<int> connected_idx;

	inline bool exist_up(const State &s) const { return s == State::STATION || s == State::RAIL_VERTICAL || s == State::RAIL_LEFT_UP || s == State::RAIL_RIGHT_UP; }
	inline bool exist_down(const State &s) const { return s == State::STATION || s == State::RAIL_VERTICAL || s == State::RAIL_LEFT_DOWN || s == State::RAIL_RIGHT_DOWN; }
	inline bool exist_left(const State &s) const { return s == State::STATION || s == State::RAIL_HORIZONTAL || s == State::RAIL_LEFT_UP || s == State::RAIL_LEFT_DOWN; }
	inline bool exist_right(const State &s) const { return s == State::STATION || s == State::RAIL_HORIZONTAL || s == State::RAIL_RIGHT_UP || s == State::RAIL_RIGHT_DOWN; }

	inline vector<Point> collect_stations(const Point &p) const
	{
		vector<Point> stations;
		for (const Point &d : neighbors)
		{
			Point np = p + d;
			if (is_inside(np) && field[np.x][np.y] == State::STATION)
				stations.push_back(np);
		}
		return stations;
	}
};

struct Components
{
	Field field;
	int money, income;
	vector<Action> ans;
	vector<int> money_debug, income_debug;

	Components() : field(Field()), money(0), income(0) {}
	Components(const Field &field, int money, int income) : field(field), money(money), income(income) {}

	inline Components &operator=(const Components &other)
	{
		if (this != &other)
		{
			this->field = other.field;
			this->money = other.money;
			this->income = other.income;
			this->ans = other.ans;
			this->money_debug = other.money_debug;
			this->income_debug = other.income_debug;
		}
		return *this;
	}
};

struct Solver
{
	Solver() : rnd(Random(0)), timer(Timer(2900))
	{
		input();
		base_components.money = K;
		base_components.income = 0;
	}

	inline void solve()
	{
		build_first_stations();
		// while ((int)base_components.ans.size() < T)
		// 	build_nothing(base_components);
		// best_ans = base_components.ans;
		// best_money_debug = base_components.money_debug;
		// best_income_debug = base_components.income_debug;
		// return;
		int iter = 0;

		while (true)
		{
			if (timer.isTimeOver())
				break;
			random_fill();
			iter++;
		}
		debug(iter);

		brush_up();
		return;
	}

	inline void output()
	{
		assert((int)best_ans.size() == T);
		for (int i = 0; i < T; i++)
		{
#ifdef LOCAL
			cout << "# " << best_money_debug[i] << " " << best_income_debug[i] << endl;
#endif
			cout << best_ans[i] << endl;
		}
		cerr << "Score: " << best_money_debug[T - 1] << endl;
		return;
	}

private:
	Random rnd;
	Timer timer;

	const int N = 50, T = 800;
	int M, K;

	vector<Point> _heme, _workspace;

	Components base_components, temp_components;

	vector<Action> best_ans;
	vector<int> best_money_debug, best_income_debug;

	inline void input()
	{
		int _;
		cin >> _ >> M >> K >> _;
		_heme.resize(M), _workspace.resize(M);
		for (int i = 0; i < M; i++)
			cin >> _heme[i] >> _workspace[i];
		base_components.field = Field(_heme, _workspace);
		return;
	}

	inline void build_station(const Point &p, Components &components, bool recalc_income = false)
	{
		components.field.build(State::STATION, p);
		components.field.update_corresponce(p);
		components.ans.push_back(Action(Operation::STATION, p));
		components.money -= COST_STATION;
		if (recalc_income)
			components.income = calc_income(components);
		components.money += components.income;
		components.money_debug.push_back(components.money);
		components.income_debug.push_back(components.income);
		return;
	}

	inline void build_rail(const Point &p, Operation op, Components &components, bool recalc_income = false)
	{
		components.field.build(static_cast<State>(op), p);
		components.ans.push_back(Action(op, p));
		components.money -= COST_RAIL;
		if (recalc_income)
			components.income = calc_income(components);
		components.money += components.income;
		components.money_debug.push_back(components.money);
		components.income_debug.push_back(components.income);
		return;
	}

	inline void build_nothing(Components &components)
	{
		components.ans.push_back(Action(Operation::DO_NOTHING));
		components.money += components.income;
		components.money_debug.push_back(components.money);
		components.income_debug.push_back(components.income);
		return;
	}

	inline int calc_income(Components &components)
	{
		int res = 0;
		vector<int> connected_idx = components.field.get_connected_idx();
		for (int idx : connected_idx)
			res += components.field.get_score(idx);
		return res;
	}

	inline vector<Point> get_shortest_path(const Point &p1, const vector<Point> &goals, Components &components)
	{
		vector dist(N, vector<int>(N, 1e9)), score(N, vector<int>(N, 0));
		vector prev(N, vector<Point>(N, Point(-1, -1)));
		queue<Point> Q;
		Point goal = Point(-1, -1);

		dist[p1.x][p1.y] = 0;
		score[p1.x][p1.y] = components.field.get_home_index(p1).size() + components.field.get_workspace_index(p1).size();
		Q.push(p1);

		while (!Q.empty())
		{
			Point now = Q.front();
			Q.pop();
			if (find(goals.begin(), goals.end(), now) != goals.end())
			{
				goal = now;
				break;
			}
			for (const Point &d : delta)
			{
				Point next = now + d;
				if (!components.field.is_inside(next))
					continue;
				if (components.field.get_state(next) != State::EMPTY && components.field.get_state(next) != State::STATION)
					continue;

				int cost = dist[now.x][now.y] + 1;
				int next_score = score[now.x][now.y] + components.field.get_home_index(next).size() + components.field.get_workspace_index(next).size();
				if (dist[next.x][next.y] > cost)
				{
					dist[next.x][next.y] = cost;
					score[next.x][next.y] = next_score;
					prev[next.x][next.y] = now;
					Q.push(next);
				}
				else if (dist[next.x][next.y] == cost && score[next.x][next.y] < next_score)
				{
					score[next.x][next.y] = next_score;
					prev[next.x][next.y] = now;
				}
			}
		}
		if (goal == Point(-1, -1))
			return {};
		vector<Point> path;
		path.push_back(goal);
		Point cur = goal;
		while (cur != p1)
		{
			cur = prev[cur.x][cur.y];
			path.push_back(cur);
		}
		reverse(path.begin(), path.end());
		return path;
	}

	inline vector<Point> get_shortest_path_by_station(const Point &p1, const vector<Point> &goals, Components &components, const vector<vector<bool>> &sts)
	{
		vector dist(N, vector<int>(N, 1e9)), score(N, vector<int>(N, 0));
		vector prev(N, vector<Point>(N, Point(-1, -1)));
		queue<Point> Q;
		Point goal = Point(-1, -1);

		dist[p1.x][p1.y] = 0;
		score[p1.x][p1.y] = components.field.get_home_index(p1).size() + components.field.get_workspace_index(p1).size();
		Q.push(p1);

		while (!Q.empty())
		{
			Point now = Q.front();
			Q.pop();
			if (find(goals.begin(), goals.end(), now) != goals.end())
			{
				goal = now;
				break;
			}
			for (const Point &d : delta)
			{
				Point next = now + d;
				if (!components.field.is_inside(next))
					continue;
				if (components.field.get_state(next) != State::EMPTY && components.field.get_state(next) != State::STATION)
					continue;

				int cost = dist[now.x][now.y] + 1;
				int next_score = score[now.x][now.y] + sts[next.x][next.y];
				if (dist[next.x][next.y] > cost)
				{
					dist[next.x][next.y] = cost;
					score[next.x][next.y] = next_score;
					prev[next.x][next.y] = now;
					Q.push(next);
				}
				else if (dist[next.x][next.y] == cost && score[next.x][next.y] < next_score)
				{
					score[next.x][next.y] = next_score;
					prev[next.x][next.y] = now;
				}
			}
		}
		if (goal == Point(-1, -1))
			return {};
		vector<Point> path;
		path.push_back(goal);
		Point cur = goal;
		while (cur != p1)
		{
			cur = prev[cur.x][cur.y];
			path.push_back(cur);
		}
		reverse(path.begin(), path.end());
		return path;
	}

	inline bool connect_by_rail(const Point &p1, const vector<Point> &goals, Components &components, bool allow_new_station = false, bool use_by_rail = false, const vector<vector<bool>> &sts = {})
	{
		vector<Point> path1, path2;
		bool accept_path2 = false;
		{ // Find the shortest path between p1 and goals by A* algorithm
			if (use_by_rail)
				path1 = get_shortest_path_by_station(p1, goals, components, sts);
			else
				path1 = get_shortest_path(p1, goals, components);
		}
		if (path1.size() == 0 || allow_new_station)
		{ // Filn the nearest rail from p1 and build additional station
			int min_dist = 1e9;
			Point nearest_rail = Point(-1, -1);
			for (int i = 0; i < N * N; i++)
			{
				Point p = components.field.index_to_point(i);
				if (components.field.get_state(p) == State::STATION || components.field.get_state(p) == State::EMPTY)
					continue;
				if (p1.dist(p) < min_dist)
				{
					min_dist = p1.dist(p);
					nearest_rail = p;
				}
			}
			assert(nearest_rail != Point(-1, -1));
			if (path1.size() == 0 || min_dist < 0)
			{
				accept_path2 = true;
				while (components.money < COST_STATION)
					build_nothing(components);
				build_station(nearest_rail, components, true);
				if (use_by_rail)
					path2 = get_shortest_path_by_station(p1, {nearest_rail}, components, sts);
				else
					path2 = get_shortest_path(p1, {nearest_rail}, components);
			}
		}
		vector<Point> path;
		path = accept_path2 ? path2 : path1;
		reverse(path.begin(), path.end());

		// Coneect by rail
		for (int i = 1; i + 1 < (int)path.size(); i++)
		{
			Point prev = path[i - 1], cur = path[i], next = path[i + 1];
			int mask = 0;
			// up
			Point up = cur + Point(-1, 0);
			if (up == prev || next == up)
				mask |= 1 << 0;
			// down
			Point down = cur + Point(1, 0);
			if (down == prev || next == down)
				mask |= 1 << 1;
			// left
			Point left = cur + Point(0, -1);
			if (left == prev || next == left)
				mask |= 1 << 2;
			// right
			Point right = cur + Point(0, 1);
			if (right == prev || next == right)
				mask |= 1 << 3;

			while (components.money < COST_RAIL)
				build_nothing(components);

			if (mask == 0b0011)
				build_rail(cur, Operation::RAIL_VERTICAL, components, i + 1 == (int)path.size() - 1);
			else if (mask == 0b0101)
				build_rail(cur, Operation::RAIL_LEFT_UP, components, i + 1 == (int)path.size() - 1);
			else if (mask == 0b1001)
				build_rail(cur, Operation::RAIL_RIGHT_UP, components, i + 1 == (int)path.size() - 1);
			else if (mask == 0b0110)
				build_rail(cur, Operation::RAIL_LEFT_DOWN, components, i + 1 == (int)path.size() - 1);
			else if (mask == 0b1010)
				build_rail(cur, Operation::RAIL_RIGHT_DOWN, components, i + 1 == (int)path.size() - 1);
			else if (mask == 0b1100)
				build_rail(cur, Operation::RAIL_HORIZONTAL, components, i + 1 == (int)path.size() - 1);
			else
				assert(false);
		}
		return true;
	}

	inline void build_first_stations()
	{
		int rail_count = (K - 2 * COST_STATION) / COST_RAIL;
		Point p1 = Point(0, 0), p2 = Point(N - 1, N - 1);
		int best_score = -1, best_cnt = -1;
		vector<bool> homes(M, false), workspaces(M, false);
		for (int i = 0; i < N * N; i++)
		{
			Point q1 = base_components.field.index_to_point(i);
			if (base_components.field.get_neighbor_cnt(q1) == 0)
				continue;
			vector<int> h_idx, w_idx;
			for (const Point &d : neighbors)
			{
				Point q = q1 + d;
				if (base_components.field.is_inside(q))
				{
					vector<int> home_idx = base_components.field.get_home_index(q);
					vector<int> workspace_idx = base_components.field.get_workspace_index(q);
					for (int idx : home_idx)
					{
						homes[idx] = true;
						h_idx.push_back(idx);
					}
					for (int idx : workspace_idx)
					{
						workspaces[idx] = true;
						w_idx.push_back(idx);
					}
				}
			}
			for (int j = i + 1; j < N * N; j++)
			{
				Point q2 = base_components.field.index_to_point(j);
				if (q1.dist(q2) - 1 > rail_count || q1.dist(q2) < 4)
					continue;
				if (base_components.field.get_neighbor_cnt(q2) == 0)
					continue;
				int score = 0;
				int cnt = homes.size() + workspaces.size();
				for (const Point &d : neighbors)
				{
					Point nq2 = q2 + d;
					if (base_components.field.is_inside(nq2))
					{
						vector<int> home_idx = base_components.field.get_home_index(nq2);
						vector<int> workspace_idx = base_components.field.get_workspace_index(nq2);
						cnt += home_idx.size() + workspace_idx.size();
						for (int idx : home_idx)
							if (workspaces[idx])
								score += base_components.field.get_score(idx), cnt -= 2;
						for (int idx : workspace_idx)
							if (homes[idx])
								score += base_components.field.get_score(idx), cnt -= 2;
					}
				}
				if (make_pair(score, cnt) > make_pair(best_score, best_cnt))
				{
					best_score = score;
					best_cnt = cnt;
					p1 = q1;
					p2 = q2;
				}
			}
			for (int idx1 : h_idx)
				homes[idx1] = false;
			for (int idx2 : w_idx)
				workspaces[idx2] = false;
		}

		connect_by_rail(p1, {p2}, base_components);
		build_station(p1, base_components);
		build_station(p2, base_components, true);
		return;
	}

	inline bool build_additional_stations(Components &components, bool use_dist)
	{
		Point best_point, mid_point = Point(N / 2, N / 2);
		int best_score = -1, best_cnt = -1, best_dist = -1e9, best_estimated_income = -1;
		bool best_on_rail = false;
		bool must_use_rail = rnd.nextInt(100) < 20;
		bool best_income_point = rnd.nextInt(100) < 10 * (T - components.ans.size()) / T;
		if (!must_use_rail)
			for (int i = 0; i < N * N; i++)
			{
				if (rnd.nextInt(100) < 2)
					continue;
				Point p = components.field.index_to_point(i);
				if (components.field.get_neighbor_cnt(p) == 0)
					continue;
				if (components.field.get_state(p) == State::STATION)
					continue;
				int score = 0, cnt = 0, dist = -p.dist(mid_point);
				int estimate_income = 0;
				bool on_rail = components.field.get_state(p) != State::EMPTY && components.field.get_state(p) != State::STATION;
				vector<Point> cur_station = components.field.get_stations();
				if ((int)cur_station.size() < 10 && use_dist)
					for (const Point &q : cur_station)
						dist = max(dist, -p.dist(q));
				score = components.field.get_cur_score(p);
				cnt = components.field.get_neighbor_cnt(p) - components.field.get_cur_unconnected_cnt(p);
				estimate_income = components.field.get_cur_estimated_income(p);
				if (((T - (int)components.ans.size()) * estimate_income > 1.4 * COST_STATION) &&
					((best_income_point &&
					  make_tuple(estimate_income, score + 10 * on_rail + use_dist * dist / 2, cnt, on_rail, dist) > make_tuple(best_estimated_income, best_score, best_cnt, best_on_rail, best_dist)) ||
					 (!best_income_point &&
					  make_tuple(score + 10 * on_rail + use_dist * dist / 2, estimate_income, dist) > make_tuple(best_score, best_estimated_income, best_dist))))
				{
					best_score = score + 10 * on_rail + use_dist * dist / 2;
					best_estimated_income = estimate_income;
					best_cnt = cnt;
					best_on_rail = on_rail;
					best_dist = dist;
					best_point = p;
				}
			}
		if (best_score <= 0)
		{
			int siz = (int)components.ans.size();
			if (T / 8 < siz && siz < T / 2 && rnd.nextInt(100) < 0)
				return build_station_on_random_point(components);
			else
				return build_station_on_rail(components);
		}

		if (components.field.get_state(best_point) != State::EMPTY)
		{
			while (components.money < COST_STATION)
				build_nothing(components);
			build_station(best_point, components, true);
			return true;
		}
		bool success = connect_by_rail(best_point, components.field.get_stations(), components);
		if (!success)
			return false;
		while (components.money < COST_STATION)
			build_nothing(components);
		build_station(best_point, components, true);
		return true;
	}

	inline bool build_station_on_random_point(Components &components)
	{
		Point best_point, mid_point = Point(N / 2, N / 2);
		int best_score = -1, best_cnt = -1, best_dist = -1e9;
		bool best_on_rail = false;
		for (int i = 0; i < N * N; i++)
		{
			Point p = components.field.index_to_point(i);
			if (components.field.get_neighbor_cnt(p) == 0)
				continue;
			if (components.field.get_state(p) == State::STATION)
				continue;
			int score = 0, cnt = 0, dist = -p.dist(mid_point);
			bool on_rail = components.field.get_state(p) != State::EMPTY && components.field.get_state(p) != State::STATION;
			score = components.field.get_cur_estimated_income(p);
			cnt = components.field.get_neighbor_cnt(p) - components.field.get_cur_unconnected_cnt(p);
			if (make_tuple(score + 10 * on_rail + dist / 2, cnt, on_rail, dist) >
				make_tuple(best_score, best_cnt, best_on_rail, best_dist))
			{
				best_score = score + 10 * on_rail + dist / 2;
				best_cnt = cnt;
				best_on_rail = on_rail;
				best_dist = dist;
				best_point = p;
			}
		}
		if (best_score <= 0)
			return build_station_on_rail(components);

		if (components.field.get_state(best_point) != State::EMPTY)
		{
			while (components.money < COST_STATION)
				build_nothing(components);
			build_station(best_point, components, true);
			return true;
		}
		bool success = connect_by_rail(best_point, components.field.get_stations(), components);
		if (!success)
			return false;
		while (components.money < COST_STATION)
			build_nothing(components);
		build_station(best_point, components, true);
		return true;
	}

	inline bool build_station_on_rail(Components &components)
	{
		Point best_point;
		int best_cnt = -1, best_score = -1, best_estimated_income = -1;
		for (int i = 0; i < N * N; i++)
		{
			Point p = components.field.index_to_point(i);
			if (components.field.get_state(p) == State::EMPTY || components.field.get_state(p) == State::STATION)
				continue;
			if (components.field.get_neighbor_cnt(p) == 0)
				continue;
			int cnt = 0, score = 0, estimated_income = 0;
			for (const Point &d : neighbors)
			{
				Point np = p + d;
				if (!components.field.is_inside(np))
					continue;
				vector<int> home_idx = components.field.get_home_index(np);
				for (int idx : home_idx)
				{
					if (components.field.exist_unconnected_workspace(idx))
						score += components.field.get_score(idx);
					else
						score += components.field.get_score(idx) / 2;
					estimated_income += components.field.get_score(idx);
				}
				vector<int> workspace_idx = components.field.get_workspace_index(np);
				for (int idx : workspace_idx)
				{
					if (components.field.exist_unconnected_home(idx))
						score += components.field.get_score(idx);
					else
						score += components.field.get_score(idx) / 2;
					estimated_income += components.field.get_score(idx);
				}
				cnt += home_idx.size() + workspace_idx.size();
			}
			if (make_tuple(score, cnt, estimated_income) > make_tuple(best_score, best_cnt, best_estimated_income) &&
				(T - (int)components.ans.size()) * score > 1.1 * COST_STATION)
			{
				best_cnt = cnt;
				best_score = score;
				best_point = p;
				best_estimated_income = estimated_income;
			}
		}
		if (best_cnt <= 0)
			return false;
		while (components.money < COST_STATION)
			build_nothing(components);
		build_station(best_point, components);
		return true;
	}

	inline void random_fill()
	{
		temp_components = base_components;
		bool use_dist = rnd.nextInt(100) < 10;
		while (!timer.isTimeOver() && (int)temp_components.ans.size() < 800)
			if (!build_additional_stations(temp_components, use_dist))
				break;

		while ((int)temp_components.ans.size() < T)
			build_nothing(temp_components);

		if (best_ans.size() == 0 || temp_components.money > best_money_debug.back())
		{
			best_ans = temp_components.ans;
			best_money_debug = temp_components.money_debug;
			best_income_debug = temp_components.income_debug;
		}
		return;
	}

	inline void brush_up()
	{
		for (int i = 0; i < T; i++)
			for (int j = i + 1; j < T; j++)
			{
				if (best_ans[i].op == Operation::DO_NOTHING || best_ans[j].op == Operation::DO_NOTHING)
					continue;
				if (best_ans[i].p == best_ans[j].p)
				{
					bool ok = true;
					for (int k = i; k <= j; k++)
						if (best_money_debug[k] < COST_STATION + best_income_debug[k - 1])
							ok = false;
					if (!ok)
						continue;
					best_ans[i].op = Operation::STATION;
					for (int k = j; k + 1 < T; k++)
						best_ans[k] = best_ans[k + 1];
					best_ans[T - 1] = Action(Operation::DO_NOTHING);
					Components new_components = Components(Field(_heme, _workspace), K, 0);
					for (int i = 0; i < T; i++)
					{
						if (best_ans[i].op == Operation::DO_NOTHING)
							build_nothing(new_components);
						else if (best_ans[i].op == Operation::STATION)
							build_station(best_ans[i].p, new_components, true);
						else
							build_rail(best_ans[i].p, best_ans[i].op, new_components);
					}
					best_ans = new_components.ans;
					best_money_debug = new_components.money_debug;
					best_income_debug = new_components.income_debug;
				}
			}

		vector<Action> final_ans;
		vector<int> final_money_debug, final_income_debug;
		Components final_components = Components(Field(_heme, _workspace), K, 0);
		for (int i = 0; i < T; i++)
		{
			if (best_ans[i].op == Operation::DO_NOTHING)
				continue;
			if (best_ans[i].op == Operation::STATION)
			{
				while (final_components.money < COST_STATION)
					build_nothing(final_components);
				build_station(best_ans[i].p, final_components, true);
			}
			else
			{
				while (final_components.money < COST_RAIL)
					build_nothing(final_components);
				build_rail(best_ans[i].p, best_ans[i].op, final_components);
			}
		}
		while ((int)final_components.ans.size() < T)
			build_nothing(final_components);
		Components rebuild_components = Components(Field(_heme, _workspace), K, 0);
		int station_cnt = 0;
		vector<vector<bool>> sts(N, vector<bool>(N, false));
		for (int i = 0; i < T; i++)
			if (best_ans[i].op == Operation::STATION)
				sts[best_ans[i].p.x][best_ans[i].p.y] = true;
		for (int i = 0; i < T; i++)
		{
			if (station_cnt < 2)
			{
				if (best_ans[i].op == Operation::STATION)
				{
					station_cnt++;
					build_station(best_ans[i].p, rebuild_components, true);
				}
				else if (best_ans[i].op != Operation::DO_NOTHING)
					build_rail(best_ans[i].p, best_ans[i].op, rebuild_components);
				else
					build_nothing(rebuild_components);
			}
			else
			{
				if (best_ans[i].op != Operation::STATION)
					continue;
				Point p = best_ans[i].p;
				if (rebuild_components.field.get_state(p) != State::EMPTY)
				{
					while (rebuild_components.money < COST_STATION)
						build_nothing(rebuild_components);
					build_station(p, rebuild_components, true);
				}
				else
				{
					vector<Point> cur_stations = rebuild_components.field.get_stations();
					connect_by_rail(p, cur_stations, rebuild_components, false, true, sts);
					while (rebuild_components.money < COST_STATION)
						build_nothing(rebuild_components);
					build_station(p, rebuild_components, true);
				}
			}
		}
		while ((int)rebuild_components.ans.size() < 780)
			if (!build_additional_stations(rebuild_components, true))
				break;
		while ((int)rebuild_components.ans.size() < T)
			build_nothing(rebuild_components);
		if ((int)rebuild_components.ans.size() == T && rebuild_components.money > final_components.money)
			final_components = rebuild_components;
		best_ans = final_components.ans;
		best_money_debug = final_components.money_debug;
		best_income_debug = final_components.income_debug;
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