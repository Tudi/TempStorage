#ifndef _PROFILE_PERSISTENT_2_H_
#define _PROFILE_PERSISTENT_2_H_

#include "serialize_generic.h"

// always update this in case you add new fields
#define PROFILE_PERSISTENT_WRITER_VERSION 1

// If you add new fields, reorder fields, you will not be able to read old data anymore
typedef enum ProfilePersistentFieldNames
{
	PPFN_ID = GSSFN_MAX_FIELD_NAME,
	PPFN_FIRST_NAME,
	PPFN_LAST_NAME,
	PPFN_LAST_MESSAGED,
	PPFN_MAX_FIELD_NAME
}ProfilePersistentFieldNames;

#endif