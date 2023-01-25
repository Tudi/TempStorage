#ifndef PROFILE_SERVER_H
#define PROFILE_SERVER_H

#include <stdbool.h>
#include <stdint.h>

// Types

typedef
struct
{
    void* p;
} ScoreServer_t;

// Constants

#define ScoreServer_NULL ((ScoreServer_t) { .p = NULL })

// Functions

ScoreServer_t scoreServer_init();
int scoreServer_run(ScoreServer_t server, 
    const char* companyDirectory,
    const char* industryDirectory,
    const char* titleDirectory,
    const char* profileDirectory,
    uint16_t serverPort, uint32_t requestArrivalTimeout, uint32_t connectionTimeout,
    uint16_t numConnections);
void scoreServer_stop(ScoreServer_t server);
bool scoreServer_isNull(ScoreServer_t server);

#endif // PROFILE_SERVER_H
