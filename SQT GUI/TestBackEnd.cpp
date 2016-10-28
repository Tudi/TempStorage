#include "StdAfx.h"
#include <stdio.h>
#include "ResponseFormatter.h"
#include "OQAPI.h"
#include "JSONFormatter.h"
#include "TestBackEnd.h"

// Run all possible testcases at once
void RunAllAPITestCases()
{
    RunProjectListTest();
    RunNodelistTest();
    RunRestoreFromCDTest();
    RunSQTTest();
    printf( "\nFinished running all testcases \n\n\n");
}

void RunNodelistTest()
{
    printf( "\nNodelist API test\n");

    //get the nodelist response
    DataStore *NodeLIst  = BuildNodeInfoResponse();

    //parse the response
    if( NodeLIst )
    {
        int NodeCount = NodeLIst->Count;
        int NameCounter = 0;
        for( int i = 0; i < NodeCount; i++ )
        {
            ValStore  *Val = DS_GetValue( NodeLIst, i );
            if( Val->Type == VT_NODE_LIST_NAME )
            {
                char      *NodeName = DS_GetStringValue( Val );
                printf( "Node name %d - %s \n", NameCounter++, NodeName );
            }
        }

        //test response in JSON format
        PrintResponseAsJSON( NodeLIst );

        //dispose resource
        Dispose_DS( &NodeLIst );
    }
    else
        printf( "Could not obtain nodelist. Test failed\n" );
}

void RunProjectListTest()
{
    printf( "\nProject List API test\n");

    //get the project response
    DataStore *ProjectList  = BuildProjectInfoResponse();

    //parse the response
    if( ProjectList )
    {
        //test response in JSON format
        PrintResponseAsJSON( ProjectList );

        //dispose resource
        Dispose_DS( &ProjectList );
    }
    else
        printf( "Could not obtain project list. Test failed\n" );
}

void RunRestoreFromCDTest()
{
    printf( "\n Restore form CD API test\n");

    //get the restore from CD response
    DataStore *RestoreRet  = RestoreProjectsFromCD();

    //parse the response
    if( RestoreRet )
    {
        //test response in JSON format
        PrintResponseAsJSON( RestoreRet );

        //dispose resource
        Dispose_DS( &RestoreRet );
    }
    else
        printf( "API did not respond. Test failed\n" );
}

void RunSQTTest()
{
    printf( "\n Perform IQ/OQ/OQMVM API test\n");

    //get the nodelist response
    DataStore *NodelistRet  = BuildNodeInfoResponse();

    // bail out ?
    if( NodelistRet == NULL )
    {
        printf( "BuildProjectInfoResponse API did not respond. Test failed\n" );
        return;
    }

    // this is a test, we want to monitor what is happening in the background
    PrintResponseAsJSON( NodelistRet );

    //parse the response and build a new packet to be passed for QO/IQ
    DataStore *IQOQparam = NULL;
    int NodeCount = NodelistRet->Count;
    int NameCounter = 0;
    int CanDoIQ = 0;        // can this node perform this operation ?
    int CanDoOQ = 0;        // can this node perform this operation ?
    int CanDoOQMMV = 0;     // can this node perform this operation ?
    for( int i = 0; i < NodeCount; i++ )
    {
        //fetech next value from response buffer
        ValStore  *Val = DS_GetValue( NodelistRet, i );
        char *Name = DS_GetStringValue( Val );
        if( Val->Type == VT_NODE_LIST_NAME )
        {
            // this is where we build a list of nodes we want to perform operations on
            IQOQparam = DS_PackToResult( IQOQparam, VT_NODE_PERFORM_NAME, strlen( Name ) + 1, Name );
            IQOQparam = DS_PackToResult( IQOQparam, VT_NODE_PERFORM_IQ, (char)1 );
            IQOQparam = DS_PackToResult( IQOQparam, VT_NODE_PERFORM_OQ, (char)1 );
            IQOQparam = DS_PackToResult( IQOQparam, VT_NODE_PERFORM_OQMVM, (char)1 );
            NameCounter++;
        }
        else if( Val->Type == VT_CAN_DO_IQ )
            CanDoIQ = DS_GetByteValue( Val );
        else if( Val->Type == VT_CAN_DO_OQ )
            CanDoOQ = DS_GetByteValue( Val );
        else if( Val->Type == VT_CAN_DO_OQMVM )
            CanDoOQMMV = DS_GetByteValue( Val );
    }

    if( CanDoIQ == 0 && CanDoOQ == 0 && CanDoOQMMV == 0 )
    {
        printf( "Test failed. Node can not perform any of the following IQ/OQ/OQMV. Please restore some projects and retry\n" );
        //we could try to restore the projects form the CD at this point. But that already failed in the previous test ;p
//        DataStore *RestoreRet  = RestoreProjectsFromCD();
        return;
    }

    if( NameCounter == 0 )
    {
        printf("Test failed. Could not find any node names to perform IQ/OQ/OQMVM on \n");
        return;
    }

    // this is a test, we want to monitor what is happening in the background
    PrintResponseAsJSON( IQOQparam );

    //perform IQ/OQ
    DataStore *SQTResponse  = PerformSQT(IQOQparam);
    PrintResponseAsJSON( SQTResponse );

    //dispose resource
    Dispose_DS( &NodelistRet );
    Dispose_DS( &IQOQparam );
    Dispose_DS( &SQTResponse );
}
