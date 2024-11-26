// Group 16 (Mamadou Diao Kaba (27070179), Maika Trepanier (40258910), Lifu Zhang (40081513))
#ifndef SRC_OPERATORCONSOLE_H_
#define SRC_OPERATORCONSOLE_H_
#include <pthread.h>
#include <fstream> // Make sure to include this for ofstream
#include "Structures.h"

class OperatorConsole {
	pthread_t operatorThreadId;
private:
    std::ofstream writer; // Ensure proper namespace usage

public:
    OperatorConsole();
    virtual ~OperatorConsole();
    static void * startOperatorClient(void *);
    void * operatorClient();
    void logger(data_t msg);
    bool validSpeed(int speedX, int speedY, int speedZ);
    bool validPosition(int x, int y);
    bool validAltitude(int z);
    void startThreads();
    void joinThreads();
};

#endif /* SRC_OPERATORCONSOLE_H_ */
