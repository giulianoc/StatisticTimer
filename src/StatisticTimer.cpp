
#include "StatisticTimer.h"
#include "ThreadLogger.h"
#include <chrono>
#include <numeric>
#include <spdlog/fmt/bundled/ranges.h>

using namespace std;
using json = nlohmann::json;

void StatisticTimer::start(const string& label)
{
	if (const auto it = _uncompletedTimers.find(label); it != _uncompletedTimers.end())
	{
		LOG_WARN("StatisticTimer ({}): the label ({}) is already present, we will update the time", _name, label);
		it->second = chrono::system_clock::now();
	}
	else
		_uncompletedTimers.insert(make_pair(label, chrono::system_clock::now()));
}

chrono::system_clock::duration StatisticTimer::stop(const string& label)
{
	chrono::system_clock::duration d(0);

	if (const auto it = _uncompletedTimers.find(label); it == _uncompletedTimers.end())
		LOG_WARN("StatisticTimer ({}): stop cannot be done because the label ({}) is not present", _name, label);
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

void StatisticTimer::stopAll()
{
	auto end = std::chrono::system_clock::now();

	for (const auto& [label, start] : _uncompletedTimers)
		_timers.emplace_back(start, end, label);

	_uncompletedTimers.clear();
}

string StatisticTimer::toString(const bool summary)
{
	if (!_uncompletedTimers.empty())
		LOG_WARN(
			"StatisticTimer ({}) has {} timers not stopped: {}", _name, _uncompletedTimers.size(),
			fmt::join(_uncompletedTimers | std::views::keys, ", ")
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
	return std::format("statistics ({}): {}", _name, oss.str());
}

json StatisticTimer::toJson()
{
	if (!_uncompletedTimers.empty())
		LOG_WARN(
			"StatisticTimer ({}) has {} timers not stopped: {}", _name, _uncompletedTimers.size(),
			fmt::join(_uncompletedTimers | std::views::keys, ", ")
		);

	json statisticsRoot = json::array();
	for (const tuple<chrono::system_clock::time_point, chrono::system_clock::time_point, string>& timer : _timers)
	{
		auto [start, stop, label] = timer;

		// statisticsRoot.push_back(std::format("{}: {} millisecs", label, chrono::duration_cast<chrono::milliseconds>(stop - start).count()));
		statisticsRoot.push_back(std::format("{}: {:.3f} millisecs", label, chrono::duration<double, std::milli>(stop - start).count()));
	}

	json root;
	root[_name] = statisticsRoot;

	return root;
}
