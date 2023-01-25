#ifndef PROFILE_CLIENT_H
#define PROFILE_CLIENT_H

#include <stdint.h>

int connectToServer(const char* serverAddress, uint16_t serverPort, int* sockt);
int endConnectionToServer(int sockt, uint32_t* responseCode);
int getScoresFromServer(int sockt, void* requestPacket,
    uint32_t requestPacketSize, void** responsePacket, uint32_t *responsePacketSize);
int sendScoreFileToServer(int sockt, uint32_t scoreType, uint32_t scoreId, const char* scoreFile, uint32_t scoreFileSize,
    uint32_t* responseCode);
int pingServer(int sockt, uint32_t* responseCode);

#endif // PROFILE_CLIENT_H
