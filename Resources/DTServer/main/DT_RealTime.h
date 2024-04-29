#pragma once
#include <boost/asio.hpp>
#include <cpprest/http_client.h>

#include "elevator.h"
#include "parse_json.h"
#include "socket_oneM2M.h"

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::client;

struct class_of_one_Building
{
	int outsideButtonMod = 0;

	vector<Elevator*> classOfAllElevators;
	map<wstring, wstring> subscriptionRI_to_RN;
	wstring ACP_NAME = L"";
	wstring log_name_for_building = L"";
};

struct hash_function{
	size_t operator()(const vector<int>& vec) const
	{
		std::hash<int> hasher;
		size_t answer = 0;

		for (int i = 0; i < vec.size(); i++)
		{
			answer ^= hasher(i) + 0x9e3779b9 +
				(answer << 6) + (answer >> 2);
		}

		return answer;
	}
};

class Building
{
public:
	Building(wstring building_name, int buttonMod);

	//const time for this building created time
	std::chrono::system_clock::time_point buliding_start_time;

	class_of_one_Building* buildingElevatorInfo;
	vector<vector<int>>* currentButtonOutsideInfos;
	logger* this_log;

	int getButtonMod();
	void getWhichElevatorToGetButtonOutside(vector<vector<int>> button_outside);

	vector<vector<int>> getDiffBetweenNewAndCurrent(vector<vector<int>> newList, vector<vector<int>> oldList);
};

class dt_real_time
{
public:
	dt_real_time();

	void Run();

	void startAsyncAccept_Embedded(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::io_context& ioContext);
	void startAsyncAccept_Notification(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::io_context& ioContext);
	void handleConnection(boost::asio::ip::tcp::socket& socket, int port);

	void Running_Embedded(const wstring& httpResponse);
	void CreateNewBuildingAndElevator(const wstring& httpResponse, const wstring& ACOR_NAME, elevator_resource_status status);
	void setConstructArgumentsOfElevator(Building* this_buliding, Elevator* new_elevator);

	void Running_Notification(const string& httpResponse);

	bool bExistsElevator(Building* one_building, const wstring& device_name);

	Building* getBuilding(const wstring& building_name);
	Elevator* getElevator(Building* class_of_one_building, const wstring& device_name);

	//MAIN 함수에서 알고리즘에 전달할 notificationContent 구조체
    notificationContent* noti_content;

	vector<Building*> allBuildingInfo; //THIS WILL BE USED
	vector<thread> one_building_threads;
	vector<wstring> ACP_NAMES;

	parse_json data_parser;
	Wparsed_struct parsed_struct;
	wstring ACOR_NAME;
	wstring AE_NAME;
	wstring CNT_NAME;
	wstring building_name;

	int VisualizeMod = 0;

	std::wstring* httpRequest;

private:
};