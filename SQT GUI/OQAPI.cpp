#include "stdafx.h"
#include "sqlconct.h"
#include "Services.h"
#include "DbChrom.h"
#include "WorkDial.h"
#include "dbproj.h"
#include "MillCmgr.h"
#include "Mfs.h"
#include "DbSysPol.h"
#include "DbValStudy.h"
#include "MsgBoard.h"
#include "projctxt.h"
#include "ProcSrv.h"

#include "OQAPI.h"
#include "ResponseFormatter.h"

const char* Project1 = "Valid_OQ1";
const char* Project2 = "Valid_OQApex"; // Set to "" to only support project1
const char* Project3 = "Valid_OQMVM";  // Set to "" to not support project 3

//this needs to get moved to localized resource
char *GetErrorAsString( int ErrorID )
{
    if( ErrorID == R_CD_NO_ERROR || ErrorID == R_CD_RESTORE_SUCCESS )
        return "All ok";
    else if( ErrorID == R_CD_INSERT_CD_WITH_PROJECTS )
        return "No CD drive found";
    else if( ErrorID == R_CD_INSERT_CD_WITH_PROJECTS )
        return "No project found on CD";
    else if( ErrorID == P_SQT_CAN_NOT_PERFORM_ACTIONS )
        return "Unable to perform SQT actions";
    else if( ErrorID == P_SQT_SUCCESS )
        return "SQT API exited. See individual statuses";
    else
        return "Error";
}

/*
    Connect to SQL database to be able to manage a specific project
*/
int SQLLogOnForProject( CString Project )
{
    CString schema;
    CString Service;
    Environment::ComputerName( Service );
    if( SqlConnection::SqlLogOn( Service, "Millennium", NULL) != OK )
        return ERROR_RET;

    // if we only wanted to make sure we want to connect to SQL
    if( Project.GetLength() == 0 )
        return OK;

    int SchemaResult = Project::GetSchema( schema, Project );

    //make sure we get a valid shema for this project
    if( SchemaResult != OK || schema.GetLength() == 0 )
        return ERROR_RET;

    // Done on the Millennium DB
    return SqlConnection::SqlLogOn( Service, schema, NULL );
}

/*
    Restore a specific project from the project CD ( or drive )
*/
int RestoreSpecificProjectFromCD( char *CDroot, const char *ProjectName, CString &service )
{
    BSTR newProj;
    BSTR newUser;
    double newQuota;
    newProj = SysAllocString((BSTR)"NoProject");
    newUser = SysAllocString((BSTR)"NoUser");
    CString tmpService;
    CString tmp;
    Environment::ComputerName(tmp);
    if (tmp.CompareNoCase(service) != 0)
        tmpService = service;

    // Now restore the second project
    CString path = CDroot;
    path += "\\";
    path += ProjectName;
    if (ReadInfFile(path, &newProj, &newUser, &newQuota) == CON_OK)
    {
        if (RestoreProjectSetup(tmpService, ProjectName, "System", path, newQuota) == CON_OK)
        {
            if (RestoreProject(tmpService, ProjectName, "System", path, newQuota) == CON_OK)
            {
                Sleep(1000);
                while(OperationInProgress() != CON_OK)
                    Sleep(5000);
      
                if (OperationStatus() != CON_OK)
                    return R_CD_RESTORE_FAILED;
            }
            else
                return R_CD_RESTORE_FAILED;
        }
        else
            return R_CD_RESTORE_FAILED;
    }
    else
        return R_CD_INSERT_CD_WITH_PROJECTS;

    return R_CD_RESTORE_SUCCESS;
}

/*
    Restore OQ projects from the first CD drive
*/
int RestoreProjectsFromCD_()
{
    // First go looking for a CD drive
    char CDroot[] = "A:";
    bool foundCD = false;
    for (int d = 'A'; d <= 'Z'; d++)
    {
        CDroot[0] = d;
        UINT type = GetDriveType(CDroot);
        if( type == DRIVE_CDROM )
        {
            foundCD = true;
            break;
        }
    }

    // If I can put a CD in, see if the user wants to
    if (foundCD)
    {
        CString Service;

        int ConnectionResult = SqlConnection::SqlLogOn( Service, "Millennium", NULL);
        if( ConnectionResult != OK )
            return R_CD_SQL_CONNECTION_FAILED;

        MStringArray projects;
        Project::Names( projects, NULL );

        int RestoreResult1 = R_CD_RESTORE_SUCCESS;
        int RestoreResult2 = R_CD_RESTORE_SUCCESS;
        int RestoreResult3 = R_CD_RESTORE_SUCCESS;

        // If I don't have project1
        if (!projects.FindNoCase( Project1 ) )
            RestoreResult1 = RestoreSpecificProjectFromCD( CDroot, Project1, Service );
        // If I don't have project2
        if ( !projects.FindNoCase( Project2 ) && SysPolicy::ApexTrackAllowed() )
            RestoreResult2 = RestoreSpecificProjectFromCD( CDroot, Project2, Service );
        // If I don't have project3
        if ( !projects.FindNoCase( Project3 ) && Environment::HasOption( OPT_IQOQ_MVM ) )
            RestoreResult3 = RestoreSpecificProjectFromCD( CDroot, Project3, Service );

        //in case one of the restores failed we signal it. Some of the projects might have still got restored
        if( RestoreResult1 != R_CD_RESTORE_SUCCESS )
            return RestoreResult1;
        if( RestoreResult1 != R_CD_RESTORE_SUCCESS )
            return RestoreResult2;
        if( RestoreResult1 != R_CD_RESTORE_SUCCESS )
            return RestoreResult3;

        return RestoreResult1;
    }
    else
        return R_CD_COULD_NOT_FIND_CD_DRIVE;

    return R_CD_NO_ERROR;
}

DataStore *RestoreProjectsFromCD()
{
    DataStore *Response = NULL;
    int RestoreRet = RestoreProjectsFromCD_();

    Response = DS_PackToResult( Response, VT_ERROR_ID, 4, &RestoreRet );

    char *ErrorAsString = GetErrorAsString( RestoreRet );
    Response = DS_PackToResult( Response, VT_ERROR_STRING, strlen( ErrorAsString ) + 1, ErrorAsString );

    return Response;
}

//Ids ssIds1IQ;   // IQ SampleSets for project1
//Ids ssIds1OQ;   // OQ SampleSets for project1
//Ids ssIds1;     // SampleSets for project1
//Ids ssIds2;     // SampleSets for project2
//Ids stIds3;     // Studies for project2
void GetSSIDs( Ids &ssIds1IQ, Ids &ssIds1OQ, Ids &ssIds1, Ids &ssIds2, Ids &stIds3 )
{
    CString schema;
    CString Service;

    Project::GetSchema( schema, Project1 );
    // Done on the Millennium DB
    SqlConnection::SqlLogOn( Service, schema, NULL );

    CString ssName;
    ssIds1IQ.RemoveAll();
    ssIds1OQ.RemoveAll();

    SampleSetDB::ListOfSSIds( ssIds1 );

    for(int i = 0; i < ssIds1.GetSize(); i++)
    {                   
        SampleSetDB::GetName(ssIds1[i], ssName);
        if (ssName == "IQ")
            ssIds1IQ.Add( ssIds1[i] );
        else
            ssIds1OQ.Add( ssIds1[i] );
    }

    // Get project2's ssIds
    Project::GetSchema(schema, Project2 );
    SqlConnection::SqlLogOff();
    SqlConnection::SqlLogOn( Service, schema, NULL);
    SampleSetDB::ListOfSSIds( ssIds2 );

    // Get project3's stIds (Study Ids)
    Project::GetSchema( schema, Project3 );
    SqlConnection::SqlLogOn( Service, schema, NULL );
    CString Name = " Study";
    Name = Name + Project3;
    ValStudy::ListOfStudyIds( stIds3, Name );

    // Done on the Project DB
    SqlConnection::SqlLogOff();
} 

DataStore *BuildNodeInfoResponse()
{
    DataStore *Response = BuildProjectInfoResponse();

    if( SQLLogOnForProject( "" ) != OK )
        return Response;

    NodeInfos aNodes;

    //fetch all members from passed group from the databse
    NodeInfo::FetchAll( aNodes );

    ValStore *NameCounter;
    Response = DS_PackToResult( Response, VT_NODE_LIST_COUNT, 1, NULL, &NameCounter );

    //for all nodes
    for (int i=0; i < aNodes.GetCount(); i++)
        if ( aNodes.GetAt(i)->nodeType != CITRIX )
        {
            CString Name = aNodes.GetAt(i)->node;
            Response = DS_PackToResult( Response, VT_NODE_LIST_NAME, Name.GetLength() + 1, Name.GetString() );
            //inside our response buffer, increase the counter
            NameCounter->Value++;
        }

    // Done on the Project DB
    SqlConnection::SqlLogOff();

    return Response;
}

DataStore *BuildProjectInfoResponse()
{
    DataStore *Response = NULL;

    if( SQLLogOnForProject( "" ) != OK )
        return NULL;

    MStringArray projects;
    Project::Names( projects, NULL );

    int CanDoIQ = 1;
    int CanDoOQ = 1;
    int CanDoOQMVM = 1;

    if ( !projects.FindNoCase(Project1) )
        CanDoIQ = 0;
    if ( !projects.FindNoCase(Project2) || !SysPolicy::ApexTrackAllowed() )
        CanDoOQ = 0;
    if ( !projects.FindNoCase(Project3) || !Environment::HasOption( OPT_IQOQ_MVM ) )
        CanDoOQMVM = 0;

    Response = DS_PackToResult( Response, VT_CAN_DO_IQ, 1, &CanDoIQ );
    Response = DS_PackToResult( Response, VT_CAN_DO_OQ, 1, &CanDoOQ );
    Response = DS_PackToResult( Response, VT_CAN_DO_OQMVM, 1, &CanDoOQMVM );

    // Done on the Project DB
    SqlConnection::SqlLogOff();

    return Response;
}

DataStore * PerformSQT(DataStore *NodeList)
{
    DataStore *Response = NULL;

    if( SQLLogOnForProject( "" ) != OK )
        return NULL;

    char *User="System";
    MStringArray projects;
    Project::Names( projects, NULL );

    int CanDoIQ = 1;
    int CanDoOQ = 1;
    int CanDoOQMVM = 1;

    if ( !projects.FindNoCase(Project1) )
        CanDoIQ = 0;
    if ( !projects.FindNoCase(Project2) || !SysPolicy::ApexTrackAllowed() )
        CanDoOQ = 0;
    if ( !projects.FindNoCase(Project3) || !Environment::HasOption( OPT_IQOQ_MVM ) )
        CanDoOQMVM = 0;

    if( CanDoIQ == 0 && CanDoOQ == 0 && CanDoOQMVM == 0 )
    {
        Response = DS_PackToResult( Response, VT_ERROR_ID, (int)P_SQT_CAN_NOT_PERFORM_ACTIONS );
        char *ErrorAsString = GetErrorAsString( P_SQT_CAN_NOT_PERFORM_ACTIONS );
        Response = DS_PackToResult( Response, VT_ERROR_STRING, strlen( ErrorAsString ) + 1, ErrorAsString );
        return Response;
    }

    Ids ssIds1IQ;   // IQ SampleSets for project1
    Ids ssIds1OQ;   // OQ SampleSets for project1
    Ids ssIds1;     // SampleSets for project1
    Ids ssIds2;     // SampleSets for project2
    Ids stIds3;     // Studies for project2
    GetSSIDs( ssIds1IQ, ssIds1OQ, ssIds1, ssIds2, stIds3 );
    bool anyErrors;

    //try to init Processing Server
    HRESULT CoInitRes = CoInitialize(NULL);
    if( FAILED( CoInitRes ) ) 
    {
        CoUninitialize();
        Response = DS_PackToResult( Response, VT_ERROR_ID, (int)P_SQT_NO_PROCSRV );
        char *ErrorAsString = GetErrorAsString( P_SQT_NO_PROCSRV );
        Response = DS_PackToResult( Response, VT_ERROR_STRING, strlen( ErrorAsString ) + 1, ErrorAsString );
        return Response;
    }

    //check if we can log in for the projects we will use
    if( SQLLogOnForProject( Project1 ) != OK )
        return NULL;
    if( SQLLogOnForProject( Project2 ) != OK )
        return NULL;
    if( SQLLogOnForProject( Project3 ) != OK )
        return NULL;

    ProcessingServer pServer;

    for( int i = 0; i < NodeList->Count; i++ )
    {
        CString currentNode;
        BOOL doIQ = false;
        BOOL doOQ = false;
        BOOL doOQMVM = false;
        int  AllDataPresent = 0;
        //gather info what kind of IQ/OQ/OQMVMV the user would want to do ( later we will check if it is doable )
        ValStore *NodeInfo = DS_GetValue( NodeList, i );
        if( NodeInfo->Type == VT_NODE_PERFORM_NAME )
        {
            currentNode = DS_GetStringValue( NodeInfo );
            i++;
            NodeInfo = DS_GetValue( NodeList, i );
            AllDataPresent++;
        }
        if( NodeInfo->Type == VT_NODE_PERFORM_IQ )
        {
            doIQ = DS_GetByteValue( NodeInfo );
            i++;
            NodeInfo = DS_GetValue( NodeList, i );
            AllDataPresent++;
        }
        if( NodeInfo->Type == VT_NODE_PERFORM_OQ )
        {
            doOQ = DS_GetByteValue( NodeInfo );
            i++;
            NodeInfo = DS_GetValue( NodeList, i );
            AllDataPresent++;
        }
        if( NodeInfo->Type == VT_NODE_PERFORM_OQMVM )
        {
            doOQMVM = DS_GetByteValue( NodeInfo );
            i++;
            NodeInfo = DS_GetValue( NodeList, i );
            AllDataPresent++;
        }
        //malformed packet detected ? Are we out of sync ? :O
        if( AllDataPresent != 4 )
            continue;

        //try to connect to the node
        if ( pServer.Connect(currentNode) != OK )
        {
            anyErrors = true;
            Response = DS_PackToResult( Response, VT_ERROR_ID, (int)P_SQT_NODENAME );
            char ErrorAsString[ 3000 ];
            sprintf_s( ErrorAsString, sizeof( ErrorAsString ), "Could not connect to node %s", currentNode );
            Response = DS_PackToResult( Response, VT_ERROR_STRING, strlen( ErrorAsString ) + 1, ErrorAsString );
            continue;
        }
        {
              ProcSrvReq req;
              CString service;
              Environment::ComputerName(service);
              req.service = service;
              req.user = User;
              req.action = ProcSrvReq::QUEUE;
              req.processing = SampleSetLine::NORMAL;
              // Request the IQ
              if (doIQ)
              {
                  if (ssIds1IQ.GetSize() <= 0)
                  {
                      anyErrors = true;
                        Response = DS_PackToResult( Response, VT_ERROR_ID, (int)P_SQT_IQ_NO_DATA );
                        char ErrorAsString[ 3000 ];
                        sprintf_s( ErrorAsString, sizeof( ErrorAsString ), "Node %s IQ does not have any Sample Set IDs", currentNode );
                        Response = DS_PackToResult( Response, VT_ERROR_STRING, strlen( ErrorAsString ) + 1, ErrorAsString );
                  }
                  else
                  {
                      // Complete the specific request
                      req.project = Project1;
                      req.idType = DBC_SAMPLESET;
                      req.doProcessing = TRUE;
                      req.doIntegrate = TRUE;
                      req.doReporting = FALSE;
                      req.processMethodSrc = ProcSrvReq::ACQUISITION_METHOD_SET;
                      req.reportMethodSrc = ProcSrvReq::IQ_REPORT;
                      // only process the first IQ sample set
	                    req.ids.Add(ssIds1IQ[0]);
                      NodeInfo::ClearIQ(currentNode);
                      
                      req.reportMethodName = "A";

                      if (pServer.Execute(req) != OK)
                      {
                        Response = DS_PackToResult( Response, VT_ERROR_ID, (int)P_SQT_IQ_NO_PROCESS );
                        char ErrorAsString[ 3000 ];
                        sprintf_s( ErrorAsString, sizeof( ErrorAsString ), "Node %s could not process IQ request", currentNode );
                        Response = DS_PackToResult( Response, VT_ERROR_STRING, strlen( ErrorAsString ) + 1, ErrorAsString );
                      }
                      else 
                      {
                        Response = DS_PackToResult( Response, VT_ERROR_ID, (int)P_SQT_IQ_SUCCESS );
                        char ErrorAsString[ 3000 ];
                        sprintf_s( ErrorAsString, sizeof( ErrorAsString ), "Node %s IQ was successful", currentNode );
                        Response = DS_PackToResult( Response, VT_ERROR_STRING, strlen( ErrorAsString ) + 1, ErrorAsString );
                      }
                  }
              }
              // Request the OQ
              if (doOQ)
              {
                  // Check the list of SampleSets in the OQ project
                  if (ssIds1OQ.GetSize() <= 0)
                  {
                      anyErrors = true;
                        Response = DS_PackToResult( Response, VT_ERROR_ID, (int)P_SQT_OQ_NO_DATA );
                        char ErrorAsString[ 3000 ];
                        sprintf_s( ErrorAsString, sizeof( ErrorAsString ), "Node %s OQ does not have any Sample Set 1 IDs", currentNode );
                        Response = DS_PackToResult( Response, VT_ERROR_STRING, strlen( ErrorAsString ) + 1, ErrorAsString );
                  }
                  else
                  {
                      // Complete the specific request
                      req.project = Project1;
                      req.idType = DBC_SAMPLESET;
                      req.doProcessing = TRUE;
                      req.doIntegrate = TRUE;
                      req.processMethodSrc = ProcSrvReq::ACQUISITION_METHOD_SET;
                      req.doReporting = FALSE;
                      req.reportMethodSrc = ProcSrvReq::ACQUISITION_METHOD_SET;
                      // Set this weird thing to signal procsrv to turn on SysSuit option for this job
                      req.exportMethodSrc = ProcSrvReq::IQ_REPORT;
                      req.ids = ssIds1OQ;
                                            
                      NodeInfo::ClearOQ(currentNode);
                      
                      // Process them all
                      if (pServer.Execute(req) != OK)
                      {
                          anyErrors = true;
                        Response = DS_PackToResult( Response, VT_ERROR_ID, (int)P_SQT_OQ_NO_PROCESS );
                        char ErrorAsString[ 3000 ];
                        sprintf_s( ErrorAsString, sizeof( ErrorAsString ), "Node %s could not process OQ request 1", currentNode );
                        Response = DS_PackToResult( Response, VT_ERROR_STRING, strlen( ErrorAsString ) + 1, ErrorAsString );
                      }
                      else
                      {
                          // Now handle the 2nd project (ApexTrack)
//                          if (!Project2.IsEmpty())
                          {
                              // Check the list of SampleSets in the OQ project2
                              if (ssIds2.GetSize() <= 0)
                              {
                                  anyErrors = true;
                                Response = DS_PackToResult( Response, VT_ERROR_ID, (int)P_SQT_OQ_NO_DATA );
                                char ErrorAsString[ 3000 ];
                                sprintf_s( ErrorAsString, sizeof( ErrorAsString ), "Node %s OQ does not have any Sample Set 2 IDs", currentNode );
                                Response = DS_PackToResult( Response, VT_ERROR_STRING, strlen( ErrorAsString ) + 1, ErrorAsString );
                              }
                              else
                              {
                                  // Complete the specific request
                                  req.project = Project2;
                                  req.idType = DBC_SAMPLESET;
                                  req.doProcessing = TRUE;
                                  req.doIntegrate = TRUE;
                                  req.processMethodSrc = ProcSrvReq::ACQUISITION_METHOD_SET;
                                  req.doReporting = FALSE;
                                  req.reportMethodSrc = ProcSrvReq::ACQUISITION_METHOD_SET;
                                  // Set this weird thing to signal procsrv to turn on SysSuit option for this job
                                  req.exportMethodSrc = ProcSrvReq::IQ_REPORT;
                                  req.ids = ssIds2;

                                  // Process them all
                                  if (pServer.Execute(req) != OK)
                                  {
                                      anyErrors = true;
                                        Response = DS_PackToResult( Response, VT_ERROR_ID, (int)P_SQT_OQ_NO_PROCESS );
                                        char ErrorAsString[ 3000 ];
                                        sprintf_s( ErrorAsString, sizeof( ErrorAsString ), "Node %s could not process OQ request 2", currentNode );
                                        Response = DS_PackToResult( Response, VT_ERROR_STRING, strlen( ErrorAsString ) + 1, ErrorAsString );
                                  }
                                  else
                                  {
                                        Response = DS_PackToResult( Response, VT_ERROR_ID, (int)P_SQT_OQ_SUCCESS );
                                        char ErrorAsString[ 3000 ];
                                        sprintf_s( ErrorAsString, sizeof( ErrorAsString ), "Node %s OQ was successful", currentNode );
                                        Response = DS_PackToResult( Response, VT_ERROR_STRING, strlen( ErrorAsString ) + 1, ErrorAsString );
                                  }
                              }
                          }
                      }
                  }
              }

              // Request the OQ for MVM
              if (doOQMVM)
              {
                  // Now handle the 3rd project (Method Validation)
//                  if (!Project3.IsEmpty())
                  {
                      // Check the list of Studies in the OQ project3
                      if (stIds3.GetSize() <= 0)
                      {
                          anyErrors = true;
                            Response = DS_PackToResult( Response, VT_ERROR_ID, (int)P_SQT_OQMVM_NO_DATA );
                            char ErrorAsString[ 3000 ];
                            sprintf_s( ErrorAsString, sizeof( ErrorAsString ), "Node %s OQMVM does not have any Sample Set 3 IDs", currentNode );
                            Response = DS_PackToResult( Response, VT_ERROR_STRING, strlen( ErrorAsString ) + 1, ErrorAsString );
                      }
                      else
                      {
                          // Complete the specific request
                          req.project = Project3;
                          req.idType = DBC_VALIDATIONSTUDY;
                          req.doProcessing = TRUE;
                          req.doIntegrate = TRUE;
                          req.processMethodSrc = ProcSrvReq::ACQUISITION_METHOD_SET;
                          req.doReporting = FALSE;
                          req.reportMethodSrc = ProcSrvReq::METHOD;
                          req.reportMethodName = "OQMVM Summary Report";
                          // Set this weird thing to signal procsrv to turn on SysSuit option for this job
                          req.exportMethodSrc = ProcSrvReq::IQ_REPORT;
                          req.ids = stIds3;

                          NodeInfo::ClearOQMVM(currentNode);

                          // Process them all
                          if (pServer.Execute(req) != OK)
                          {
                              anyErrors = true;
                            Response = DS_PackToResult( Response, VT_ERROR_ID, (int)P_SQT_OQ_NO_PROCESS );
                            char ErrorAsString[ 3000 ];
                            sprintf_s( ErrorAsString, sizeof( ErrorAsString ), "Node %s could not process OQMVM request", currentNode );
                            Response = DS_PackToResult( Response, VT_ERROR_STRING, strlen( ErrorAsString ) + 1, ErrorAsString );
                          }
                          else 
                          {
                                Response = DS_PackToResult( Response, VT_ERROR_ID, (int)P_SQT_OQMVM_SUCCESS );
                                char ErrorAsString[ 3000 ];
                                sprintf_s( ErrorAsString, sizeof( ErrorAsString ), "Node %s OQMVM was successful", currentNode );
                                Response = DS_PackToResult( Response, VT_ERROR_STRING, strlen( ErrorAsString ) + 1, ErrorAsString );
                          }
                      }
                  }
              }
              pServer.Disconnect();
        }
    }

    //cleanup
    CoUninitialize();

    //All ok response
    if( anyErrors == false )
    {
        Response = DS_PackToResult( Response, VT_ERROR_ID, (int)P_SQT_SUCCESS );
        char *ErrorAsString = GetErrorAsString( P_SQT_SUCCESS );
        Response = DS_PackToResult( Response, VT_ERROR_STRING, strlen( ErrorAsString ) + 1, ErrorAsString );
    }

    return Response;
}
