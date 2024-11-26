// Group 16 (Mamadou Diao Kaba (27070179), Maika Trepanier (40258910), Lifu Zhang (40081513))
#include "Aircraft.h"
#include "ComputerSystem.h"
#include "DataDisplay.h"
#include "OperatorConsole.h"
#include "Radar.h"
#include <vector>
#include <fstream>
#include "CommSystem.h"
#include "Structures.h"

using namespace std;

int main() {
	vector<Aircraft*> aircraftVector;
	int entryTime, id, x, y, z, speedX, speedY, speedZ;

	//read the file
	ifstream aircraftFile;
//	aircraftFile.open ("lowTraffic.txt");
//	aircraftFile.open ("mediumTraffic.txt");
	aircraftFile.open ("highTraffic.txt");
//	aircraftFile.open ("overloadTraffic.txt");

	if (aircraftFile.is_open()) {
		//read the next line available until the end and push in a vector of aircraft
		while(aircraftFile >> entryTime >> id >> x >> y >> z >> speedX >> speedY >> speedZ) {
			Aircraft * aircraft = new Aircraft (entryTime,id, x, y, z, speedX, speedY, speedZ);
			aircraftVector.push_back(aircraft);
		}
		aircraftFile.close();
	} else {
		printf("File not Opened...Program Terminated!");
		return 1;
	}

	// classes
	Radar* radar = new Radar(aircraftVector);
	ComputerSystem * computerSystem = new ComputerSystem();
	DataDisplay * dataDisplay = new DataDisplay();
	OperatorConsole * operatorConsole = new OperatorConsole();
	CommSystem * commSystem = new CommSystem();



	vector<pthread_t> aircraftThreadIdVector;
	vector<pthread_t> aircraftFlyThreadIdVector;

	for (unsigned int i = 0; i < aircraftVector.size(); i++) {
		pthread_t aircraftThreadId;
		pthread_t aircraftFlyThreadId;
		aircraftThreadIdVector.push_back(aircraftThreadId);
		aircraftFlyThreadIdVector.push_back(aircraftFlyThreadId);
	}

	int err_no;

	// create threads
	for (unsigned int i = 0; i < aircraftVector.size(); i++) {
		if (aircraftVector.at(i)->entryTime == 0) {
			err_no = pthread_create(&aircraftThreadIdVector.at(i), NULL, aircraftVector.at(i)->startingAircraftServer, aircraftVector.at(i));
			if (err_no != 0){printf("ERROR when creating the thread \n");}

			err_no = pthread_create(&aircraftFlyThreadIdVector.at(i), NULL, aircraftVector.at(i)->StartingupdateAircraftPosition, aircraftVector.at(i));
			if (err_no != 0){printf("ERROR when creating the thread \n");}
		}
	}


	 computerSystem->startThreads();
	 commSystem->startThreads();
	 radar->startThreads();
	 dataDisplay->startThreads();
	 operatorConsole->startThreads();

	// any planes which start later (t=1 or after)
	for (unsigned int i = 0; i < aircraftVector.size(); i++) {
		if (aircraftVector.at(i)->entryTime > 0) {
			if (aircraftVector.at(i)->entryTime != aircraftVector.at(i-1)->entryTime){
				sleep(aircraftVector.at(i)->entryTime - aircraftVector.at(i-1)->entryTime);
			}

			err_no = pthread_create(&aircraftThreadIdVector.at(i), NULL, aircraftVector.at(i)->startingAircraftServer, aircraftVector.at(i));
			if (err_no != 0){printf("ERROR when creating the thread \n");}

			err_no = pthread_create(&aircraftFlyThreadIdVector.at(i), NULL, aircraftVector.at(i)->StartingupdateAircraftPosition, aircraftVector.at(i));
			if (err_no != 0){printf("ERROR when creating the thread \n");}
		}
	}

	  // Joining threads
	    radar->joinThreads();
	    computerSystem->joinThreads();
	    dataDisplay->joinThreads();
	    operatorConsole->joinThreads();
	    commSystem->joinThreads();


	for (unsigned int i = 0; i < aircraftVector.size(); i++) {
		err_no = pthread_join(aircraftThreadIdVector.at(i), NULL);
		if (err_no != 0){printf("ERROR when joining the thread\n");}

		err_no = pthread_join(aircraftFlyThreadIdVector.at(i), NULL);
		if (err_no != 0){printf("ERROR when joining the thread\n");}
	}

	// deleting pointers
	delete radar;
	delete computerSystem;
	delete dataDisplay;
	delete operatorConsole;
	delete commSystem;

	for (unsigned int i = 0; i < aircraftVector.size(); i++) {
		delete aircraftVector.at(i);
	}

	return 0;
}
