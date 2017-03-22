#include <conio.h>
#include <stdio.h>
#include "ParsePackets.h"
#include "CapturePackets.h"
#include "HTTPSendData.h"
#include "LordsMobileControl.h"
#include <Windows.h>

void OfflineParsing()
{
//	HttpSendStartup();
//	ProcessPacketDebug("AC 08 17 05 00 4F 28 08 00 54 75 64 69 36 39 00 00 00 00 00 00 00 23 42 4C 43 00 13 03 2D 14 03 47 8F 24 D2 58 00 00 00 00 C3 00 00 00 00 00 00 00 00 00 00 00 ");
	ParseOfflineDump("tel_click_fields_lvl2");
	//wait for the HTTP queue to finish
	while (IsHTTPQueueEmpty() == 0)
		Sleep(10);
	return;
}

void OnlineScanParsing()
{
	//move the screen, close popups
	LordsMobileControlStartup();

	HttpSendStartup();
	//	HTTPPostData(67, 1, 2, "Tudi", "wib", "sea wolves", 3, 4, 5, 6, 7, 8, 9);

	//parse network packets and intiate a http-post to insert them into the DB
	CreateBackgroundPacketProcessThread();

	// listen to network interface, assemble packets, queue them to the process queue
	StartCapturePackets(3);
}

void main()
{
	OfflineParsing();
//	OnlineScanParsing();

	printf("Waiting for packets to come and process\n. Press 'a' key to exit");
	char AKey = ' ';
	while (AKey != 'a')
		scanf_s("%c", &AKey, sizeof(AKey));

	//shut everything down
	StopCapturePackets();
	StopThreadedPacketParser();
	HttpSendShutdown();
	LordsMobileControlShutdown();
	printf("Properly shut everything down. Exiting\n");
}
