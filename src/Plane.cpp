
#include "Plane.h"
#include <time.h>
#include <iostream>
#include <csignal>
#include <ctime>
#include <unistd.h> // For sleep()
#include <sys/dispatch.h>

typedef struct {
	unsigned int id;
	char body[200];
} msg_struct;

// Creates the plane and hosts the plane server until exiting the area.
Plane::Plane(int arrivalTime, std::string ID, float X, float Y, float Z, float speedX, float speedY, float speedZ) {
	this->arrivalTime = arrivalTime;
	this->ID = ID;
	this->X = X;
	this->Y = Y;
	this->Z = Z;
	this->speedX = speedX;
	this->speedY = speedY;
	this->speedZ = speedZ;
	//std::cout << toString() << std::endl;
	outOfBounds = false;
	setUpTimer();
	name_attach_t *attach;
	do {
		attach = name_attach(NULL, ID.c_str(), 0);
	} while (attach == NULL);
	//std::cout << "Server is running, waiting for messages..." << std::endl;
	while (true) {
		int rcvid;
		msg_struct msg;
		rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);

		if (rcvid == -1) {
			perror("No msg received");
			continue;
		}

		//std::cout <<"Received message: " << msg.body << std::endl;
		// TODO Add a message for when the plane is not yet into the area
		// (Arrival time not met)
		if (!outOfBounds) {
			msg_struct reply = {msg.id, NULL};
			std::strcpy(reply.body, toString().c_str());
			MsgReply(rcvid, 0, &reply, sizeof(reply));
		} else {
			msg_struct reply = {msg.id, "LEFT_AREA"};
			MsgReply(rcvid, 0, &reply, sizeof(reply));
			break;
		}
	}
	name_detach(attach,0);
}

Plane::~Plane() {
	// TODO Auto-generated destructor stub
}

int Plane::setUpTimer() {
	timer_t timer_id;
	struct sigevent sev;
	struct itimerspec its;

	// Set up the sigevent for the timer
	sev.sigev_notify = SIGEV_THREAD;  // Notify via thread
	sev.sigev_notify_function = &Plane::positionUpdater;  // Function to call on timer expiration
	sev.sigev_value.sival_ptr = this;  // No additional data needed
	sev.sigev_notify_attributes = nullptr;  // Default thread attributes

	// Create the timer
	if (timer_create(CLOCK_REALTIME, &sev, &timer_id) == -1) {
		std::cerr << "Error creating timer: " << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}

	// Set the timer to expire every 1 seconds
	its.it_value.tv_sec = 1;  // First expiration after 1 seconds
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = 1;  // Repeat every 1 seconds
	its.it_interval.tv_nsec = 0;

	// Start the timer
	if (timer_settime(timer_id, 0, &its, nullptr) == -1) {
		std::cerr << "Error setting timer: " << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void Plane::positionUpdater(union sigval sv) {
	Plane* item = (Plane*) sv.sival_ptr;
	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time);
	// TODO Implement arrival time, this doesn't work.
	if (time.tv_sec < item->getArrivalTime()) return;
	item->modifyPositionWithSpeed();
	//std::cout << item->getCoordinatesString() << std::endl;
	item->tester();
}
// TODO update when a plane gets out of bounds
void Plane::tester() {
	if (X < -10 || X > 100010 || Y < 14990 || Y > 40010 || Z < -10 || Z > 100010 ) {outOfBounds = true;}
}
void Plane::modifyPositionWithSpeed() {
	X += speedX;
	Y += speedY;
	Z += speedZ;
}
std::string Plane::getCoordinatesString() {
	std::string str = "Plane " + ID + ", current position: (" + std::to_string(X) + "," + std::to_string(Y) + "," + std::to_string(Z) + ")";
	return str;
}
int Plane::getArrivalTime() {
	return this->arrivalTime;
}

std::string Plane::toString() {
	return "Plane " + ID + ", current position: (" + std::to_string(X) + "," + std::to_string(Y) + "," + std::to_string(Z) + ")"
			+ "\nArrival Time: " + std::to_string(arrivalTime) + ", speed: (" + std::to_string(speedX) + "," + std::to_string(speedY)
			+ "," + std::to_string(speedZ) + ")";
}
