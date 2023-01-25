#ifndef _DAO_FILE_VERSION_CONVERTERS_H_
#define _DAO_FILE_VERSION_CONVERTERS_H_

#include <daos.h>
#include <item_functions.h>
#include <profile_persistent.h>
#include <company.h>

// Sometimes we might be able to convert a specific DAO file version to a newer file version
int upgradeFileVersion(Daos_t daos, ItemFunctions_t* itemFunctions, const char *inFileName, unsigned int fileVersion);

const uint8_t* binaryToCompany_V7(const uint8_t* byteStream, struct Company* company);
const uint8_t* binaryToProfilePersistent_V7(const uint8_t* byteStream, struct ProfilePersistent* profile);

#endif