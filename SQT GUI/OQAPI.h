#pragma once

/*
    Describe possible return values from function "RestoreProjectsFromCD"
*/
enum OQAPI_ErrorCodes
{
    // RestoreProjectsFromCD
    R_CD_NO_ERROR   = 0,
    R_CD_INSERT_CD_WITH_PROJECTS ,
    R_CD_DO_NOT_USE_CD_FOR_RESTORE ,
    R_CD_RESTORE_FAILED ,
    R_CD_RESTORE_SUCCESS ,
    R_CD_COULD_NOT_FIND_CD_DRIVE,
    R_CD_SQL_CONNECTION_FAILED,
    // PerformSQT
    P_SQT_CAN_NOT_PERFORM_ACTIONS,
    P_SQT_SUCCESS,
    P_SQT_NODENAME,
    P_SQT_IQ_SUCCESS,
    P_SQT_OQ_SUCCESS,
    P_SQT_OQMVM_SUCCESS,
    P_SQT_IQ_NO_DATA,
    P_SQT_OQ_NO_DATA,
    P_SQT_OQMVM_NO_DATA,
    P_SQT_IQ_NO_PROCESS,
    P_SQT_OQ_NO_PROCESS,
    P_SQT_OQMVM_NO_PROCESS,
    P_SQT_NO_PROCSRV,
};

// We will define this later in some other header
struct DataStore;

/*
Brief : Build a list of nodes on which we can try to perform IQ/OQ/OQMVM
return : values represent which project are available for SQT
*/
DataStore *BuildProjectInfoResponse();
/*
Brief : Build a list of nodes on which we can try to perform IQ/OQ/OQMVM
return : list of strings representing node names
*/
DataStore *BuildNodeInfoResponse();
/*
Brief : try to find CD drive. Check drive if it contains projects to restore.
return : RestoreFromCDReturnCodes codes
*/
DataStore *RestoreProjectsFromCD();
/*
Brief : Parse a list of nodes received in param and try to perform the requested operations on them
return : ErrorID 
*/
DataStore * PerformSQT(DataStore *NodeList);
