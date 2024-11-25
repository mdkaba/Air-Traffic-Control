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
