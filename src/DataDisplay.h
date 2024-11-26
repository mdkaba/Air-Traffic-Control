// Group 16 (Mamadou Diao Kaba (27070179), Maika Trepanier (40258910), Lifu Zhang (40081513))
#ifndef SRC_DATADISPLAY_H_
#define SRC_DATADISPLAY_H_
#include <pthread.h>
class DataDisplay {
	pthread_t displayThreadId;
public:
DataDisplay();
virtual ~DataDisplay();
void * dataDisplay_client();
static void * startingDataDisplayClient(void *);
void startThreads();
void joinThreads();
};

#endif /* SRC_DATADISPLAY_H_ */
