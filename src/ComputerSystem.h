#ifndef COMPUTER_SYSTEM_H
#define COMPUTER_SYSTEM_H

#include <pthread.h>
#include <mutex>
#include <vector>
#include <string>
#include <sys/neutrino.h> // For MsgReceive, MsgSend, name_attach, name_detach, etc.
#include "Structures.h" // Assuming this header contains necessary type definitions and constants

// Assuming the definitions of aircraft_data_t, data_t, and other types are provided in "defs.h" or similar
// If not, you'll need to define these structures or include their definitions

class ComputerSystem {
    pthread_t computerSystemThreadId, computerSystemLoggerThreadId, collisionSystemThreadId;

public:
    ComputerSystem();
    ~ComputerSystem();

    void updatingAircraftInfo(const aircraft_data_t& airData);
    void removeOutofRangeAircraft(const aircraft_data_t& airData);
    void* collisionMonitor();
    static void* startingCollisionMonitor(void* context);

    void calculateForCollision(const std::vector<aircraft_data_t>& tempVector, int time);

    void* computerSystem_server();
    static void* startingComputerSystemServer(void* context);

    void sendDataToCommSys(const aircraft_data_t& aircraft, const std::string& command);
    void* CSLog();
    static void* startingCSLog(void* context);
    void startThreads();
    void joinThreads();

private:
    pthread_mutex_t mutex;
    std::vector<aircraft_data_t> aircraftVector;
    int n;


};

#endif // COMPUTER_SYSTEM_H
