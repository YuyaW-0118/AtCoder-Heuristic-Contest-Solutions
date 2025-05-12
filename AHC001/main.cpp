#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <vector>

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

struct point
{
	int x, y;
	point() : x(0), y(0) {}
	point(int x, int y) : x(x), y(y) {}
	point(const point &p) : x(p.x), y(p.y) {}
};

struct rect
{
	point p1, p2;
	rect() : p1(point()), p2(point()) {}
	rect(point p1, point p2) : p1(p1), p2(p2) {}
	rect(const rect &r) : p1(r.p1), p2(r.p2) {}

	int area()
	{
		return (p2.x - p1.x) * (p2.y - p1.y);
	}
	double score(int required_area)
	{
		double ratio = (double)std::min(required_area, area()) / std::max(required_area, area());
		return 1.0 - (1.0 - ratio) * (1.0 - ratio);
	}
	bool intersect(const rect &other)
	{
		if (std::max(p1.x, other.p1.x) < std::min(p2.x, other.p2.x) && std::max(p1.y, other.p1.y) < std::min(p2.y, other.p2.y))
		{
			return true;
		}
		return false;
	}
	bool include(const point &p)
	{
		return p1.x <= p.x && p.x < p2.x && p1.y <= p.y && p.y < p2.y;
	}
};
std::ostream &operator<<(std::ostream &os, const rect &r)
{
	os << r.p1.x << " " << r.p1.y << " " << r.p2.x << " " << r.p2.y;
	return os;
}

struct Solver
{
public:
	Solver() : rnd(Random(0)), timekeeper(TimeKeeperDouble(4950))
	{
		input();
	}

	void solve()
	{
		ans.resize(n);
		for (int i = 0; i < n; i++)
		{
			ans[i] = rect(required_points[i], point(required_points[i].x + 1, required_points[i].y + 1));
		}
		double first_temp = 2e-4, last_temp = 1e-7;
		while (true)
		{
			timekeeper.setNowTime();
			if (timekeeper.isTimeOver())
				break;
			double temp = first_temp + (last_temp - first_temp) * timekeeper.getNowTime() / 5000;
			annealing(temp);
		}
		return;
	}

	void output()
	{
		for (int i = 0; i < n; i++)
		{
			std::cout << ans[i] << std::endl;
		}
	}

private:
	int n;
	std::vector<point> required_points;
	std::vector<int> required_areas;

	Random rnd;
	TimeKeeperDouble timekeeper;

	std::vector<rect> ans;

	void input()
	{
		std::cin >> n;
		required_points.resize(n);
		required_areas.resize(n);
		for (int i = 0; i < n; i++)
		{
			std::cin >> required_points[i].x >> required_points[i].y >> required_areas[i];
		}
		return;
	}

	bool is_in_grid(const point &p)
	{
		return 0 <= p.x && p.x < 10000 && 0 <= p.y && p.y < 10000;
	}
	bool is_in_grid(const rect &r)
	{
		return is_in_grid(r.p1) && is_in_grid(r.p2);
	}

	rect change_rect(const rect &r, int op)
	{
		rect new_rect = r;
		if (op == 0)
			new_rect.p1.x--;
		else if (op == 1)
			new_rect.p1.y--;
		else if (op == 2)
			new_rect.p2.x++;
		else if (op == 3)
			new_rect.p2.y++;
		else if (op == 4)
			new_rect.p1.x++;
		else if (op == 5)
			new_rect.p1.y++;
		else if (op == 6)
			new_rect.p2.x--;
		else if (op == 7)
			new_rect.p2.y--;
		return new_rect;
	}

	void annealing(double temp)
	{
		int idx = rnd.nextInt(n);
		int op = rnd.nextInt(8);
		rect new_rect = change_rect(ans[idx], op);
		if (!is_in_grid(new_rect) || !new_rect.include(required_points[idx]))
			return;
		double pre_score = ans[idx].score(required_areas[idx]);
		double new_score = new_rect.score(required_areas[idx]);
		double diff = new_score - pre_score;
		if (diff < 0 && rnd.nextDouble() > exp(diff / temp))
			return;
		bool ok = true;
		for (int i = 0; i < n; i++)
			if (i != idx)
			{
				if (new_rect.intersect(ans[i]))
				{
					ok = false;
					break;
				}
			}
		if (ok)
			ans[idx] = new_rect;
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
