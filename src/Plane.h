/*
 * Plane.h
 *
 *  Created on: Nov. 1, 2024
 *      Author: maika
 */

#ifndef PLANE_H_
#define PLANE_H_
#include <string>
class Plane {
public:
	Plane(int, std::string, float, float, float, float, float, float);
	void modifyPositionWithSpeed();
	std::string getCoordinatesString();
	void tester();
	int getArrivalTime();
	std::string toString();
	virtual ~Plane();
private:
	int arrivalTime;
	std::string ID;
	float X;
	float Y;
	float Z;
	float speedX;
	float speedY;
	float speedZ;
	bool outOfBounds;
	static void positionUpdater(union sigval sv);
	int setUpTimer();
};

#endif /* PLANE_H_ */
