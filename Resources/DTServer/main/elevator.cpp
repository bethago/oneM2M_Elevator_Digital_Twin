#include "elevator.h"

Elevator::Elevator(parse_json::parsed_struct parsed_struct, vector<string> ACP_NAMES) :
sock(parsed_struct, ACP_NAMES),
p(parsed_struct.underground_floor, parsed_struct.ground_floor, {
        -55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, -10, -7, -4, 1
    })
{
	this->isRunning = true;
	this->RETRIEVE_interval_millisecond = 100;

	this->Elevator_opcode = 0;
	this->go_To_Floor = 0;
	this->building_name = parsed_struct.building_name;
	this->device_name = parsed_struct.device_name;

	this->floor_info.push_back(parsed_struct.underground_floor);
	this->floor_info.push_back(parsed_struct.ground_floor);

	cout << "CREATED ELEVATOR CLASS BUILDING NAME : " << building_name << " DEVICE : " << device_name << endl;
  }

Elevator::~Elevator()
{
	stop_thread();
}

void Elevator::start_thread()
{
	thread temp(&Elevator::run, this);
	temp.detach();
}

void Elevator::stop_thread()
{
}

void Elevator::run()
{
	vector<vector<string>> retrieved_string;
	vector<string> ret_bin;

	vector<vector<long double>> temp2;

    system_clock::time_point start;
    chrono::duration<double> interval;

	socket_oneM2M s = this->sock;
	physics p = this->p;
	simulation sim = this->p.s;

	vector<int> main_trip_list = sim.main_trip_list;
	vector<vector<int>> outside_button_list;

	std::chrono::steady_clock::time_point free_time;

	bool flag;

	while(isRunning)
	{
		start = system_clock::now();
		//GET RETRIEVED INFORMATIONS
		retrieved_string = this->RETRIEVE_from_oneM2M();

		//interval = system_clock::now() - start;
		//cout << device_name << " RETRIEVE TIME : " << interval.count()<< " seconds..." << endl;
		//start = system_clock::now();

		//SET or MODIFY MAIN TRIP LIST
		flag = this->latest.empty;

		//IF FIRST ON OPERATION
		if(flag)
		{
			cout << this->building_name << " -> " << this->device_name  << " : latest RETRIEVE info is EMPTY. SAVING..." << endl;
			this->latest = this->parse_oneM2M_RETRIEVE_to_STRUCT(retrieved_string);

			if(!this->latest.button_outside.empty())
			{
				//VERY FIRST REQUEST FROM DATA -> DEFINE ELEVATOR DIRECTION
				//SET INITIAL DIRECTION
				if(p.init == true)
				{
					p.init = false;
					p.set_initial_elevator_direction(this->latest.button_outside);
				}
				sim.update_main_trip_list_via_outside_data(this->latest.button_outside, p.current_direction);
			}

			if(!this->latest.button_inside.empty())
			{
				if(p.init == true)
				{
					p.init = false;
					p.set_initial_elevator_direction(this->latest.button_inside);
				}
				sim.update_main_trip_list_via_inside_data(this->latest.button_inside, p.current_direction);
			}
			cout << "goTo Floor is Changed None to : "  << sim.main_trip_list[0] << endl;
			//CHANGE V_T Graph
			current_goTo_Floor_vector_info = p.draw_vt_on_single_floor(sim.main_trip_list[0]);

			it = current_goTo_Floor_vector_info.begin();
			current_goTo_Floor_single_info = *it;

			//interval = system_clock::now() - start;
			//cout << device_name <<  "FIRST OPERTAION TIME : " << interval.count()<< " seconds..." << endl;
			//start = system_clock::now();
		}

		//CHECK WITH PREVIOUS CIN RETRIEVE
		else
		{
			latest_RETRIEVE_STRUCT current = this->parse_oneM2M_RETRIEVE_to_STRUCT(retrieved_string);

			sim.check_cin_and_modify_main_trip_list_between_previous_RETRIEVE(this->latest, current, p.current_direction);
			this->latest_RETRIEVE_info = retrieved_string;

			//CHECK LATEST TRIP INFO WITH MODIFIED TRIP LIST
			if(!latest_trip_list_info.empty() && !sim.main_trip_list.empty())
			{
				//cout << "CHECK TRIP LIST IS MODIFIED..." << endl;

				//IF CLOSEST goTo Floor is Changed
				if(latest_trip_list_info[0] != sim.main_trip_list[0])
				{
					cout << "goTo Floor is Changed " << latest_trip_list_info[0] << " to " << sim.main_trip_list[0] << endl;
					//CHANGE V_T Graph
					current_goTo_Floor_vector_info = p.draw_vt_on_single_floor(sim.main_trip_list[0]);

					it = current_goTo_Floor_vector_info.begin();
					current_goTo_Floor_single_info = *it;
				}
				else
				{
					//cout << "goTo Floor is NOT Changed "  << endl;
				}
			}
			//IF CURRENT TRIP LIST IS ENDED -> CHANGE ELEVATOR STATUS
			if(current_goTo_Floor_single_info == current_goTo_Floor_vector_info.back())
			{
				if(p.lock)
				{
					p.lock = false;
					//3 Second LOCK
					free_time = std::chrono::steady_clock::now() + std::chrono::seconds(3);
				}
				else
				{
					if(std::chrono::steady_clock::now() >= free_time)
					{
						p.lock = true;
						if(sim.main_trip_list.size() == 1)
						{
							sim.main_trip_list = sim.pop_floor_of_trip_list(sim.main_trip_list);
							sim.swap_trip_list();

							if(sim.main_trip_list.empty())
							{
								cout << "ALL TRIP LIST IS ENDED, SHUTTING DOWN ELEVATOR..." << endl;
								isRunning = false;
							}
							else
							{
								current_goTo_Floor_vector_info = p.draw_vt_on_single_floor(sim.main_trip_list[0]);

								it = current_goTo_Floor_vector_info.begin();
								current_goTo_Floor_single_info = *it;
							}
						}
						else
						{
							if(sim.main_trip_list.size() >= 2 && sim.main_trip_list[0] == sim.main_trip_list[1])
							{
								sim.main_trip_list = sim.pop_floor_of_trip_list(sim.main_trip_list);
							}
							sim.main_trip_list = sim.pop_floor_of_trip_list(sim.main_trip_list);

							current_goTo_Floor_vector_info = p.draw_vt_on_single_floor(sim.main_trip_list[0]);

							it = current_goTo_Floor_vector_info.begin();
							current_goTo_Floor_single_info = *it;
						}
					}
				}
			}
			else
			{
				++it;
				current_goTo_Floor_single_info = *it;
				p.current_velocity = current_goTo_Floor_single_info[1];
				p.current_altimeter = current_goTo_Floor_single_info[2];
			}
			//interval = system_clock::now() - start;
			//cout << device_name <<  "DEFAULT OPERTAION TIME : " << interval.count()<< " seconds..." << endl;
			//start = system_clock::now();
		}

		if(sim.main_trip_list.empty() && sim.reserved_trip_list_up.empty() && sim.reserved_trip_list_down.empty())
		{
			cout << "ALL TRIP LIST IS ENDED, SHUTTING DOWN ELEVATOR..." << endl;
			isRunning = false;
		}
		else
		{
			//cout << "CURRENT TRIP INFO : " << current_goTo_Floor_single_info[0] << " : VELOCITY : " <<
			//current_goTo_Floor_single_info[1] << " ALTIMETER : " << current_goTo_Floor_single_info[2] << endl;

			latest_trip_list_info = sim.main_trip_list;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(this->RETRIEVE_interval_millisecond));
	}
}

vector<vector<string>> Elevator::RETRIEVE_from_oneM2M()
{
	//RETRIEVE from oneM2M Resource Tree Based on its building_name and device_name
	return this->sock.retrieve_oneM2M_cins(this->floor_info);
}

latest_RETRIEVE_STRUCT Elevator::parse_oneM2M_RETRIEVE_to_STRUCT(vector<vector<string>> ret)
{
	string floor, direction;
	int floor_int, direction_int;

	latest_RETRIEVE_STRUCT temp;

	temp.empty = false;

	temp.velocity = stod(ret[0][0]);
	temp.altimeter = stod(ret[1][0]);

	temp.button_inside = ret[2];

	for(const string& elem : ret[3])
	{
		stringstream ss(elem);

		getline(ss, floor, ':');
		getline(ss >> ws, direction);

		floor_int = this->p.s.string_floor_to_int(floor);
		direction_int = direction._Equal("Up") ? 1 : 0;

		temp.button_outside.push_back({floor_int, direction_int});
	}

	return temp;
}

void Elevator::dev_print()
{
	cout << "ELEVATOR : " << this << " DEV PRINT" << endl;
	cout << "SOCK : " << this->sock.socket_name << endl;
	cout << "PHYSICS : " << this->p.current_velocity << endl;
	cout << "BUILDING NAME : " << this->building_name << endl;
	cout << "DEVICE NAME : " << this->device_name << endl;
}

/*
 * 			string first_goTo_floor = this->latest_RETRIEVE_info[2].front();
			int goTo_floor_index = this->p.s.string_floor_to_int(first_goTo_floor);

			//DRAW V_T TABLE FOR FIRST GOTO FLOOR
			this->current_goTo_Floor_vector_info = this->p.draw_vt_on_sigle_floor(goTo_floor_index);
 */