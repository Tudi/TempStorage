#include "StdAfx.h"
#include "OQ_HTTP_API.h"
#include "Microhttp\microhttpd.h"
#include "ResponseFormatter.h"
#include "OQAPI.h"
#include "JSONFormatter.h"

// forward declarations
int HandleProjectList( struct MHD_Connection *connection );
int HandleProjectRestore( struct MHD_Connection *connection );
int HandleNodeList( struct MHD_Connection *connection );
int HandleSQT( struct MHD_Connection *connection );

static int http_send_data( struct MHD_Connection *connection, const char *Data, int Len )
{
  int ret;
  struct MHD_Response *response;

  response = MHD_create_response_from_buffer (Len, (void *) Data, MHD_RESPMEM_PERSISTENT);
  if (!response)
    return MHD_NO;

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}

int HandleURL( struct MHD_Connection *connection, const char *Url )
{
    if( strcmp( Url, "/ProjectList" ) == 0 )
        return HandleProjectList( connection );
    else if( strcmp( Url, "/Restore" ) == 0 )
        return HandleProjectRestore( connection );
    else if( strcmp( Url, "/NodeList" ) == 0 )
        return HandleNodeList( connection );
    else if( strcmp( Url, "/PerformSQT" ) == 0 )
        return HandleSQT( connection );

    return 0;
}

int HandleProjectList( struct MHD_Connection *connection )
{
    //get the project response
    DataStore *ProjectList  = BuildProjectInfoResponse();

    //parse the response
    if( ProjectList == NULL )
        return 1;

    char *JSONresp = ResponseToString( ProjectList );
    if( JSONresp == NULL )
    {
        Dispose_DS( &ProjectList );
        return 1;
    }

    int ret = http_send_data( connection, JSONresp, strlen( JSONresp ) );

    //dispose resource
    Dispose_DS( &ProjectList );
//    Dispose_JSON( &JSONresp );    // will be disposed
    return ret;
}

int HandleProjectRestore( struct MHD_Connection *connection )
{
    //get the project response
    DataStore *RestoreResp  = RestoreProjectsFromCD();

    //parse the response
    if( RestoreResp == NULL )
        return 1;

    char *JSONresp = ResponseToString( RestoreResp );
    if( JSONresp == NULL )
    {
        Dispose_DS( &RestoreResp );
        return 1;
    }

    int ret = http_send_data( connection, JSONresp, strlen( JSONresp ) );

    //dispose resource
    Dispose_DS( &RestoreResp );
//    Dispose_JSON( &JSONresp );    // will be disposed
    return ret;
}

int HandleNodeList( struct MHD_Connection *connection )
{
    //get the project response
    DataStore *NodeList  = BuildNodeInfoResponse();

    //parse the response
    if( NodeList == NULL )
        return 1;

    char *JSONresp = ResponseToString( NodeList );
    if( JSONresp == NULL )
    {
        Dispose_DS( &NodeList );
        return 1;
    }

    int ret = http_send_data( connection, JSONresp, strlen( JSONresp ) );

    //dispose resource
    Dispose_DS( &NodeList );
//    Dispose_JSON( &JSONresp );    // will be disposed
    return ret;
}

char *CopyJSONVal( char *src, char *dst )
{
    while( *src != ',' && *src != '}' && *src != 0 )
        *dst++ = *src++;
    *dst = 0;

    return src;
}
/*
// !! Disabled this until JSON parsing is properly implemented
int HandleSQT( struct MHD_Connection *connection )
{
    char *NodeList = (char *)MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, "NodeList");
    if( NodeList == NULL )
        return 0;

    DataStore *IQOQparam = NULL;

    //we should be using a JSON parser here...
    // !!! rewrite it later !!!

    // repeating structure of : nodename, DoIQ, DoOQ, DoOQMVM
    char TempValStore[300];
    char NodeName[300]; //this is not safe at all
    int DoIQ, DoOQ, DoOQMVM;
    NodeList++; //skip '{'
    while( *NodeList != 0 )
    {
        NodeList = CopyJSONVal( NodeList, NodeName );

        NodeList++;
        NodeList = CopyJSONVal( NodeList, TempValStore );
        DoIQ = atoi( TempValStore );

        NodeList++;
        NodeList = CopyJSONVal( NodeList, TempValStore );
        DoOQ = atoi( TempValStore );

        NodeList++;
        NodeList = CopyJSONVal( NodeList, TempValStore );
        DoOQMVM = atoi( TempValStore );

        NodeList++;

        IQOQparam = DS_PackToResult( IQOQparam, VT_NODE_PERFORM_NAME, strlen( NodeName ) + 1, NodeName );
        IQOQparam = DS_PackToResult( IQOQparam, VT_NODE_PERFORM_IQ, (char)1 );
        IQOQparam = DS_PackToResult( IQOQparam, VT_NODE_PERFORM_OQ, (char)1 );
        IQOQparam = DS_PackToResult( IQOQparam, VT_NODE_PERFORM_OQMVM, (char)1 );
    }

    //get the project response
    DataStore *SQTResp  = PerformSQT( IQOQparam );

    //parse the response
    if( SQTResp == NULL )
        return 1;

    char *JSONresp = ResponseToString( SQTResp );
    if( JSONresp == NULL )
    {
        Dispose_DS( &SQTResp );
        return 1;
    }

    int ret = http_send_data( connection, JSONresp, strlen( JSONresp ) );

    //dispose resource
    Dispose_DS( &SQTResp );
//    Dispose_JSON( &JSONresp );    // will be disposed
    return ret;
}
*/

int HandleSQT( struct MHD_Connection *connection )
{
    char *NodeName = (char *)MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, "NodeName");
    if( NodeName == NULL )
        return 0;
    char *DoIQ = (char *)MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, "DoIQ");
    if( DoIQ == NULL )
        return 0;
    char *DoOQ = (char *)MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, "DoOQ");
    if( DoOQ == NULL )
        return 0;
    char *DoOQMVM = (char *)MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, "DoOQMVM");
    if( DoOQMVM == NULL )
        return 0;

    int Temp = atoi( DoIQ );
    DataStore *IQOQparam = NULL;
    IQOQparam = DS_PackToResult( IQOQparam, VT_NODE_PERFORM_NAME, strlen( NodeName ) + 1, NodeName );
    Temp = atoi( DoIQ );
    IQOQparam = DS_PackToResult( IQOQparam, VT_NODE_PERFORM_IQ, (char)Temp );
    Temp = atoi( DoOQ );
    IQOQparam = DS_PackToResult( IQOQparam, VT_NODE_PERFORM_OQ, (char)Temp );
    Temp = atoi( DoOQMVM );
    IQOQparam = DS_PackToResult( IQOQparam, VT_NODE_PERFORM_OQMVM, (char)Temp );

    //get the project response
    DataStore *SQTResp  = PerformSQT( IQOQparam );

    //parse the response
    if( SQTResp == NULL )
        return 1;

    char *JSONresp = ResponseToString( SQTResp );
    if( JSONresp == NULL )
    {
        Dispose_DS( &SQTResp );
        return 1;
    }

    int ret = http_send_data( connection, JSONresp, strlen( JSONresp ) );

    //dispose resource
    Dispose_DS( &SQTResp );
//    Dispose_JSON( &JSONresp );    // will be disposed
    return ret;
}
