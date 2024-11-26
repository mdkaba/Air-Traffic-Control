// Group 16 (Mamadou Diao Kaba (27070179), Maika Trepanier (40258910), Lifu Zhang (40081513))
#include "DataDisplay.h"
#include "Structures.h"
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>

using namespace std;

//constructor
DataDisplay::DataDisplay() {}

//destructor
DataDisplay::~DataDisplay() {}

void* DataDisplay::dataDisplay_client() {

    int dataDisplayID;
    data_t msg, rcvmsg;


    array<array<int, 25>, 25> airspace_grid{};
    for (auto &row : airspace_grid) row.fill(-1);

    cTimer refreshTimer(5, 0);

    if ((dataDisplayID = name_open(COMPUTER_SYSTEM_ATTACH, 0)) == -1) {
        perror("Failed to connect to the computer system channel");
        return nullptr; // Early return on failure to connect
    }

    while (true) {
        string craftInfo;
        string gridView;

        msg.hdr.type = static_cast<_Uint16t>(HEADER_TYPE::DATA_DISPLAY);

        if (MsgSend(dataDisplayID, &msg, sizeof(msg), &rcvmsg, sizeof(rcvmsg)) == -1) {
            cerr << " Unable to retrieve data from the computer system server\n";
            break;
        }

        // Reset airspace using fill
        for (auto &row : airspace_grid) row.fill(-1);

        for (const auto& aircraft : rcvmsg.display_data.aircraftDataVector) {
            int gridX = aircraft.x / 5000;
            int gridY = aircraft.y / 5000;

            if (gridX >= 0 && gridX < 30 && gridY >= 0 && gridY < 30) {
                airspace_grid[gridX][gridY] = aircraft.id;
            }
        }


					gridView += "┌───────────────────────────────── RADAR VIEW ──────────────────────────────┐\n";
					for (const auto& row : airspace_grid) {
					    gridView += "│";
					    for (const int id : row) {
					        gridView += id == -1 ? " · " : " " + std::to_string(id) + (id < 10 ? " " : ""); // Adjust for single-digit IDs.
					    }
					    gridView += "│\n";
					}
					gridView += "└───────────────────────────────────────────────────────────────────────────┘\n";

        cout << craftInfo;
        cout << gridView;

        refreshTimer.waitTimer();
    }

    name_close(dataDisplayID);

    return nullptr;
}

void* DataDisplay::startingDataDisplayClient(void* context) {
    return ((DataDisplay*)context)->dataDisplay_client();
}

void DataDisplay::startThreads() {


    // Create and start data display client thread
    if (pthread_create(&displayThreadId, NULL, startingDataDisplayClient, this) != 0) {
        perror("Failed to create data display client thread");
    }


}

void DataDisplay::joinThreads() {
	int err_no = pthread_join(displayThreadId, NULL);
	if (err_no != 0){printf("ERROR when joining the thread\n");}

}

