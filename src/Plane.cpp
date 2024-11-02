/*
 * Plane.cpp
 *
 *  Created on: Nov. 1, 2024
 *      Author: maika
 */

#include "Plane.h"
#include <time.h>
#include <iostream>
#include <csignal>
#include <ctime>
#include <unistd.h> // For sleep()

Plane::Plane(int arrivalTime, std::string ID, float X, float Y, float Z, float speedX, float speedY, float speedZ) {
	this->arrivalTime = arrivalTime;
	this->ID = ID;
	this->X = X;
	this->Y = Y;
	this->Z = Z;
	this->speedX = speedX;
	this->speedY = speedY;
	this->speedZ = speedZ;
	std::cout << toString() << std::endl;
	outOfBounds = false;
	setUpTimer();
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

	// Keep the program running until the timer completes
	// Wait indefinitely until the program exits from countdown_handler
	while (!outOfBounds) {
		sleep(1); // Sleep to let the timer work
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
	std::cout << item->getCoordinatesString() << std::endl;
	item->tester();
}
void Plane::tester() {
	//if (X < 0 || X > 100000 || Y < 15000 || X > 40000 || Z < 0 || Z > 100000 ) {outOfBounds = true; delete this;}
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
