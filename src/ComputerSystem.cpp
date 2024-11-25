#include <chrono>
#include <ctime>
#include <fstream>
#include <mutex>
#include <vector>
#include <sys/neutrino.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include "ComputerSystem.h"
#include "Structures.h"

using namespace std;

//constructor initialized n = 180 by default
ComputerSystem::ComputerSystem() : n(180) {
    pthread_mutex_init(&mutex, nullptr);
}


ComputerSystem::~ComputerSystem() {
    pthread_mutex_destroy(&mutex);
}

//updating aircraft info
void ComputerSystem::updatingAircraftInfo(const aircraft_data_t& airData) {
	//Flag to track if aircraft already exists in vector.
    bool exists = false;

    pthread_mutex_lock(&mutex);

    //Iterate over existing aircraft data to find a match by ID
    for (auto& aircraft : aircraftVector) {
        if (aircraft.id == airData.id) {
            // Update existing aircraft information.
            aircraft.x = airData.x;
            aircraft.y = airData.y;
            aircraft.z = airData.z;
            aircraft.speedX = airData.speedX;
            aircraft.speedY = airData.speedY;
            aircraft.speedZ = airData.speedZ;
            exists = true; // Mark as found.
            break; // Exit the loop as the update is complete.
        }
    }

    // If the aircraft does not exist, add it to the vector.
    if (!exists) {
        aircraftVector.push_back(airData);
    }

    pthread_mutex_unlock(&mutex);
}


//removing aircraft that is out of range
void ComputerSystem::removeOutofRangeAircraft(const aircraft_data_t& airData) {
    pthread_mutex_lock(&mutex); // Ensure thread-safe access to aircraftVector.

    // Search for the aircraft by ID and remove if found.
    auto it = std::find_if(aircraftVector.begin(), aircraftVector.end(),
    [&airData](const aircraft_data_t& item) { return item.id == airData.id; });
    if (it != aircraftVector.end()) {
        printf("Aircraft ID %d is out of bounds and will be removed\n", airData.id);
        aircraftVector.erase(it);
    }

    pthread_mutex_unlock(&mutex);
}


void* ComputerSystem::collisionMonitor() {
    while (true) {
        cTimer timer(1, 0);

        vector<aircraft_data_t> tempVector;

        pthread_mutex_lock(&mutex);
        // Copy the vector .
        tempVector = aircraftVector;
        pthread_mutex_unlock(&mutex);

        //Update positions based on speed, simulating movement over time
        for (int i = 0; i < n; i++) {
            for (auto& airData : tempVector) {
                airData.x += airData.speedX;
                airData.y += airData.speedY;
                airData.z += airData.speedZ;
            }
            // Check for potential collisions after position updates
            calculateForCollision(tempVector, i);
        }

        timer.waitTimer();
    }

    return nullptr;
}



//calcualte to see if possible collsion
void ComputerSystem::calculateForCollision(const vector<aircraft_data_t>& tmpVector, int time) {

    // Check each unique pair of aircraft for collision risk based on their distances.
    for (size_t i = 0; i < tmpVector.size(); i++) {
        for (size_t j = i + 1; j < tmpVector.size(); j++) {
            if (abs(tmpVector[i].x - tmpVector[j].x) <= 3000 &&
                abs(tmpVector[i].y - tmpVector[j].y) <= 3000 &&
                abs(tmpVector[i].z - tmpVector[j].z) <= 1000) {
                // Print a collision warning.
                printf("time = %d: COLLISION DETECTED BETWEEN AIRCRAFTS %d (%d, %d, %d) AND %d (%d, %d, %d) IMMINENT\n", time,
                       tmpVector[i].id, tmpVector[i].x, tmpVector[i].y, tmpVector[i].z,
                       tmpVector[j].id, tmpVector[j].x, tmpVector[j].y, tmpVector[j].z);
            }
        }
    }
}


//wrapper to start the thread
void* ComputerSystem::startingCollisionMonitor(void* context) {
    return ((ComputerSystem*)context)->collisionMonitor();
}

void *ComputerSystem::computerSystem_server() {

    name_attach_t *attach;
    if ((attach = name_attach(NULL, COMPUTER_SYSTEM_ATTACH, 0)) == NULL) {
        perror("Error occurred while creating the channel");

    } else {
        printf("Computer system attached\n");
    }

    int rcvid;
    int infoAircraftID = -1; // Store the last requested aircraft ID for information.
    data_t msg, request_msg;
    display_data_t display_msg;

    while (true) {

        rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);

        if (rcvid == -1) {
            perror("Message Receive Error");
            break;
        }

        if (rcvid == 0) {
            switch (msg.hdr.code) {
                case _PULSE_CODE_DISCONNECT:
                    ConnectDetach(msg.hdr.scoid);
                    break;
                case _PULSE_CODE_UNBLOCK:
                    // Handling for UNBLOCK pulse, if required.
                    break;
                default:
                    // Handling for other pulse codes.
                    break;
            }
            continue;
        }

        //Handle connection and standard IO messages.
        if (msg.hdr.type == _IO_CONNECT) {
            MsgReply(rcvid, EOK, NULL, 0);
            continue;
        } else if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX) {
            MsgError(rcvid, ENOSYS);
            continue;
        }

        //Process messages based on their header type
        switch (msg.hdr.type) {
            case static_cast<_Uint16t>(HEADER_TYPE::COMP_PRIMARY):
            case static_cast<_Uint16t>(HEADER_TYPE::COMP_SECONDARY):

                //Update aircraft info if in bounds or remove it if out of bounds
                if (msg.hdr.type == static_cast<_Uint16t>(HEADER_TYPE::COMP_PRIMARY) && !msg.aircraft_data.aircraftInBounds) {
                    removeOutofRangeAircraft(msg.aircraft_data);
                } else {
                    updatingAircraftInfo(msg.aircraft_data);
                }
                MsgReply(rcvid, EOK, NULL, 0);
                break;

            case static_cast<_Uint16t>(HEADER_TYPE::DATA_DISPLAY):

                // Prepare and send display data.
                display_msg.augmentedInfoAircraftId = infoAircraftID;
                pthread_mutex_lock(&mutex);
                display_msg.aircraftDataVector = aircraftVector;
                pthread_mutex_unlock(&mutex);
                request_msg.display_data = display_msg;
                MsgReply(rcvid, EOK, &request_msg, sizeof(request_msg));
                infoAircraftID = -1; // Reset after use.
                break;

            case static_cast<_Uint16t>(HEADER_TYPE::OPERATOR_VALID_ID):
                // Check for valid aircraft ID and respond.
                request_msg.command_data.validId = std::any_of(aircraftVector.begin(), aircraftVector.end(),
                                                               [&](const aircraft_data_t& air) { return air.id == msg.command_data.newAircraft.id; });
                MsgReply(rcvid, EOK, &request_msg, sizeof(request_msg));
                break;

            case static_cast<_Uint16t>(HEADER_TYPE::OPERATOR_COMMAND):
                // Handle operator commands.
                sendDataToCommSys(msg.command_data.newAircraft, msg.command_data.command);
                MsgReply(rcvid, EOK, NULL, 0);
                break;

            case static_cast<_Uint16t>(HEADER_TYPE::OPERATOR_REQUEST_INFO):
                // Store ID for later information requests.
                infoAircraftID = msg.display_data.augmentedInfoAircraftId;
                MsgReply(rcvid, EOK, NULL, 0);
                break;

            case static_cast<_Uint16t>(HEADER_TYPE::OPERATOR_PREDICTION_TIME):
                // Update prediction time based on operator input.
                n = msg.command_data.n;
                MsgReply(rcvid, EOK, NULL, 0);
                break;

            default:

                MsgError(rcvid, ENOSYS);
                break;
        }
    }

    name_detach(attach, 0);

    return EXIT_SUCCESS;
}


void * ComputerSystem::startingComputerSystemServer(void * context) {
	 return ((ComputerSystem *)context)->computerSystem_server();
}


// Sends aircraft data and command to the communication system.
void ComputerSystem::sendDataToCommSys(const aircraft_data_t& airData, const std::string& cmd) {
    // Attempt connection with the communication system.
    int csid = name_open(COMMUNICATION_SYSTEM_ATTACH, 0);
    if (csid == -1) {
        perror("Failed to attach to communication system");
        return;
    }

    data_t msg;
    msg.hdr.type = static_cast<_Uint16t>(HEADER_TYPE::COMM_SYS);
    msg.command_data.newAircraft = airData;
    msg.command_data.command = cmd;

    // send the message. Log error on failure.
    if (MsgSend(csid, &msg, sizeof(msg), nullptr, 0) == -1) {
        perror("Failed to send message to communication system");
    }

    name_close(csid);
}


//write in a compute rsystem log every 30 seconds
void* ComputerSystem::CSLog() {
    // Open the log file for appending.
    std::ofstream writer("systemLog.txt", std::ios::app);
    if (!writer.is_open()) {
        perror("Failed to open log file");
        return (void*)EXIT_FAILURE;
    }

    while (true) {
        cTimer timer(30, 0);
        pthread_mutex_lock(&mutex);

        // Log current time.
        auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        writer << "----------------------------------------------------------- \n";
        writer << "NEW LOG " << std::ctime(&currentTime) << "\n";

        // Log information for each tracked aircraft.
        for (const auto& airData : aircraftVector) {
            writer << "Aircraft " << airData.id << ": pos = (" << airData.x << ", " << airData.y << ", " << airData.z << ") "
                   << "speed = (" << airData.speedX << ", " << airData.speedY << ", " << airData.speedZ << ")\n";
        }

        writer << "----------------------------------------------------------- \n";

        pthread_mutex_unlock(&mutex);

        // Flush to ensure data is written to the file.
        writer.flush();
        timer.waitTimer();
    }
    // While the function is intended to run indefinitely, provide a proper exit on loop break.
    return (void*)EXIT_SUCCESS;
}


void* ComputerSystem::startingCSLog(void* context) {
    return ((ComputerSystem*)context)->CSLog();
}

// Implementation of startThreads
void ComputerSystem::startThreads() {

    // Create and start the computer system server thread
    int err_no = pthread_create(&computerSystemThreadId, NULL, startingComputerSystemServer, this);
    if (err_no != 0) { printf("ERROR when creating the computer system server thread \n"); }

    // Create and start the computer system logger thread
    err_no = pthread_create(&computerSystemLoggerThreadId, NULL, startingCSLog, this);
    if (err_no != 0) { printf("ERROR when creating the computer system logger thread \n"); }

    // Create and start the collision system thread
    err_no = pthread_create(&collisionSystemThreadId, NULL, startingCollisionMonitor, this);
    if (err_no != 0) { printf("ERROR when creating the collision system thread \n"); }


}

void ComputerSystem::joinThreads() {
	int err_no = pthread_join(collisionSystemThreadId, NULL);
	if (err_no != 0){printf("ERROR when joining the thread\n");}

	err_no = pthread_join(computerSystemThreadId, NULL);
	if (err_no != 0){printf("ERROR when joining the thread\n");}

	err_no = pthread_join(computerSystemLoggerThreadId, NULL);
	if (err_no != 0){printf("ERROR when joining the thread\n");}
}


