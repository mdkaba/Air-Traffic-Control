#include "Aircraft.h"
#include <mutex>
#include <string>


//constructor initializing aircraft and different attachpoint
Aircraft::Aircraft(int time, int id, int x, int y, int z, int vx, int vy, int vz)
: entryTime(time), id(id), x(x), y(y), z(z), vx(vx), vy(vy), vz(vz),
attachPoint("attach_" + std::to_string(id)) {
    pthread_mutex_init(&lock, nullptr);
}


Aircraft::~Aircraft() {
    pthread_mutex_destroy(&lock);
}

// change current speed to new speed
void Aircraft::newVelocity(int newVX, int newVY, int newVZ) {
	pthread_mutex_lock(&lock);
	vx = newVX;
	vy = newVY;
	vz = newVZ;
	pthread_mutex_unlock(&lock);
}

// change position to new position
void Aircraft::newPosition(int newX, int newY) {
	pthread_mutex_lock(&lock);
	x = newX;
	y = newY;
	pthread_mutex_unlock(&lock);
}

// change current altitude to new altitude
void Aircraft::newAltitude(int newAltitude) {
	pthread_mutex_lock(&lock);
	z = newAltitude;
	pthread_mutex_unlock(&lock);
}


void * Aircraft::updateAircraftPosition() {
	//Initilize Timer to 1s period
	cTimer timer(1, 0);

	// updating position when the aircraft is in bounds
	while (true)	{
		pthread_mutex_lock(&lock);
		x += vx;
		y += vy;
		z += vz;
		pthread_mutex_unlock(&lock);
		timer.waitTimer();
	}
	pthread_exit(EXIT_SUCCESS);
}


//Static wrapper used to start the updateAicraftPosition() thread
void* Aircraft::StartingupdateAircraftPosition(void* context) {
    return ((Aircraft*)context)->updateAircraftPosition();
}

// Server thread for handling messages incomming from primary and secondary radar and command request from operator
void* Aircraft::aircraft_server() {
    int rcvid;
    name_attach_t* attach;
    data_t msg, request_msg;
    aircraft_data_t airData;

    // Create a channel for communication
    if ((attach = name_attach(nullptr, attachPoint.c_str(), 0)) == nullptr) {
        perror("Error occurred while creating the channel");
    }

    while (true) {
        rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), nullptr);
        if (rcvid == -1) break; // Exit if message reception fails

        if (rcvid == 0) { // Handle pulse messages
            processPulse(msg);
            continue;
        }

        switch (msg.hdr.type) {
            case _IO_CONNECT: // Handle connection request
                MsgReply(rcvid, EOK, nullptr, 0);
                break;

            case static_cast<_Uint16t>(HEADER_TYPE::PRIMARY_RADAR):
                // Handle primary radar request
                pthread_mutex_lock(&lock);
                airData.id = id;
                airData.x = x;
                airData.y = y;
                airData.z = z;
                airData.speedX = vx; // Include speed
                airData.speedY = vy;
                airData.speedZ = vz;
                pthread_mutex_unlock(&lock);

                request_msg.aircraft_data = airData;
                MsgReply(rcvid, EOK, &request_msg, sizeof(request_msg));
                break;

            case static_cast<_Uint16t>(HEADER_TYPE::SECONDARY_RADAR):
                // Handle secondary radar request
                processRadarRequest(rcvid, msg, request_msg, airData);
                break;

            case static_cast<_Uint16t>(HEADER_TYPE::COMMAND):
                // Handle operator command
                processCmdRequest(rcvid, msg);
                break;

            default:
                MsgError(rcvid, ENOSYS); // Return error for unrecognized message types
        }
    }

    name_detach(attach, 0); // Detach the channel
    return EXIT_SUCCESS;
}




//Static wrapper used to start aircraft_server() thread
void* Aircraft::startingAircraftServer(void* context) {
    return ((Aircraft*)context)->aircraft_server();
}



void Aircraft::processPulse(const data_t& msg) {
    switch (msg.hdr.code) {
        case _PULSE_CODE_DISCONNECT:
            ConnectDetach(msg.hdr.scoid);
            break;
        case _PULSE_CODE_UNBLOCK:

            break;
        default:

            break;
    }
}

//process secondary radar request
void Aircraft::processRadarRequest(int rcvid, const data_t& msg, data_t& request_msg, aircraft_data_t& airData) {
	pthread_mutex_lock(&lock);

    // Populate aircraftData based on the radar type and existing state
    airData.id = id;
    airData.x = x;
    airData.y = y;
    airData.z = z;
    if (msg.hdr.type == static_cast<_Uint16t>(HEADER_TYPE::SECONDARY_RADAR)) {
        airData.speedX = vx;
        airData.speedY = vy;
        airData.speedZ = vz;
    }
    request_msg.aircraft_data = airData;

    pthread_mutex_unlock(&lock);

    MsgReply(rcvid, EOK, &request_msg, sizeof(request_msg));
}

//process command request
void Aircraft::processCmdRequest(int rcvid, const data_t& msg) {

				aircraft_data_t newAircraft = msg.command_data.newAircraft;

				printf("Aircraft: (%d, %d, %d, %d, %d, %d, %d)\n", id, x, y, z, vx, vy, vz);

				//verify type of command
				if (msg.command_data.command == "CHANGE_SPEED") {
					newVelocity(newAircraft.speedX, newAircraft.speedY, newAircraft.speedZ);
				} else if (msg.command_data.command == "CHANGE_POSITION") {
					newPosition(newAircraft.x, newAircraft.y);
				} else if (msg.command_data.command == "CHANGE_ALTITUDE") {
					newAltitude(newAircraft.z);
				}

				printf("Updated Aircraft: (%d, %d, %d, %d, %d, %d, %d)\n", id, x, y, z, vx, vy, vz);

				MsgReply(rcvid, EOK, 0, 0);
}


