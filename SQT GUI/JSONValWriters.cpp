#include "StdAfx.h"
#include "JSONFormatter.h"
#include "ResponseFormatter.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Output should be : VectorName:[val1,val2....valX]
void WriteUnIndexedNameVector( DataStore *DS, int at, ValStore *Val, char *Out, int MaxLen )
{
    //write header
    sprintf_s( Out, MaxLen, "\"%s\":[", ValStoreGetVariableName( Val->Type ) );

    //parse the DS and translate it into a JSON
    int NodeCount = DS->Count;
    int NameListLength = DS_GetIntValue( Val );
    int ValuesWritten = 0;
    for( int i = at + 1; i < NodeCount && i - at < NameListLength; i++ )
    {
        ValStore  *Val1 = DS_GetValue( DS, i );

        // buget
        if( Val1 == NULL )
            continue;

        //we parsed it wrong
        if( Val1->Type != VT_NODE_LIST_NAME )
            break;

        char *Name = DS_GetStringValue( Val1 );
        //write the new node name
        sprintf_s( Out, MaxLen, "%s\"%s\",", Out, Name );
        ValuesWritten++;
    }

    //add the ending to the JSON value
    if( ValuesWritten == 0 )
        strcpy_s( Out, MaxLen, "" );
    else if( ValuesWritten > 0 )
    {
        //pinch off last character
        Out[ strlen( Out ) - 1 ] = 0;
        //write vector ending
        sprintf_s( Out, MaxLen, "%s]", Out );
    }
}

// output : nothing
void WriteNoValue( DataStore *DS, int at, ValStore *Val, char *Out, int MaxLen )
{
    // we just received a "string" as node name that is part of a vector of values
    // we already handled this value. We skip it in this function
}


// Output should be : "Errors":[{ErrorID:1,Error:"nope"}]
void WriteUnIndexedErrorWithString( DataStore *DS, int at, ValStore *Val, char *Out, int MaxLen )
{
    //check if we have string also
    ValStore *ErrorStringVal = DS_GetValue( DS, at + 1 );
    if( ErrorStringVal == NULL || ErrorStringVal->Type != VT_ERROR_STRING )
    {
        sprintf_s( Out, MaxLen, "\"%s\":%d", ValStoreGetVariableName( Val->Type ), DS_GetIntValue( Val ) );
        return;
    }
    int IsFirstError = 1;
    for( int i = 0; i < at; i++ )
    {
        ValStore *tVal = DS_GetValue( DS, i );
        if( tVal->Type == VT_ERROR_ID )
        {
            IsFirstError = 0;
            break;
        }
    }
    if( IsFirstError == 0 )
        return;

    sprintf_s( Out, MaxLen, "\"Errors\":[" );

    int NodeCount = DS->Count;
    int ValuesWritten = 0;
    for( int i = at; i < NodeCount; i++ )
    {
        ValStore  *Val1 = DS_GetValue( DS, i );
        // buget
        if( Val1 == NULL )
            break;
        //could be a string also
        if( Val1->Type != VT_ERROR_ID )
            continue;

        char *ErrorString = NULL;
        //do we have a matching error string for this ID ?
        ValStore  *Val2 = DS_GetValue( DS, i + 1 );
        //do we have an error string attached to this ID ?
        if( Val2 != NULL && Val2->Type == VT_ERROR_STRING )
            ErrorString = DS_GetStringValue( Val2 );

        //write the value pair to the list of errors
        sprintf_s( Out, MaxLen, "%s{\"%s\":%d", Out, ValStoreGetVariableName( Val1->Type ), DS_GetIntValue( Val ) );
        if( ErrorString == NULL )
            sprintf_s( Out, MaxLen, "%s,\"%s\":\"%s\"},", Out, ValStoreGetVariableName( Val2->Type ), "" );
        else
            sprintf_s( Out, MaxLen, "%s,\"%s\":\"%s\"},", Out, ValStoreGetVariableName( Val2->Type ), DS_GetStringValue( Val2 ) );

        ValuesWritten++;
    }
    if( ValuesWritten == 0 )
        strcpy_s( Out, MaxLen, "" );
    else if( ValuesWritten > 0 )
    {
        //pinch off last character
        Out[ strlen( Out ) - 1 ] = 0;
        //write vector ending
        sprintf_s( Out, MaxLen, "%s]", Out );
    }
}