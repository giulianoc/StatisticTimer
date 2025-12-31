
#pragma once

#include <chrono>
#include <map>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

/*
using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;
using namespace nlohmann::literals;
*/

class StatisticTimer
{

  public:
	StatisticTimer()= default;
	explicit StatisticTimer(const std::string_view &name) { _name = name; }
	~StatisticTimer()= default;
	void setName(const std::string_view &name) { if (_name.empty()) _name = name; }
	void start(const std::string& label);
	std::chrono::system_clock::duration stop(const std::string& label);
	std::string toString(bool summary = false);
	nlohmann::json toJson();

  private:
	std::string _name;

	std::map<std::string, std::chrono::system_clock::time_point> _uncompletedTimers;

	std::vector<std::tuple<std::chrono::system_clock::time_point, std::chrono::system_clock::time_point, std::string>> _timers;
};
