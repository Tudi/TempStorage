#include "../StdAfx.h"
#include "QtCore/QFileInfo.h"
#include "GenericServer.h"
#include <iostream>
#include "NetworkPacket.h"
#include <cassert>
#include <time.h>
#include "../GetBiosUUID.h"
#include "../Tools.h"

void Server::PH_QueryUUID()
{
	//check what counter we should be at for this specific VM
	UUIDQueryReplyPacket ReplyPacket;
	UUIDQueryPacket *QueryPacket = (UUIDQueryPacket *)IncommingPacket->GetData();
	int UUIDHash = crc32((const unsigned char*)QueryPacket->UUID, sizeof(QueryPacket->UUID));
	std::map<int,int>::iterator itr = UUIDCounterStatus.find(UUIDHash);
	if (itr == UUIDCounterStatus.end() || QueryPacket->RequestCounter == 1)
	{
		DEBUG_SSL_PRINT(std::cout << "New session started for UUID query " << std::endl;);
		UUIDCounterStatus[UUIDHash] = QueryPacket->RequestCounter + 1;
		GetBiosUUID((unsigned char*)ReplyPacket.UUID, sizeof(ReplyPacket.UUID));
	}
	else if (itr != UUIDCounterStatus.end() && (*itr).second != QueryPacket->RequestCounter)
	{
		DEBUG_SSL_PRINT(std::cout << "Deny reply for UUID query. Have counter " << (*itr).second << " Received " << QueryPacket->RequestCounter << std::endl;);
	}
	else
	{
		DEBUG_SSL_PRINT(std::cout << "Continue session for UUID query " << std::endl;);
		(*itr).second = QueryPacket->RequestCounter + 1;
		//only return a valid reply if there is only a single instance of UUID service running on this PC
		GetBiosUUID((unsigned char*)ReplyPacket.UUID, sizeof(ReplyPacket.UUID));
	}

	QSslSocket *socket = dynamic_cast<QSslSocket *>(sender());
	ReplyPacket.PacketSize = sizeof(ReplyPacket);
	ReplyPacket.PacketStamp = time(NULL);
	ReplyPacket.PacketType = PACKET_TYPE_UUID_REPLY;
	DEBUG_SSL_PRINT(std::cout << "Sent UUID" << std::endl;);
	DEBUG_SSL_PRINT(for (int i = 0; i < 16; i++)printf("%02X", ReplyPacket.UUID[i]); printf("\n"););
	FragmentedNetworkPacket::SendPacket(socket, (char*)&ReplyPacket, sizeof(ReplyPacket));

}