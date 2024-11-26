// Group 16 (Mamadou Diao Kaba (27070179), Maika Trepanier (40258910), Lifu Zhang (40081513))
#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <string>
#include <pthread.h>
#include "Structures.h"

class Aircraft {
public:
    // Constructor and Destructor
    Aircraft(int time, int id, int x, int y, int z, int vx, int vy, int vz);
    ~Aircraft();
    int entryTime;
    std::string attachPoint;
    //wrapper
    static void* StartingupdateAircraftPosition(void* context);
    static void* startingAircraftServer(void* context);

private:
    //aircraft variables
    int id;
    int x, y, z;
    int vx, vy, vz;

	pthread_mutex_t lock;

    void* updateAircraftPosition();
    void* aircraft_server();
    void newVelocity(int newVX, int newVY, int newVZ);
    void newPosition(int newX, int newY);
    void newAltitude(int newAltitude);


    void processPulse(const data_t& msg);
    void processRadarRequest(int rcvid, const data_t& msg, data_t& request_msg, aircraft_data_t& aircraftData);
    void processCmdRequest(int rcvid, const data_t& msg);

    void startThreads();
};

#endif // AIRCRAFT_H
