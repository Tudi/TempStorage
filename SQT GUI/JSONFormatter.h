#pragma once

struct DataStore;
struct ValStore;

char *ResponseToString( DataStore *in );
void Dispose_JSON( char **Buf );

// get a string for a specific variable name. If it exists
char *ValStoreGetVariableName(int type);

// List of value writers for structures or vectors
void WriteUnIndexedNameVector( DataStore *in, int at, ValStore *Val, char *Out, int MaxLen );
void WriteUnIndexedErrorWithString( DataStore *in, int at, ValStore *Val, char *Out, int MaxLen );
void WriteNoValue( DataStore *in, int at, ValStore *Val, char *Out, int MaxLen );

void PrintResponseAsJSON( DataStore *in );