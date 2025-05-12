#include <iostream>
#include <cassert>
#include <chrono>
#include <random>
#include <algorithm>
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
	inline int randInt(int l, int r)
	{
		assert(r > l);
		return l + nextInt(r - l);
	}
	inline double nextDouble() { return dd_(mt_); }
	inline double nextLog() { return log(dd_(mt_)); }
};

struct Point
{
	int x, y;
	Point() : x(0), y(0) {}
	Point(int x, int y) : x(x), y(y) {}
	bool operator==(const Point &p) const { return x == p.x && y == p.y; }
	bool operator!=(const Point &p) const { return !(*this == p); }
	bool operator<(const Point &p) const { return x == p.x ? y < p.y : x < p.x; }
	bool operator>(const Point &p) const { return x == p.x ? y > p.y : x > p.x; }
	bool operator<=(const Point &p) const { return *this < p || *this == p; }
	bool operator>=(const Point &p) const { return *this > p || *this == p; }

	bool parallel(const Point &p) const { return *this != p && (x == p.x || y == p.y); }
};
istream &operator>>(istream &is, Point &p)
{
	is >> p.x >> p.y;
	return is;
}
ostream &operator<<(ostream &os, const Point &p)
{
	os << p.x << " " << p.y;
	return os;
}

struct Line
{
	Point p1, p2;
	Line(Point p1, Point p2) : p1(p1), p2(p2)
	{
		assert(p1.parallel(p2));
		if (p1 > p2)
			swap(this->p1, this->p2);
	}

	int len() const { return abs(p1.x - p2.x) + abs(p1.y - p2.y); }
	bool x_parallel() const { return p1.y == p2.y; }

	bool cross(const Line &l) const
	{
		if ((x_parallel() && l.x_parallel()) || (!x_parallel() && !l.x_parallel()))
			return max(p1.x, l.p1.x) < min(p2.x, l.p2.x) && max(p1.y, l.p1.y) < min(p2.y, l.p2.y);
		else
		{
			if (!x_parallel())
				return l.p1.x < p1.x && p1.x < l.p2.x && p1.y < l.p1.y && l.p1.y < p2.y;
			else
				return p1.x < l.p1.x && l.p1.x < p2.x && l.p1.y < p1.y && p1.y < l.p2.y;
		}
	}

	bool touch(const Line &l) const
	{
		if ((x_parallel() && l.x_parallel()) || (!x_parallel() && !l.x_parallel()))
			return max(p1.x, l.p1.x) <= min(p2.x, l.p2.x) && max(p1.y, l.p1.y) <= min(p2.y, l.p2.y);
		else
		{
			if (!x_parallel())
				return l.p1.x <= p1.x && p1.x <= l.p2.x && p1.y <= l.p1.y && l.p1.y <= p2.y;
			else
				return p1.x <= l.p1.x && l.p1.x <= p2.x && l.p1.y <= p1.y && p1.y <= l.p2.y;
		}
	}

	Line add_vertical(int l) const
	{
		if (x_parallel())
			return Line(Point(p1.x, p1.y + l), Point(p2.x, p2.y + l));
		else
			return Line(Point(p1.x + l, p1.y), Point(p2.x + l, p2.y));
	}
};

struct Solver
{
public:
	Solver() : rnd(Random(0)), timekeeper1(TimeKeeperDouble(450)), timekeeper2(TimeKeeperDouble(1990))
	{
		input();
	}

	void solve()
	{
		int max_x = 0, max_y = 0, min_x = 1e9, min_y = 1e9;
		for (int i = 0; i < N; i++)
		{
			max_x = max(max_x, ok[i].x);
			max_y = max(max_y, ok[i].y);
			min_x = min(min_x, ok[i].x);
			min_y = min(min_y, ok[i].y);
		}
		MAX = Point(max_x, max_y);
		MIN = Point(min_x, min_y);
		int best = -1;
		Point best_p1, best_p2;
		while (true)
		{
			timekeeper1.setNowTime();
			if (timekeeper1.isTimeOver())
				break;
			search_rectangle(best, best_p1, best_p2);
		}
		now.push_back(Line(best_p1, Point(best_p2.x, best_p1.y)));
		now.push_back(Line(Point(best_p2.x, best_p1.y), best_p2));
		now.push_back(Line(Point(best_p1.x, best_p2.y), best_p2));
		now.push_back(Line(best_p1, Point(best_p1.x, best_p2.y)));
		mmax = best_p2;
		mmin = best_p1;
		int cur_length = (best_p2.x - best_p1.x + best_p2.y - best_p1.y) * 2;
		while (true)
		{
			timekeeper2.setNowTime();
			if (timekeeper2.isTimeOver())
				break;
			int pre_size = now.size();
			hill_climbing(cur_length);
		}
		return;
	}

	void output()
	{
		for (int i = 0; i < now.size(); i++)
			for (int j = i + 2; j < now.size() - 1; j++)
				if (now[i].touch(now[j]))
				{
					cerr << "!" << endl;
					cout << 4 << endl;
					cout << mmin << endl;
					cout << mmax.x << " " << mmin.y << endl;
					cout << mmax << endl;
					cout << mmin.x << " " << mmax.y << endl;
					return;
				}
		cout << now.size() << endl;
		Line pre = now.back();
		for (auto l : now)
		{
			if (include(pre.p1, pre.p2, l.p1))
				cout << l.p1 << endl;
			else if (include(pre.p1, pre.p2, l.p2))
				cout << l.p2 << endl;
			else
				assert(false);
			pre = l;
		}
	}

private:
	Random rnd;
	TimeKeeperDouble timekeeper1, timekeeper2;

	const int N = 5000;
	vector<Point> ok, ng;

	vector<Line> ans, now;

	Point MIN, MAX, mmax, mmin;

	void input()
	{
		int _;
		cin >> _;
		ok.resize(N);
		ng.resize(N);
		for (int i = 0; i < N; i++)
			cin >> ok[i];
		for (int i = 0; i < N; i++)
			cin >> ng[i];
		return;
	}

	bool in_grid(Point p)
	{
		return 0 <= p.x && p.x <= 1e5 && 0 <= p.y && p.y <= 1e5;
	}

	void search_rectangle(int &best, Point &best_p1, Point &best_p2)
	{
		Point p1 = Point(rnd.randInt(MIN.x, MAX.x), rnd.randInt(MIN.y, MAX.y));
		Point p2 = Point(rnd.randInt(MIN.x, MAX.y), rnd.randInt(MIN.y, MAX.y));
		if (p2 < p1)
			swap(p1, p2);
		int cnt = 0;
		for (int i = 0; i < N; i++)
		{
			if (include(p1, p2, ok[i]))
				cnt++;
			if (include(p1, p2, ng[i]))
				cnt--;
		}
		if (cnt > best)
		{
			best = cnt;
			best_p1 = p1;
			best_p2 = p2;
		}
	}

	void hill_climbing(int &cur_length, bool d = false)
	{
		int idx = rnd.nextInt(now.size());
		int type = rnd.nextInt(2);
		if (d)
			type = 0;
		if (type == 0)
		{
			int pre_idx = (idx + now.size() - 1) % now.size();
			int nxt_idx = (idx + 1) % now.size();
			Point pre_far, nxt_far;
			if (include(now[idx].p1, now[idx].p2, now[pre_idx].p1))
				pre_far = now[pre_idx].p2;
			else if (include(now[idx].p1, now[idx].p2, now[pre_idx].p2))
				pre_far = now[pre_idx].p1;
			else
				assert(false);
			if (include(now[idx].p1, now[idx].p2, now[nxt_idx].p1))
				nxt_far = now[nxt_idx].p2;
			else if (include(now[idx].p1, now[idx].p2, now[nxt_idx].p2))
				nxt_far = now[nxt_idx].p1;
			else
				assert(false);

			bool up = upper(pre_far, now[idx]);
			if (up != upper(nxt_far, now[idx]))
				return;
			int can = min(dist(pre_far, now[idx]), dist(nxt_far, now[idx]));
			if (can / 2 <= 2)
				return;
			int r = rnd.randInt(1, can / 2);
			Line new_line = Line({0, 0}, {0, 1});
			if (dist(pre_far, now[idx]) < r || dist(nxt_far, now[idx]) < r)
				return;
			if (up)
				new_line = now[idx].add_vertical(r);
			else
				new_line = now[idx].add_vertical(-r);
			for (int i = 0; i < now.size(); i++)
			{
				if (i == idx || i == pre_idx || i == nxt_idx)
					continue;
				if (now[i].touch(new_line))
					return;
			}

			vector<Point> points = {new_line.p1, new_line.p2, now[idx].p1, now[idx].p2};
			sort(points.begin(), points.end());
			int cnt = 0;
			for (int i = 0; i < N; i++)
			{
				if (include(points[0], points[3], ok[i]))
					cnt++;
				if (include(points[0], points[3], ng[i]))
					cnt--;
			}
			if ((is_inside(mid(new_line)) && cnt <= 0) || (!is_inside(mid(new_line)) && cnt >= 0))
			{
				now[idx] = new_line;
				if (include(now[pre_idx].p1, now[pre_idx].p2, new_line.p1))
				{
					now[pre_idx] = Line(new_line.p1, pre_far);
					now[nxt_idx] = Line(nxt_far, new_line.p2);
				}
				else
				{
					now[pre_idx] = Line(new_line.p2, pre_far);
					now[nxt_idx] = Line(nxt_far, new_line.p1);
				}
#ifdef LOCAL
				output();
#endif
				cur_length -= abs(r) * 2;
			}
		}
		else
		{
			if (now[idx].len() == 1)
				return;
			int l = rnd.randInt(1, now[idx].len()), r = rnd.randInt(1, now[idx].len());
			if (l == r)
				return;
			if (l > r)
				swap(l, r);
			if ((r - l) * 4 < now[idx].len())
				return;
			if (r - l <= 2500)
				return;
			int width = rnd.randInt(-17000, 17000);
			if (width == 0)
				return;
			if (cur_length + abs(width) * 2 > 4e5)
				return;

			Line new_line = Line({0, 0}, {0, 1});
			if (now[idx].x_parallel())
				new_line = Line(Point(now[idx].p1.x + l, now[idx].p1.y), Point(now[idx].p1.x + r, now[idx].p2.y));
			else
				new_line = Line(Point(now[idx].p1.x, now[idx].p1.y + l), Point(now[idx].p2.x, now[idx].p1.y + r));

			Point p1 = new_line.p1, p2 = new_line.p2;
			Line new_line1 = new_line.add_vertical(width);
			vector<Point> points = {p1, p2, new_line1.p1, new_line1.p2};
			if (!in_grid(new_line1.p1) || !in_grid(new_line1.p2))
				return;
			Line new_line2 = Line(p1, new_line1.p1), new_line3 = Line(p2, new_line1.p2);
			int pre_idx = (idx + now.size() - 1) % now.size();
			int nxt_idx = (idx + 1) % now.size();
			for (int i = 0; i < now.size(); i++)
			{
				if (i == idx /*|| i == pre_idx || i == nxt_idx*/)
					continue;
				if (now[i].cross(new_line1) || now[i].cross(new_line2) || now[i].cross(new_line3))
					return;
			}
			sort(points.begin(), points.end());
			int cnt = 0;
			for (int i = 0; i < N; i++)
			{
				if (include(points[0], points[3], ok[i]))
					cnt++;
				if (include(points[0], points[3], ng[i]))
					cnt--;
			}
			if ((is_inside(new_line1.p1) && cnt < 0) || (!is_inside(new_line1.p1) && cnt > 0))
			{
				if (width > 5000 && abs(cnt) < 10)
					return;
				Line new_line4 = Line(now[idx].p1, p1), new_line5 = Line(p2, now[idx].p2);
				now[idx] = new_line1;
				assert(now[pre_idx].x_parallel() ^ new_line4.x_parallel());
				if (new_line4.touch(now[pre_idx]))
				{
					now.insert(now.begin() + idx, new_line4);
					now.insert(now.begin() + idx + 1, new_line2);
					now.insert(now.begin() + idx + 3, new_line3);
					now.insert(now.begin() + idx + 4, new_line5);
				}
				else if (new_line4.touch(now[nxt_idx]))
				{
					now.insert(now.begin() + idx, new_line5);
					now.insert(now.begin() + idx + 1, new_line3);
					now.insert(now.begin() + idx + 3, new_line2);
					now.insert(now.begin() + idx + 4, new_line4);
				}
				else
					assert(false);
#ifdef LOCAL
				output();
#endif
				cur_length += abs(width) * 2;
			}
		}
		return;
	}

	bool include(Point l, Point r, Point x)
	{
		return l.x <= x.x && x.x <= r.x && l.y <= x.y && x.y <= r.y;
	}

	bool is_inside(Point p)
	{
		Line line = Line(p, Point(1e9, p.y));
		int cnt = 0;
		for (auto l : now)
			if (!l.x_parallel())
			{
				cnt += l.p1.y <= line.p1.y && line.p1.y < l.p2.y && line.p1.x <= l.p1.x && l.p1.x <= 1e9;
			}
		return cnt % 2 == 1;
	}

	int dist(Point p, Line l)
	{
		if (l.x_parallel())
			return abs(p.y - l.p1.y);
		else
			return abs(p.x - l.p1.x);
	}

	bool upper(Point p, Line l)
	{
		if (l.x_parallel())
			return p.y > l.p1.y;
		else
			return p.x > l.p1.x;
	}

	Point mid(Point p1, Point p2)
	{
		return Point((p1.x + p2.x) / 2, (p1.y + p2.y) / 2);
	}
	Point mid(Line l)
	{
		return mid(l.p1, l.p2);
	}
};

int main()
{
	Solver solver;
	solver.solve();
	solver.output();
	return 0;
}