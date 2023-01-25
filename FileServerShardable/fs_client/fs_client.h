#ifndef PROFILE_CLIENT_H
#define PROFILE_CLIENT_H

#include <daos_definitions.h>
#include <request_response_definitions.h>
#include <stdint.h>

int connectToServer(const char* serverAddress, uint16_t serverPort, int* sockt);
int endConnectionToServer(int sockt, uint8_t* responseCode);

int getFileFromServer(int sockt, uint8_t entryType, uint32_t entryId, uint8_t* responseCode, FSPacketSaveFile** file,
    uint32_t* receivedfileSize);
int sendFileToServer(int sockt, uint8_t entryType, uint32_t entryId, const char* file, uint32_t fileSize,
    uint8_t* responseCode);
int pingServer(int sockt, uint8_t* responseCode);

#endif // PROFILE_CLIENT_H
