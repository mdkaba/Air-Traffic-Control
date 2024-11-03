#include <iostream>
#include <sstream>
#include "Plane.h"
#include <unistd.h> // For sleep()
#include <sys/dispatch.h>

using namespace std;

const char LEFT_AREA[200] = "LEFT_AREA";
// Message structure
typedef struct {
	unsigned int id;
	char body[200];
} msg_struct;

void* createPlane(void* arg);

int main() {
	// Rectangle from 0,15000,0 to 100000,40000,100000
	// Time,ID,X,Y,Z,SpeedX,SpeedY,SpeedZ
	// To replace with reading from a file

	string examples[4] = {
			"10,1,0,15000,0,15,5,15",
			"10,2,100000,40000,0,-20,-5,5",
			"15,3,0,40000,100000,15,-15,-15",
			"20,4,100000,15000,100000,-25,10,-5",
	};

/*
	// To test when all threads left
	// TODO fix issue when using these examples
	string examples[4] = {
			"10,1,0,15000,999960,15,5,15",
			"10,2,100000,15020,0,-20,-5,5",
			"15,3,0,40000,45,15,-15,-15",
			"20,4,100000,39990,100000,-25,10,-5",
	};
*/
	pthread_t thread_id[4];
	pthread_attr_t attr[4];
	string IDs[4];
	volatile int activePlaneCount = 4;
	// Create a thread for each plane
	for (int i = 0; i < 4; i++) {
		pthread_attr_init(&attr[i]);
		pthread_attr_setdetachstate(&attr[i], PTHREAD_CREATE_JOINABLE);
		std::cout << "Creating thread now." << std::endl;
		if (pthread_create(&thread_id[i], &attr[i], &createPlane, (void *) &examples[i])) {
			std::cout << "Failed to create thread1" << std::endl;
			pthread_attr_destroy(&attr[i]); // Clean up
			return EXIT_FAILURE;
		}
	}
	cout << "Plane init completed" << endl;
	// Get the ID for each server
	for (int i = 0; i < 4; i++) {
		stringstream ss(examples[i]);
		getline(ss, IDs[i], ',');
		getline(ss, IDs[i], ',');
	}
	// Simulating the radar on the main thread
	// The radar is the client, plane objects (threads)
	// each have their own server with ID = plane ID.
	while(activePlaneCount > 0) {
		// Loop through the ID list
		for (int i = 0; i < 4; i++) {
			if (IDs[i] == "") continue;
			cout << activePlaneCount << endl;
			// Connect to the server
			int coid = name_open(IDs[i].c_str(), 0);
			if (coid == -1) continue;
			msg_struct msg;
			msg.id = std::stoi(IDs[i]);
			// Send message identify to a plane
			strcpy(msg.body, "IDENTIFY");
			cout << "IDENTIFY message sent to " << IDs[i] << endl;
			msg_struct reply;
			int status = MsgSend(coid, &msg, sizeof(msg), &reply, sizeof(reply));
			if (status == -1) {
				cout << "Failed to send the message" << endl;
			}
			// If plane is still in the area they'll send information
			if (strcmp(reply.body, LEFT_AREA) != 0) {
				// Display received information
				cout << "Reply : " << reply.body << endl;
			} else {
				// If area left, stop pinging that plane.
				cout << "LEAVE AREA ACKNOWLEDGED" << endl;
				activePlaneCount--;
				IDs[i] = "";
				// When no planes are left end the program.
				if (activePlaneCount < 1) break;
			}
			// Close the connection with the plane and sleep before
			// polling the next one.
			name_close(coid);
			sleep(1);
		}
	}
	return 0;
}
// Creates a thread splitting all items in the input and creates a plane.
// The constructor function of the plane contains the server and calls
// the periodic update of the coordinates with a timer and another thread.
void* createPlane(void* arg) {
	string* val = (string*) arg;
	string input[8];
	stringstream ss(*val);
	for (int i = 0; i < 8; i++) {
		getline(ss, input[i], ',');
	}
	//delete &ss;
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	int arrivalTime = t.tv_nsec + 1000*stoi(input[0]);
	Plane* plane = new Plane(arrivalTime, input[1], stof(input[2]), stof(input[3]), stof(input[4]), stof(input[5]), stof(input[6]), stof(input[7]));
	return nullptr;
}


