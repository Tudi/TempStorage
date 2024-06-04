#include "stdafx.h"
#include <ctime>

WebApiErrorCodes ExtractDBColumnToBinary::DBH_APIResultValid(int CurlErr, char* response, yyjson_doc* &jsonResponse, LogSourceGroups sg, const char* func)
{
    CURLcode err = (CURLcode)CurlErr;
    if (err != CURLE_OK || response == NULL)
    {
        auto srcGroupStrView = magic_enum::enum_name<LogSourceGroups>(sg);
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityUnexpected, sg, 0, 0,
            "%s:%s: Failed to fetch DB data.", srcGroupStrView.data(), func);
        return WebApiErrorCodes::WAE_CouldNotReachEndpoint;
    }
    // empty response string ? All good
    if (response[0] == 0)
    {
        return WebApiErrorCodes::WAE_EmptyResponse;
    }

    // parse response
    jsonResponse = yyjson_read(response, strlen(response), 0);
    yyjson_val* root = yyjson_doc_get_root(jsonResponse);
    yyjson_val* ErrorId = yyjson_obj_get(root, "ErrorId");

    // check for errors
    if (ErrorId != NULL)
    {
        int errorCode = yyjson_get_int(ErrorId);
        if (errorCode == SharedErrorCodesWithBackend::EC_BANNED)
        {
            sAppSession.SessionIdInit(0);
            return WebApiErrorCodes::WAE_LogClientBanned;
        }
        if (errorCode == SharedErrorCodesWithBackend::EC_INVALID_SESSION)
        {
            sAppSession.SessionIdInit(0);
            return WebApiErrorCodes::WAE_InvalidSession;
        }
        if (errorCode == SharedErrorCodesWithBackend::EC_INVALID_USER)
        {
            sWindowManager.OnUserLoggeOut();
            sAppSession.SessionIdInit(0);
            sUserSession.SetUserId(0);
            return WebApiErrorCodes::WAE_InvalidUser;
        }
        if (errorCode == SharedErrorCodesWithBackend::EC_INACTIVE_USER)
        {
            sAppSession.SessionIdInit(0);
            return WebApiErrorCodes::WAE_InactiveUser;
        }
        if (errorCode == SharedErrorCodesWithBackend::EC_SESSION_EXPIRED)
        {
            sAppSession.SessionIdInit(0);
            return WebApiErrorCodes::WAE_SessionExpired;
        }
        if (errorCode != SharedErrorCodesWithBackend::EC_NO_ERROR)
        {
            auto srcGroupStrView = magic_enum::enum_name<LogSourceGroups>(sg);
            AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, sg, 0, 0,
                "%s:%s:Failed to fetch DB data. Error %d:%s", srcGroupStrView.data(), func,
                errorCode, GetSharedErrorCodeString((SharedErrorCodesWithBackend)errorCode));
            return WebApiErrorCodes::WAE_UnknowLogError;
        }
    }

    return WebApiErrorCodes::WAE_NoError;
}

void ExtractDBColumnToBinary::DBH_ParseDBRowFromJSON(yyjson_doc* jsonResponse, const char* DataGroupName,
    ExtractDBColumnToBinary* colArr, LogSourceGroups sg)
{
    yyjson_val* root = yyjson_doc_get_root(jsonResponse);
    yyjson_val* yyDataGroupName = yyjson_obj_get(root, DataGroupName);

    if (yyDataGroupName == NULL || yyjson_is_arr(yyDataGroupName) == 0)
    {
        auto srcGroupStrView = magic_enum::enum_name<LogSourceGroups>(sg);
        AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, sg, 0, 0,
            "%s:No DB data is available ", srcGroupStrView.data());
        return;
    }

    if (colArr[0].cbInit != NULL)
    {
        int elemCount = (int)yyjson_arr_size(yyDataGroupName);
        // set row count
        colArr[0].cbInit(elemCount, colArr);
    }

    int rowsAdded = 0;
    size_t idx, maxIdx;
    yyjson_val* yyRow;
    yyjson_arr_foreach(yyDataGroupName, idx, maxIdx, yyRow)
    {
        // add each column to the table
        bool bFullRowParsed = true;
        for (uint32_t colIndex = 0; colArr[colIndex].DBCol != NULL; colIndex++)
        {
            const char* colName = colArr[colIndex].DBCol;
            yyjson_val* yyRowCol = yyjson_obj_get(yyRow, colName);
            if (yyRowCol == NULL)
            {
                auto srcGroupStrView = magic_enum::enum_name<LogSourceGroups>(sg);
                AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, sg, 0, 0,
                    "%s:Missing expected DB col value %s", srcGroupStrView.data(), colName);
                bFullRowParsed = false;
                continue;
            }

            if (yyjson_is_str(yyRowCol) == false)
            {
                colArr[colIndex].SetVal("", rowsAdded); // maybe 'null' value
            }
            else
            {
                bool addRes = colArr[colIndex].SetVal(yyjson_get_str(yyRowCol), colIndex, rowsAdded);
                if (addRes == false)
                {
                    bFullRowParsed = false;
                    auto srcGroupStrView = magic_enum::enum_name<LogSourceGroups>(sg);
                    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, sg, 0, 0,
                        "%s:Missing expected form field for DB col name %s", srcGroupStrView.data(), colName);
                    continue;
                }
            }
        }

        // if we are parsing multiple rows, call the callback function
        if (bFullRowParsed &&
            colArr[0].cbDataRowFinished != NULL &&
            colArr[0].cbDataRowFinished(rowsAdded, colArr) == true)
        {

        }
        // only parse 1 row unless it's a multi row result store
        else
        {
//            break;
        }

        // increase store index
        rowsAdded++;
    }
}

bool ExtractDBColumnToBinary::SetVal(const char* val, int colIndex, int rowsAdded)
{
    if (bStoredAsReference)
    {
        colValReadOnly = val;
    }

    char* sFormattedVal;
    if (fmtFnc != NULL)
    {
        sFormattedVal = fmtFnc(val, colIndex, rowsAdded, this);
    }
    else
    {
        sFormattedVal = (char*)val;
    }

    if (FormField != NULL)
    {
        FormField->ResetState(sFormattedVal);
    }
    else if (FormStrField != NULL)
    {
        strcpy_s(FormStrField, FormStrFieldSize, sFormattedVal);
    }
    else if (GridCol != 0xFFFF && customDestinationObj != NULL)
    {
        GenericDataGrid* dataGrid = typecheck_castL(GenericDataGrid, customDestinationObj);
        size_t nStoredSize;
        // some formatter functions may generate custom sized objects
        if (this->nSoredDataHasSpecificSize != 0)
        {
            nStoredSize = this->nSoredDataHasSpecificSize;
        }
        else
        {
            // generic case we use strings
            nStoredSize = strlen(sFormattedVal) + 1;
        }
        // store the blob of data
        dataGrid->SetData(GridCol, (int)rowsAdded, sFormattedVal, nStoredSize);
    }
    else if (ullNumericData != NULL)
    {
        *ullNumericData = atoll(sFormattedVal);
    }
    else if (cNumericData != NULL)
    {
        *cNumericData = (unsigned char)atoi(sFormattedVal);
    }
    else if (sFormattedVal == NULL)
    {
        // do nothing. Our custom store function took care of it
    }
    else
    {
        return false;
    }

    // if this is a temp formatted value
    if (sFormattedVal != val && 
        sFormattedVal != AllocOnceBufferForValFormat &&
        bDoNotDeallocFormattedString == false)
    {
        InternalFree(sFormattedVal);
    }

    return true;
}

char* AlertStatusIdToStr(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef)
{
    colIndex; rowsIndex; colDef;
    int nVal = atoi(_DBVal);
    char* ret = (char*)sLocalization.GetAlertStateIdString(nVal);
    colDef->bDoNotDeallocFormattedString = true;
    return ret;
}

char* TimeStampToStr(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef)
{
    colIndex; rowsIndex; colDef;
    time_t nVal = atoll(_DBVal);

    struct tm timeinfo;
    const size_t buffSize = 90;
    char *buffer = colDef->AllocOnceBufferForValFormat;

    localtime_s(&timeinfo, &nVal); // Convert to local time

    strftime(buffer, buffSize, "%m/%d/%Y %I:%M:%S %p", &timeinfo);

    return buffer;
}

char* TimeStampToSortableStr(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef)
{
    colIndex; rowsIndex; colDef;
    time_t nVal = atoll(_DBVal);

    struct tm timeinfo;
    const size_t buffSize = 90;
    char* buffer = colDef->AllocOnceBufferForValFormat;

    localtime_s(&timeinfo, &nVal); // Convert to local time

    strftime(buffer, buffSize, "%Y/%m/%d %H:%M:%S", &timeinfo);

    return buffer;
}

char* TimeStampToStrAlertCard(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef)
{
    colIndex; rowsIndex; colDef;
    time_t nVal = atoll(_DBVal);

    struct tm timeinfo;
    const size_t buffSize = 90;
    char* buffer = colDef->AllocOnceBufferForValFormat;

    localtime_s(&timeinfo, &nVal); // Convert to local time

    strftime(buffer, buffSize, "%H:%M, %e %b", &timeinfo);

    return buffer;
}

char* TimeStampToDateStr(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef)
{
    colIndex; rowsIndex; colDef;
    time_t nVal = atoll(_DBVal);

    struct tm timeinfo;
    const size_t buffSize = 90;
    char* buffer = colDef->AllocOnceBufferForValFormat;

    localtime_s(&timeinfo, &nVal); // Convert to local time

    strftime(buffer, buffSize, "%m/%d/%Y", &timeinfo);

    return buffer;
}

char* TimeStampToTimeStr(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef)
{
    colIndex; rowsIndex; colDef;
    time_t nVal = atoll(_DBVal);

    struct tm timeinfo;
    const size_t buffSize = 90;
    char* buffer = colDef->AllocOnceBufferForValFormat;

    localtime_s(&timeinfo, &nVal); // Convert to local time

    strftime(buffer, buffSize, "%H:%M:%S ", &timeinfo);

    return buffer;
}

char* LogToMsgOnly(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef)
{
    colIndex; rowsIndex; colDef;
    const char* msgStart = _DBVal;

    // find the start of the actual message in a log message
    for (size_t i = 0; i < 7; i++)
    {
        while (msgStart[0] != 0 && msgStart[0] != ':')
        {
            msgStart++;
        }
        if (msgStart[0] != 0)
        {
            msgStart++;
        }
    }

    // eeek, we don't know this Log format
    if (msgStart[0] == 0)
    {
        return (char*)_DBVal;
    }

    // duplicate the str part
    const size_t buffSize = strlen(msgStart) + 1;
    char* buffer;
    if (buffSize < sizeof(colDef->AllocOnceBufferForValFormat))
    {
        buffer = colDef->AllocOnceBufferForValFormat;
    }
    else
    {
        buffer = (char*)InternalMalloc(buffSize);
    }
    if (buffer == NULL)
    {
        return NULL;
    }

    memcpy(buffer, msgStart, buffSize);

    return buffer;
}

char* ModuleStatusIdToStr(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef)
{
    colIndex; rowsIndex; colDef;
    int nVal = atoi(_DBVal);
    char* ret = (char*)sLocalization.GetModuleStatusTypeIdString(nVal);
    colDef->bDoNotDeallocFormattedString = true;
    return ret;
}

void InitDatagridToStoreRows(size_t rowCount, ExtractDBColumnToBinary* colDef)
{
    if (colDef == NULL || colDef->customDestinationObj == NULL)
    {
        return;
    }
    GenericDataGrid* dstObj = typecheck_castL(GenericDataGrid, colDef->customDestinationObj);
    dstObj->SetSize(0, (uint32_t)rowCount);
}

void InitDropdownToStoreRows(size_t rowCount, ExtractDBColumnToBinary* colDef)
{
    if (colDef == NULL || colDef->customDestinationObj == NULL)
    {
        return;
    }
    GenericDropdown* dstObj = (GenericDropdown*)colDef->customDestinationObj;
    dstObj->SetSize((uint32_t)rowCount);
}

char* GridStrValToUint64(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef)
{
    colIndex; rowsIndex; colDef;
    int64_t nVal = _atoi64(_DBVal);
    int64_t* ret = (int64_t*)colDef->AllocOnceBufferForValFormat;
    *ret = nVal;
    colDef->nSoredDataHasSpecificSize = sizeof(int64_t);
    return (char*)ret;
}

char* StrToCustomObjFloat(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef)
{
    colIndex; rowsIndex;
    assert(colDef->CustomDataSize >= sizeof(float));
    char* endp;
    *(float*)colDef->CustomData = strtof(_DBVal, &endp);
    return NULL;
}

char* StrToStdStr(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef)
{
    colIndex; rowsIndex;
    assert(colDef->CustomDataSize >= sizeof(std::string));
    *(std::string*)colDef->CustomData = _DBVal;
    return NULL;
}

char* StrToInt32(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef)
{
    colIndex; rowsIndex;
    assert(colDef->CustomDataSize >= sizeof(int));
    char* endp;
    *(int*)colDef->CustomData = strtol(_DBVal, &endp, 10);
    return NULL;
}
