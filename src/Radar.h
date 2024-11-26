// Group 16 (Mamadou Diao Kaba (27070179), Maika Trepanier (40258910), Lifu Zhang (40081513))
#ifndef RADAR_H
#define RADAR_H

#include <vector>
#include <pthread.h>
#include "Aircraft.h"
#include "Structures.h"

class Radar {

	 pthread_t primaryThreadId, secondaryThreadId;

public:
    // Constructor and Destructor
    Radar(std::vector<Aircraft*> aircraftVector);
    ~Radar();


    static void* startingPrimaryRadarCLient(void* context);

    static void* startingSecondaryRadarClient(void* context);

    bool ifAirisInBound(aircraft_data_t aircraft);
    void startThreads();
    void joinThreads();

private:

    std::vector<Aircraft*> airVector;
    std::vector<aircraft_data_t> radarAircraft;
    pthread_mutex_t mutex;


    void* primaryRadar_client();
    void* secondaryRadar_client();

};

#endif // RADAR_H
