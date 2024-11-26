// Group 16 (Mamadou Diao Kaba (27070179), Maika Trepanier (40258910), Lifu Zhang (40081513))
#include <pthread.h>
#include "Structures.h"
#ifndef SRC_COMMSYSTEM_H_
#define SRC_COMMSYSTEM_H_

class CommSystem {
	pthread_t commSystemThreadId;
public:
CommSystem();
virtual ~CommSystem();
void * CommSystem_server();
static void * startCommServer(void *);
void startThreads();
void joinThreads();
};

#endif /* SRC_COMMSYSTEM_H_ */
