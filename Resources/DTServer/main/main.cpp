﻿#include "main.h"
#include "parse_json.h"
#include "send_oneM2M.h"
#include "socket_oneM2M.h"
#include "simulation.h"
#include "elevator.h"
#include "test.h"

#define PORT_EMBEDDED 10050
#define PORT_NOTIFICATION 10053
#define BUFFER_SIZE 1024

#define _WIN32_WINNT_WIN10 0x0A00
#define oneM2M_CSE_Server "192.168.0.178"
#define oneM2M_CSE_Port "10051"

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::vector;
using std::chrono::system_clock;

void print_elapsed_time()
{
	int elapsed = 0;
    while(true)
    {
	    std::this_thread::sleep_for(std::chrono::seconds(1));
        elapsed++;
        std::cout << "Elapsed Time " << elapsed << "seconds \n";
    }
}

DTServer::DTServer()
{
}

bool DTServer::existsElevator(class_of_one_Building one_building, string device_name)
{
    try
	{
		for(const Elevator* elem : one_building.classOfAllElevators)
	    {
		    if(elem->device_name == device_name)
		    {
			    return true;
		    }
	    }
        return false;
	}
	catch (exception& e)
	{
        cerr << "ERROR Occurred on DTServer::get_elevator : " << e.what() << endl;
        exit(0);
	}
}

class_of_one_Building* DTServer::get_building_vector(string ACOR_NAME)
{
    for(int i = 0 ; i<this->class_of_all_Buildings.size() ; i++)
    {
        if(this->class_of_all_Buildings[i]->ACP_NAME == ACOR_NAME)
	    {
		    return this->class_of_all_Buildings[i];
	    }
    }
    return NULL;
}

Elevator* DTServer::getElevator(class_of_one_Building* class_of_one_building, string device_name)
{
    try
	{
        for(size_t i = 0; i < class_of_one_building->classOfAllElevators.size(); i++)
        {
	        if(class_of_one_building->classOfAllElevators[i]->device_name == device_name)
	        {
		        return class_of_one_building->classOfAllElevators[i];
	        }
        }
	}
	catch (exception& e)
	{
        cerr << "ERROR Occurred on DTServer::get_elevator : " << e.what() << endl;
        std::exit(0);
	}
}

socket_oneM2M DTServer::get_oneM2M_socket_based_on_AE_ID(vector<Elevator*> elevator_array, const string AE_ID)
{
    for(const Elevator* each_elevator_class : elevator_array)
    {
        socket_oneM2M socket = *(each_elevator_class->sock);
	    if(socket.building_name == AE_ID)
	    {
            return socket;
	    }
    }
    std::cout << "Error occurred in Class DTServer::get_oneM2M_socket_based_on_AE_ID -> AE_ID Not Found : " << AE_ID << std::endl;
    exit(0);
}

void DTServer::Run()
{
    boost::asio::io_context ioContext;

    tcp::endpoint EMBEDDED_END(tcp::v4(), PORT_EMBEDDED);
    tcp::endpoint NOTIFICATION_END(tcp::v4(), PORT_NOTIFICATION);

    tcp::acceptor acceptor1(ioContext, EMBEDDED_END, PORT_EMBEDDED);
    tcp::acceptor acceptor2(ioContext, NOTIFICATION_END, PORT_NOTIFICATION);
    //std::thread timerThread(print_elapsed_time);

    acceptor1.async_accept([&](const boost::system::error_code& error, ip::tcp::socket socket) {
        if (!error) {
            //std::cout << "Accepted connection on port " << PORT_EMBEDDED << std::endl;
            handleConnection(socket, PORT_EMBEDDED);
        }
    	else 
        {
            std::cerr << "Error accepting connection on port " << PORT_EMBEDDED << ": " << error.message() << std::endl;
        }
        // Start accepting again
        startAsyncAccept(acceptor1,  ioContext);
    });

    acceptor2.async_accept([&](const boost::system::error_code& error, ip::tcp::socket socket) {
        if (!error) {
            //std::cout << "Accepted connection on port " << PORT_NOTIFICATION << std::endl;
            handleConnection(socket, PORT_NOTIFICATION);
        }
    	else 
        {
            std::cerr << "Error accepting connection on port " << PORT_NOTIFICATION << ": " << error.message() << std::endl;
        }
        // Start accepting again
        startAsyncAccept2(acceptor2,  ioContext);
    });

    ioContext.run();

    // Start accepting connections
    // (You would typically handle connections asynchronously)
}

void DTServer::startAsyncAccept(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::io_context& ioContext)
{
    acceptor.async_accept([&](const boost::system::error_code& error, ip::tcp::socket socket) {
        if (!error) {
            //std::cout << "Accepted connection on port " << PORT_EMBEDDED << std::endl;
            handleConnection(socket, PORT_EMBEDDED);
        }
    	else 
        {
            std::cerr << "Error accepting connection on port " << PORT_EMBEDDED << ": " << error.message() << std::endl;
        }
        // Start accepting again
        startAsyncAccept(acceptor, ioContext);
    });
}

void DTServer::startAsyncAccept2(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::io_context& ioContext)
{
    acceptor.async_accept([&](const boost::system::error_code& error, ip::tcp::socket socket) {
        if (!error) {
            //std::cout << "Accepted connection on port " << PORT_NOTIFICATION << std::endl;
            handleConnection(socket, PORT_NOTIFICATION);
        }
    	else 
        {
            std::cerr << "Error accepting connection on port " << PORT_NOTIFICATION << ": " << error.message() << std::endl;
        }
        // Start accepting again
        startAsyncAccept2(acceptor, ioContext);
    });
}

void DTServer::handleConnection(boost::asio::ip::tcp::socket& socket, int port)
{
    boost::asio::streambuf receiveBuffer(BUFFER_SIZE);
    boost::system::error_code error;
    std::string httpRequest;

    if(port == PORT_NOTIFICATION)
    {
	    //boost::asio::read_until(socket, receiveBuffer, "\0", error);
        boost::asio::read(socket, receiveBuffer, error);

        std::istream inputStream(&receiveBuffer);
        std::ostringstream httpBodyStream;

        // Read the HTTP headers
        while (std::getline(inputStream, httpRequest) && httpRequest != "\r") {
            // Process or ignore HTTP headers as needed
        }

        // Read the HTTP body (JSON string)
        if (inputStream.peek() != EOF) {
            std::getline(inputStream, httpRequest); // Read the empty line after the headers
        }
    }
    else
    {
	    boost::asio::read_until(socket, receiveBuffer, "\n");

        std::istream inputStream(&receiveBuffer);
        std::getline(inputStream, httpRequest);
    }
    if (!error) 
    {
        if (port == PORT_EMBEDDED)
        {
            this->Running_Embedded(httpRequest);
        }
    	else if (port == PORT_NOTIFICATION) 
        {
            this->Running_Notification(httpRequest);
        }
    }
	else
    {
        std::cerr << "Error in DTServer::handleConnection : " << error.message() << std::endl;
        exit(0);
    }
}

void DTServer::Running_Embedded(const std::string& httpResponse)
{
    Elevator* thisBuildingElevator;

    parse_json c;
    const parse_json::parsed_struct parsedStruct = c.parsing(httpResponse);

    string ACOR_NAME = parsedStruct.building_name;
    string AE_NAME = parsedStruct.building_name;
    string CNT_NAME = parsedStruct.device_name;

    int ACP_FLAG = ACP_Validation_Socket.acp_validate(ACOR_NAME, 0);

    // CHECK THIS ACP Exists
    if(ACP_FLAG == 0 || ACP_FLAG == 1)
    {
        this->ACP_NAMES.push_back(ACOR_NAME);

        // Create THIS AE(Building) Class
        class_of_one_Building* temp = new class_of_one_Building;

        int algNumber;

        //cout <<  "SET THIS ELEVATOR ALGORITHM : 1, 2 = ";
        //cin >> algNumber;

        // TEST
        if(CNT_NAME == "EV1")
        {
	        algNumber = 1;
        }
        else
        {
	        algNumber = 2;
        }

        // Create THIS CNT(Building's Elevator) Class
        thisBuildingElevator = new Elevator(parsedStruct, this->ACP_NAMES, algNumber);

        temp->classOfAllElevators.push_back(thisBuildingElevator);
        temp->ACP_NAME = ACOR_NAME;

        class_of_all_Buildings.push_back(temp);

		thisBuildingElevator->runElevator();
        thisBuildingElevator->sock->create_oneM2M_SUBs(parsedStruct);
        thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);

        //DISCOVERY TEST
	    //this_Building_Elevator->sock.socket.discovery_retrieve(this_Building_Elevator->sock.originator_name, 0);

        //RCN TEST
        //this_Building_Elevator->sock.socket.result_content_retrieve(this_Building_Elevator->sock.originator_name, 0);
    }
    else
    {
        std::cout << "Building Exists, Check Elevator " << CNT_NAME << " Exists..." << std::endl;

        // GET THIS BUILDING CLASS and CHECK THIS CNT(Device Name) Exists
        class_of_one_Building* temp = this->get_building_vector(ACOR_NAME);

        // if TEMP == NULL, It means oneM2M Resource Tree Exists, But Server Restarted
        if(temp == NULL)
        {
	        this->ACP_NAMES.push_back(ACOR_NAME);

	        // Create THIS AE(Building) Class
	        class_of_one_Building* temp = new class_of_one_Building;

            int algNumber;
            //cout <<  "SET THIS ELEVATOR ALGORITHM : 1, 2 = ";
			//cin >> algNumber;

            if(CNT_NAME == "EV1")
	        {
		        algNumber = 1;
	        }
	        else
	        {
		        algNumber = 2;
	        }

	        // Create THIS CNT(Building's Elevator) Class
	        thisBuildingElevator = new Elevator(parsedStruct, this->ACP_NAMES, algNumber);

	        temp->classOfAllElevators.push_back(thisBuildingElevator);
	        temp->ACP_NAME = ACOR_NAME;

	        class_of_all_Buildings.push_back(temp);

			thisBuildingElevator->runElevator();
        	thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
        }

        else
        {
	        bool flag = this->existsElevator(*temp, CNT_NAME);

	        if(flag)
	        {
	            // THIS CNT Exists
	            Elevator* thisBuildingElevator = this->getElevator(temp, CNT_NAME);

	            // Update CIN Value
	            std::cout << "Elevator Found, Updating CIN based on this Elevator : " << CNT_NAME << std::endl;
	            thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
	            //DISCOVERY TEST
		        //This_Elevator.sock.socket.discovery_retrieve(This_Elevator.sock.originator_name, 0);

	            //RCN TEST
	            //This_Elevator.sock.socket.result_content_retrieve(This_Elevator.sock.originator_name, 0);
	        }
	        else
	        {
	            // THIS CNT NOT Exists
	            std::cout << "Elevator Not Found, Creating New Elevator : " << CNT_NAME << std::endl;

                int algNumber;
	            //cout <<  "SET THIS ELEVATOR ALGORITHM : 1, 2 = ";
				//cin >> algNumber;

                if(CNT_NAME == "EV1")
		        {
			        algNumber = 1;
		        }
		        else
		        {
			        algNumber = 2;
		        }

	            // Create THIS CNT(Building's Elevator) Class
		        thisBuildingElevator = new Elevator(parsedStruct, this->ACP_NAMES, algNumber);

		        temp->classOfAllElevators.push_back(thisBuildingElevator);
		        temp->ACP_NAME = ACOR_NAME;

		        class_of_all_Buildings.push_back(temp);

		        //THREADING
		        thisBuildingElevator->runElevator();
		        thisBuildingElevator->sock->create_oneM2M_SUBs(parsedStruct);
		        thisBuildingElevator->sock->create_oneM2M_CINs(parsedStruct);
	        }
        }
    }
}

void DTServer::Running_Notification(const std::string& httpResponse)
{
    try
	{
		auto json_body = nlohmann::json::parse(httpResponse);

		if (json_body.contains("m2m:sgn")) 
		{
            auto nev = json_body["m2m:sgn"]["nev"];

            // Check if "rep" object exists within "nev"
            if (nev.contains("rep")) 
			{
                auto rep_json = nlohmann::json::parse(nev["rep"].get<std::string>());

                if (rep_json.contains("m2m:sub")) 
				{
					return ;
                }
            	else
				{
                    //START ANALYZE
					std::string sur_string = json_body["m2m:sgn"]["sur"];
					std::stringstream ss(sur_string);

                    std::vector<std::string> sur_values;
                    std::string item;

                    while (std::getline(ss, item, '/')) 
					{
                        sur_values.push_back(item);
                    }

					string building_name = sur_values[1];
					string device_name = sur_values[2];
					string majorCNTName = sur_values[3];

                    class_of_one_Building* this_building = this->get_building_vector(building_name);
                    Elevator* thisElevator = this->getElevator(this_building, device_name);

					string con;
					std::string rep_value_str = json_body["m2m:sgn"]["nev"]["rep"];

			        // Parse the value of "rep" as JSON
					nlohmann::json rep_value_json = nlohmann::json::parse(rep_value_str);

                    tempContentMoveToAlgorithm = new notificationContent;

					if(majorCNTName == "Elevator_physics")
					{
						string minorCNTName = sur_values[4];

						if(minorCNTName.find("Velocity") != string::npos)
						{
							con = rep_value_json["m2m:cin"]["con"].get<string>();
                            tempContentMoveToAlgorithm->velocity = stod(con);
						}
						else if(minorCNTName.find("Altimeter") != string::npos)
						{
							con = rep_value_json["m2m:cin"]["con"].get<string>();
							tempContentMoveToAlgorithm->altimeter = stod(con);
						}
                        thisElevator->setNotificationContent(tempContentMoveToAlgorithm);
					}
					else if(majorCNTName == "Elevator_button_inside")
					{
						string minorCNTName = sur_values[4];

						if(minorCNTName.find("Button_List") != string::npos)
						{
				            con = rep_value_json["m2m:cin"]["con"].get<string>();

							std::vector<int> integers;
							std::istringstream iss(con);
						    std::string token;
						    while (iss >> token) 
							{
                                // Convert each token (integer string) to an integer and store it in the vector
                                if(token[0] == 'B')
                                {
	                                integers.push_back(-1 * stoi(token.substr(1)));
                                }

                                else
                                {
	                                integers.push_back(stoi(token));
                                }
							}
                            if(integers[0] == -1)
                            {
	                            cout << "" <<endl;
                            }
							tempContentMoveToAlgorithm->button_inside_list = integers;
						}
						else if(minorCNTName.find("goTo") != string::npos)
						{
					        con = rep_value_json["m2m:cin"]["con"].get<string>();
							tempContentMoveToAlgorithm->goTo = stoi(con);
						}

                        thisElevator->setNotificationContent(tempContentMoveToAlgorithm);
                        thisElevator->setUpdateElevatorTick(thisElevator->UEsock, thisElevator->p);
					}
					else if(majorCNTName == "Elevator_button_outside")
					{
						string minorCNTName = sur_values[4];
						con = rep_value_json["m2m:cin"]["con"].get<string>();

				        int called_floor;
				        bool direction;

                        //가정 1. 바깥에 누른 층은 한번 누르면 끝
						if(con == "None") //바깥의 층에 도달하여 자연스럽게 사라진 경우 None
						{
							return ;
						}
				        else if(con == "Up")
				        {
					        direction = true;
				        }
				        else
				        {
					        direction = false;
				        }
						if(!minorCNTName.empty() && minorCNTName[0] == 'B')
				        {
					        called_floor = (minorCNTName[minorCNTName.size()-1] - '0') * -1;
							tempContentMoveToAlgorithm->added_button_outside_altimeter = thisElevator->p->info.altimeter_of_each_floor[called_floor+thisElevator->p->info.underground_floor];
				        }
				        else
				        {
					        called_floor = stoi(minorCNTName);
							tempContentMoveToAlgorithm->added_button_outside_altimeter = thisElevator->p->info.altimeter_of_each_floor[called_floor+thisElevator->p->info.underground_floor-1];
				        }
						tempContentMoveToAlgorithm->added_button_outside_floor = called_floor;
						tempContentMoveToAlgorithm->added_button_outside_direction = direction;

                        thisElevator->setNotificationContent(tempContentMoveToAlgorithm);
                        thisElevator->setUpdateElevatorTick(thisElevator->UEsock, thisElevator->p);
					}
                }
            }
			else 
			{
                std::cout << "rep field does not exist within nev." << std::endl;
				exit(0);
            }
        }
		else 
		{
            std::cout << "m2m:sgn field does not exist." << std::endl;
			exit(0);
        }
	}
	catch (std::exception e)
	{
		std::cerr << "Error in DTServer::Running_Notification : " << e.what() << std::endl;
	}
}

int main()
{
    try
    {
    	DTServer digital_twin_server;
    	digital_twin_server.Run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}