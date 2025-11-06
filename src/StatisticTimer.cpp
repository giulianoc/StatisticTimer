
#include "StatisticTimer.h"
#include "spdlog/spdlog.h"
#include <chrono>
#include <numeric>

void StatisticTimer::start(const string& label)
{
	auto it = _uncompletedTimers.find(label);
	if (it != _uncompletedTimers.end())
	{
		SPDLOG_WARN("StatisticTimer ({}): the label ({}) is already present, we will update the time", _name, label);
		it->second = chrono::system_clock::now();
	}
	else
		_uncompletedTimers.insert(make_pair(label, chrono::system_clock::now()));
}

chrono::system_clock::duration StatisticTimer::stop(const string& label)
{
	chrono::system_clock::duration d(0);

	auto it = _uncompletedTimers.find(label);
	if (it == _uncompletedTimers.end())
		SPDLOG_WARN("StatisticTimer ({}): stop cannot be done because the label ({}) is not present", _name, label);
	else
	{
		chrono::system_clock::time_point start = it->second;
		chrono::system_clock::time_point end = chrono::system_clock::now();
		d = end - start;
		_timers.emplace_back(start, end, label);
		_uncompletedTimers.erase(it);
	}

	return d;
}

string StatisticTimer::toString(bool summary)
{
	if (!_uncompletedTimers.empty())
		SPDLOG_WARN(
			"StatisticTimer ({}) has {} timers not stopped: {}", _name, _uncompletedTimers.size(),
			accumulate(
				begin(_uncompletedTimers), end(_uncompletedTimers), string(),
				[](const string &s, pair<string, chrono::system_clock::time_point> timer)
				{ return (s.empty() ? timer.first : (s + ", " + timer.first)); }
			)
		);

	ostringstream oss;
	bool ossEmpty = true;
	long totalElapsed = 0;
	for (const tuple<chrono::system_clock::time_point, chrono::system_clock::time_point, string>& timer : _timers)
	{
		auto [start, stop, label] = timer;

		if (summary)
			totalElapsed += chrono::duration_cast<chrono::milliseconds>(stop - start).count();
		else
			oss << std::format("{}{}: {}", ossEmpty ? "" : ", ", label, chrono::duration_cast<chrono::milliseconds>(stop - start).count());
		ossEmpty = false;
	}

	if (summary)
		return std::format("statistics ({}): {}", _name, totalElapsed);
	else
		return std::format("statistics ({}): {}", _name, oss.str());
}

json StatisticTimer::toJson()
{
	if (!_uncompletedTimers.empty())
		SPDLOG_WARN(
			"StatisticTimer ({}) has {} timers not stopped: {}", _name, _uncompletedTimers.size(),
			accumulate(
				begin(_uncompletedTimers), end(_uncompletedTimers), string(),
				[](const string &s, pair<string, chrono::system_clock::time_point> timer)
				{ return (s == "" ? timer.first : (s + ", " + timer.first)); }
			)
		);

	json statisticsRoot = json::array();
	for (const tuple<chrono::system_clock::time_point, chrono::system_clock::time_point, string>& timer : _timers)
	{
		auto [start, stop, label] = timer;

		statisticsRoot.push_back(std::format("{}: {} millisecs", label, chrono::duration_cast<chrono::milliseconds>(stop - start).count()));
	}

	json root;
	root[_name] = statisticsRoot;

	return root;
}
