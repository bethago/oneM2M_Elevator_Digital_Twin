#pragma once
#include <map>
#include <chrono>

#include "send_oneM2M.h"
#include "parse_json.h"

using std::chrono::system_clock;
using std::map;

class socket_oneM2M
{
public:
	send_oneM2M socket;

    string originator_name;
	string building_name;
    string device_name;

	vector<string> Default_CNTs;
    vector<string> Default_PHYSICS_CNTs;
    vector<string> Default_INSIDE_CNTs;
    vector<string> Default_OUTSIDE_CNTs;

	socket_oneM2M(parse_json::parsed_struct parsed_struct, vector<string> ACP_NAMES);
	~socket_oneM2M();

	bool create_oneM2M_under_device_name(parse_json::parsed_struct parsed_struct);

	bool create_oneM2M_under_CNTs(parse_json::parsed_struct parsed_struct);

	bool create_oneM2M_CNTs(parse_json::parsed_struct parsed_struct);

	bool create_oneM2M_SUBs(parse_json::parsed_struct parsed_struct);

	bool create_oneM2M_CINs(parse_json::parsed_struct parsed_struct);

	vector<vector<string>> retrieve_oneM2M_cins(vector<int> floor_info);
};