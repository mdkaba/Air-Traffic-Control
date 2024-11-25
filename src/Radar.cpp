#include "Radar.h"
#include <errno.h>
#include <mutex>
#include <algorithm>

//constructor
Radar::Radar(vector<Aircraft*> aircraftVector) : airVector(aircraftVector) {
    pthread_mutex_init(&mutex, nullptr); // Use nullptr instead of NULL for modern C++
}

//destructor - destroy the mutex
Radar::~Radar() {
    pthread_mutex_destroy(&mutex);
}


// Check if an aircraft is within operational airspace boundaries.
bool Radar::ifAirisInBound(aircraft_data_t aircraft) {
    // Valid airspace defined within a 3D rectangular prism.
    return aircraft.x >= 0 && aircraft.x <= 100000 && // Check horizontal bounds on X-axis.
           aircraft.y >= 0 && aircraft.y <= 100000 && // Check horizontal bounds on Y-axis.
           aircraft.z >= 15000 && aircraft.z <= 25000; // Check vertical bounds (Z-axis).
}


//primary radar client
void* Radar::primaryRadar_client() {
    vector<int> IDVector;
    int rcvid, csid;
    data_t msg, request_msg, cs_msg;
    aircraft_data_t airData;

    // Timer initialized with a period of 1 second
    cTimer timer(1, 0);

    // Connect to the ComputerSystem
    if ((csid = name_open(COMPUTER_SYSTEM_ATTACH, 0)) == -1) {
        perror("Primary Radar: Error occurred while attaching the computer channel");
    }

    while (true) {
        // Update connections to any new aircraft
        for (auto it = airVector.begin(); it != airVector.end();) {
            Aircraft* aircraft = *it;
            rcvid = name_open(aircraft->attachPoint.c_str(), 0);
            if (rcvid != -1) {
                IDVector.push_back(rcvid);
                it = airVector.erase(it); // Remove connected aircraft from vector
            } else {
                ++it;
            }
        }

        // Clear the list of radar-detected aircraft
        radarAircraft.clear();

        for (auto &sCoid : IDVector) {
            msg.hdr.type = static_cast<_Uint16t>(HEADER_TYPE::PRIMARY_RADAR);

            if (MsgSend(sCoid, &msg, sizeof(msg), &request_msg, sizeof(request_msg)) == -1) {
                perror("Primary Radar: Error while requesting aircraft info");
                IDVector.erase(std::remove(IDVector.begin(), IDVector.end(), sCoid), IDVector.end());
                name_close(sCoid);
                continue;
            }

            airData = request_msg.aircraft_data;
            airData.aircraftInBounds = ifAirisInBound(airData);

            cs_msg.hdr.type = static_cast<_Uint16t>(HEADER_TYPE::COMP_PRIMARY);
            cs_msg.aircraft_data = airData;

            if (MsgSend(csid, &cs_msg, sizeof(cs_msg), nullptr, 0) == -1) {
                perror("Primary Radar: Error while sending to computer system");
                break;
            }

            if (!airData.aircraftInBounds) {
                name_close(sCoid);
                IDVector.erase(std::remove(IDVector.begin(), IDVector.end(), sCoid), IDVector.end());
            } else {
                radarAircraft.push_back(airData);
            }
        }

        // Avoid overload
        timer.waitTimer();
    }

    // Close connections
    for (int sCoid : IDVector) {
        name_close(sCoid);
    }

    return EXIT_SUCCESS;
}



//Static wrapper to start primaryRadarClient() thread
void* Radar::startingPrimaryRadarCLient(void* context) {
    return ((Radar*)context)->primaryRadar_client();
}



void* Radar::secondaryRadar_client() {
    int rcvid, csid;
    data_t msg, request_msg, cs_msg;

    // Timer initialized with a period of 1 second
    cTimer timer(1, 0);

    // Connect to the Communication System
    if ((csid = name_open(COMMUNICATION_SYSTEM_ATTACH, 0)) == -1) {
        perror("Secondary Radar: Error occurred while attaching the communication channel");
    }

    while (true) {
        std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);

        for (const auto& aircraft : radarAircraft) {
            string attach_point = "attach_" + std::to_string(aircraft.id);
            rcvid = name_open(attach_point.c_str(), 0);

            if (rcvid == -1) {
                printf("Secondary Radar: Error occurred while attaching the channel %s\n", attach_point.c_str());
                continue;
            }

            msg.hdr.type = static_cast<_Uint16t>(HEADER_TYPE::SECONDARY_RADAR);

            if (MsgSend(rcvid, &msg, sizeof(msg), &request_msg, sizeof(request_msg)) == -1) {
                perror("Secondary Radar: Error requesting data from aircraft");
                name_close(rcvid);
                continue;
            }

            cs_msg.hdr.type = static_cast<_Uint16t>(HEADER_TYPE::COMP_SECONDARY);
            cs_msg.aircraft_data = request_msg.aircraft_data;

            if (MsgSend(csid, &cs_msg, sizeof(cs_msg), nullptr, 0) == -1) {
                perror("Secondary Radar: Error sending data to computer system");
                break;
            }

            name_close(rcvid);
        }

        timer.waitTimer();
    }

    name_close(csid);
    return EXIT_SUCCESS;
}



//wrapper to start thread
void* Radar::startingSecondaryRadarClient(void* context) {
    return ((Radar*)context)->secondaryRadar_client();
}

void Radar::startThreads() {
   // pthread_t primaryThreadId, secondaryThreadId;

    // Create and start primary radar client thread
    if (pthread_create(&primaryThreadId, NULL, startingPrimaryRadarCLient, this) != 0) {
        perror("Failed to create primary radar client thread");
    }

    // Create and start secondary radar client thread
    if (pthread_create(&secondaryThreadId, NULL, startingSecondaryRadarClient, this) != 0) {
        perror("Failed to create secondary radar client thread");
    }


}

void Radar::joinThreads() {
    int err_no = pthread_join(primaryThreadId, NULL);
    if (err_no != 0) { printf("ERROR when joining the primary radar thread\n"); }

    err_no = pthread_join(secondaryThreadId, NULL);
    if (err_no != 0) { printf("ERROR when joining the secondary radar thread\n"); }
}
