#ifndef DAOS_DEFINITIONS_H
#define DAOS_DEFINITIONS_H

#include <stdint.h>

#define DAOS_ID_TO_NETWORK htonl
#define NETWORK_TO_DAOS_ID ntohl

#define DAOS_COUNT_TO_NETWORK htonl
#define NETWORK_TO_DAOS_COUNT ntohl

typedef uint16_t DaosFileVersion_t;
typedef int32_t DaosId_t;
typedef uint32_t DaosOffset_t;
typedef uint32_t DaosSize_t;
typedef uint32_t DaosCount_t;

#endif // DAOS_DEFINITIONS_H
