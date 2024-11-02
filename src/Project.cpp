#include <iostream>
#include <sstream>
#include "Plane.h"
using namespace std;

void* createPlane(void* arg);

int main() {
	// Rectangle from 0,15000,0 to 100000,40000,100000
	// Time,ID,X,Y,Z,SpeedX,SpeedY,SpeedZ

	string examples[4] = {
			"10,1,0,15000,0,15,0,15",
			"10,2,100000,40000,0,-20,-5,5",
			"15,3,0,40000,100000,15,-15,15",
			"20,4,100000,15000,100000,-25,10,-5",
	};

	for (int i = 0; i < 4; i++) {
		pthread_t thread_id1;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		std::cout << "Creating thread now." << std::endl;
		if (pthread_create(&thread_id1, &attr, &createPlane, (void *) &examples[i])) {
			std::cout << "Failed to create thread1" << std::endl;
			pthread_attr_destroy(&attr); // Clean up
			return EXIT_FAILURE;
		}
	}
	cout << "Plane init completed" << endl;
	while(true);
	return 0;
}

void* createPlane(void* arg) {
	string* val = (string*) arg;
	string input[8];
	stringstream ss(*val);
	for (int i = 0; i < 8; i++) {
		getline(ss, input[i], ',');
	}
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	int arrivalTime = t.tv_nsec + 1000*stoi(input[0]);
	Plane* plane = new Plane(arrivalTime, input[1], stof(input[2]), stof(input[3]), stof(input[4]), stof(input[5]), stof(input[6]), stof(input[7]));
	return nullptr;
}


