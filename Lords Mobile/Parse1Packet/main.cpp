#include <conio.h>
#include "ParsePackets.h"
#include "CapturePackets.h"
#include "HTTPSendData.h"

void main()
{
//	PacketParserPointer = &ProcessPacket1;
//	ParseOfflineDump("p_good");
//	StartCapturePackets(3);
	HTTPPostData(67, 1, 2, "Tudi", "wib", "sea wolves", 3, 4, 5, 6, 7, 8, 9);
	_getch();
}
