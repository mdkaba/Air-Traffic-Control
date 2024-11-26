// Group 16 (Mamadou Diao Kaba (27070179), Maika Trepanier (40258910), Lifu Zhang (40081513))
#include <sys/dispatch.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "cTimer.h"
using namespace std;

#ifndef SRC_DEFS_H_
#define SRC_DEFS_H_

#define COMPUTER_SYSTEM_ATTACH "comp_sys_attach"
#define COMMUNICATION_SYSTEM_ATTACH "comm_sys_attach"

typedef struct _pulse msg_header_t;

typedef struct aircraft_data {
	int entryTime;
	int id;
	int x;
	int y;
	int z;
	int speedX;
	int speedY;
	int speedZ;
	bool aircraftInBounds;
	string attachPoint;
} aircraft_data_t;

typedef struct display_data {
	int augmentedInfoAircraftId;
	vector<aircraft_data_t> aircraftDataVector;
} display_data_t;

typedef struct command_data {
	aircraft_data_t newAircraft;
	string command;
	bool validId;
	int n;
} command_data_t;

typedef struct data {
	msg_header_t hdr;
	aircraft_data_t aircraft_data;
	display_data_t display_data;
	command_data_t command_data;
} data_t;


enum class HEADER_TYPE : uint16_t {
	PRIMARY_RADAR = 0x00,
	SECONDARY_RADAR = 0x01,
	COMP_PRIMARY = 0x03,
	COMP_SECONDARY = 0x04,
	DATA_DISPLAY = 0x05,
	COMM_SYS = 0x06,
	COMMAND = 0x07,
	OPERATOR_COMMAND = 0x08,
	OPERATOR_REQUEST_INFO = 0x09,
	OPERATOR_PREDICTION_TIME = 0x0A,
	OPERATOR_VALID_ID = 0x0B
};

#endif /* SRC_DEFS_H_ */
