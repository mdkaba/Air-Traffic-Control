#include "CommSystem.h"

//constructor
CommSystem::CommSystem() {}

//destructor
CommSystem::~CommSystem() {}


//Server thread for commuSystem
void* CommSystem::CommSystem_server() {
    name_attach_t* channel_id;
    data_t msg, cmdMsg;
    int rcvid;

    //Initialize timer for a 1-second period
    cTimer timer(1, 0);

    if ((channel_id = name_attach(NULL, COMMUNICATION_SYSTEM_ATTACH, 0)) == NULL) {
        perror("Failed to establish the communication channel");
    }

    while (true) {
        rcvid = MsgReceive(channel_id->chid, &msg, sizeof(msg), NULL);

        //if msg not received
        if (rcvid == -1) {
            break;
        }
        //if msg received
        if (rcvid == 0) {
            switch (msg.hdr.code) {
                case _PULSE_CODE_DISCONNECT:
                    ConnectDetach(msg.hdr.scoid);
                    break;
                case _PULSE_CODE_UNBLOCK:
                    // Handle unblock pulse
                    break;
                default:
                    // Handle other system events
                    break;
            }
            continue;
        }

        // handling connection requests
        if (msg.hdr.type == _IO_CONNECT) {
            MsgReply(rcvid, EOK, NULL, 0);
            continue;
        }

        // rejecting unsupported message types
        if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX) {
            MsgError(rcvid, ENOSYS);
            continue;
        }

        if (msg.hdr.type == static_cast<_Uint16t>(HEADER_TYPE::COMM_SYS)) {

            //Process command messages to aircraft
            int commSysID;
            string channelID = "attach_" + std::to_string(msg.command_data.newAircraft.id);

            if ((commSysID = name_open(channelID.c_str(), 0)) == -1) {
                std::cerr << "Failed to connect to aircraft channel_id\n";
            }
            cmdMsg.hdr.type = static_cast<_Uint16t>(HEADER_TYPE::COMMAND);
            cmdMsg.command_data = msg.command_data;

            if (MsgSend(commSysID, &cmdMsg, sizeof(cmdMsg), NULL, 0) == -1) {
                std::cerr << "Failed to dispatch command to aircraft\n";
            }

            name_close(commSysID);

            //acknowledge the received command
            MsgReply(rcvid, EOK, NULL, 0);
        }
        timer.waitTimer();
    }

    name_detach(channel_id, 0);
    return EXIT_SUCCESS;
}


//Static wrapper used to start the CommSystem_server() thread
void* CommSystem::startCommServer(void* context) {
	return ((CommSystem *)context)->CommSystem_server();
}

// Method to start the CommSystem server thread
void CommSystem::startThreads() {
    // Create and start the CommSystem server thread
    if (pthread_create(&commSystemThreadId, NULL, startCommServer, this) != 0) {
        perror("Failed to create CommSystem server thread");
    }
}

void CommSystem::joinThreads() {
	int err_no = pthread_join(commSystemThreadId, NULL);
	if (err_no != 0){printf("ERROR when joining the thread\n");}
}

