#pragma once
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;

struct Wparsed_struct
{
	wstring TCName = L"";

	wstring building_name = L"";
	wstring device_name = L"";
	wstring timestamp = L"";

	int underground_floor = 0;
	int ground_floor = 0;
	int init_floor = 0;

	int timestampOffset = 0;

	double velocity = 0.0;
	double max_velocity = 0.0;
	double acceleration = 0.0;

	double altimeter = 0.0;
	double temperature = 0.0;

	double idle_power = 0.0;
	double standby_power = 0.0;
	double iso_power = 0.0;

	bool button_outside_direction;
	bool inFlag = true;
	bool outFlag = true;
	bool useEnergyCalulationFlag = false;

	vector<double> each_floor_altimeter = vector<double>{};

	vector<wstring> button_inside = vector<wstring>{};
	vector<vector<int>> button_outside = vector<vector<int>>{};
};

class parse_json
{
public:
	Wparsed_struct p;

	Wparsed_struct parsingWithBulidingAlgorithms(string json_data, int algorithm_number);

	Wparsed_struct parsingOnlyBuildingName(string json_data);
	Wparsed_struct parsingDedicatedButtonBuilding(string json_data);
	Wparsed_struct parsingCrowdControlButtonBuliding(string json_data);
	Wparsed_struct parsingText(wstring text);

	vector<wstring> splitText(const wstring& s, wchar_t delimiter);

	vector<int> parse_content_type(const std::string& content_type);
	int parse_M2M_RI(const std::string& content_type);
	void modify_inside_data_to_oneM2M(Wparsed_struct p);
	void modify_outside_data_to_oneM2M(Wparsed_struct p);
};