#include <conio.h>
#include <stdio.h>
#include "ParsePackets.h"
#include "CapturePackets.h"
#include "HTTPSendData.h"

void main()
{
//	PacketParserPointer = &ProcessPacket1;
//	ParseOfflineDump("p_good");
//	HTTPPostData(67, 1, 2, "Tudi", "wib", "sea wolves", 3, 4, 5, 6, 7, 8, 9);

	//parse network packets and intiate a http-post to insert them into the DB
	CreateBackgroundPacketProcessThread(); 

	// listen to network interface, assemble packets, queue them to the process queue
	StartCapturePackets(3);

	printf("Waiting for packets to come and process\n. Press any key to exit");
	_getch();

	//shut everything down
	StopCapturePackets();
	StopThreadedPacketParser();
	printf("Properly shut everything down. Exiting\n");
}
