
#pragma once

#include <chrono>
#include <map>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;
using namespace nlohmann::literals;

using namespace std;

class StatisticTimer
{

  public:
	StatisticTimer()= default;
	explicit StatisticTimer(const string_view &name) { _name = name; }
	~StatisticTimer()= default;
	void setName(const string_view &name) { if (_name.empty()) _name = name; }
	void start(const string& label);
	chrono::system_clock::duration stop(const string& label);
	string toString(bool summary = false);
	json toJson();

  private:
	string _name;

	map<string, chrono::system_clock::time_point> _uncompletedTimers;

	vector<tuple<chrono::system_clock::time_point, chrono::system_clock::time_point, string>> _timers;
};
