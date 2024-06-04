#pragma once

/*
* This class underwent many modifications. Due to backward compatibility it has some ugly parts.
* Chances are all you want from it is : src pointer, src size, source field name, dstpointer, dstsize
*/

enum LogSourceGroups : int;
class ExtractDBColumnToBinary;

// Generic declaration of a field formatter function
typedef void (*DBDestInit)(size_t rowCount, ExtractDBColumnToBinary* colDef);
typedef char *(*DBColFormatFnc)(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary *colDef);
typedef bool (*DBRowFinishedFnc)(int rowIndex, ExtractDBColumnToBinary *rowColDataArr);

// init callback functions
void InitDatagridToStoreRows(size_t rowCount, ExtractDBColumnToBinary* colDef);
void InitDropdownToStoreRows(size_t rowCount, ExtractDBColumnToBinary* colDef);

// Specific case that formats the AlertStatusId into the string representation
char* AlertStatusIdToStr(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef);
char* TimeStampToStr(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef);
char* TimeStampToSortableStr(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef);
char* TimeStampToStrAlertCard(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef);
char* TimeStampToDateStr(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef);
char* TimeStampToTimeStr(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef);
char* LogToMsgOnly(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef);
char* ModuleStatusIdToStr(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef);
char* GridStrValToUint64(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef);
char* StrToCustomObjFloat(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef);
char* StrToStdStr(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef);
char* StrToInt32(const char* _DBVal, int colIndex, int rowsIndex, ExtractDBColumnToBinary* colDef);

// parse a DB column into some sort of form field
class ExtractDBColumnToBinary
{
public:
    // extract DB string as reference. It will be later processed by callback function "on row finished"
    ExtractDBColumnToBinary(const char* _DBCol)
    {
        InitializeAllFields();
        DBCol = _DBCol;
        bStoredAsReference = true;
    }

    // extract string into an input field value
    ExtractDBColumnToBinary(const char* _DBCol, InputTextData* _FormField)
    {
        InitializeAllFields();
        DBCol = _DBCol;
        FormField = _FormField;
    }

    // Extract string into a grid column directly. 
    // You need the grid column specified because we might extract it into a different column than source column
    ExtractDBColumnToBinary(const char* _DBCol, uint32_t _GridCol, void *grid)
    {
        InitializeAllFields();
        DBCol = _DBCol;
        GridCol = _GridCol;
        customDestinationObj = grid;
    }

    // Extract string into a grid column directly. 
    // You need the grid column specified because we might extract it into a different column than source column
    // function is used to change the source data into a different string
    ExtractDBColumnToBinary(const char* _DBCol, uint32_t _GridCol, void* grid, DBColFormatFnc _fmtFnc)
    {
        InitializeAllFields();
        DBCol = _DBCol;
        GridCol = _GridCol;
        fmtFnc = _fmtFnc;
        customDestinationObj = grid;
    }

    // extract DB string into a preallocated buffer with fixed size
    ExtractDBColumnToBinary(const char* _DBCol, char *_FormStrField, size_t _FormStrFieldSize)
    {
        InitializeAllFields();
        DBCol = _DBCol;
        FormStrField = _FormStrField;
        FormStrFieldSize = _FormStrFieldSize;
    }

    // Use formatter function on extract DB string into a preallocated buffer with fixed size
    ExtractDBColumnToBinary(const char* _DBCol, char* _FormStrField, size_t _FormStrFieldSize, DBColFormatFnc _fmtFnc)
    {
        InitializeAllFields();
        DBCol = _DBCol;
        FormStrField = _FormStrField;
        FormStrFieldSize = _FormStrFieldSize;
        fmtFnc = _fmtFnc;
    }

    // turn extracted string data into an uint64 value
    ExtractDBColumnToBinary(const char* _DBCol, uint64_t *out_NumericData)
    {
        InitializeAllFields();
        DBCol = _DBCol;
        ullNumericData = out_NumericData;
    }

    // turn extracted string data into an uint64 value using a custom function
    ExtractDBColumnToBinary(const char* _DBCol, uint64_t* out_NumericData, DBColFormatFnc _fmtFnc)
    {
        InitializeAllFields();
        DBCol = _DBCol;
        ullNumericData = out_NumericData;
        fmtFnc = _fmtFnc;
    }

    ExtractDBColumnToBinary(const char* _DBCol, unsigned char* out_NumericData)
    {
        InitializeAllFields();
        DBCol = _DBCol;
        cNumericData = out_NumericData;
    }

    // turn extracted string data into an uint64 value using a custom function
    ExtractDBColumnToBinary(const char* _DBCol, void* outData, size_t outSize, DBColFormatFnc _fmtFnc)
    {
        InitializeAllFields();
        DBCol = _DBCol;
        CustomData = outData;
        CustomDataSize = outSize;
        fmtFnc = _fmtFnc;
    }

    // wrapper function to copy source string into destination buffer
    bool SetVal(const char* val, int colIndex = 0, int rowsAdded = 0);

    // make sure all fields are initialized all the time
    void InitializeAllFields()
    {
        DBCol = NULL;
        FormField = NULL;
        FormStrField = NULL;
        fmtFnc = NULL;
        ullNumericData = NULL;
        cNumericData = NULL;
        FormStrFieldSize = 0;
        GridCol = 0xFFFF;
        cbInit = NULL;
        cbDataRowFinished = NULL;
        cbDRF_userData1 = NULL;
        cbDRF_userData2 = NULL;
        customDestinationObj = NULL; // could be a grid or dropdown
        colValReadOnly = NULL;
        bStoredAsReference = false;
        nSoredDataHasSpecificSize = 0;
        CustomData = NULL;
        CustomDataSize = 0;
        bDoNotDeallocFormattedString = false;
    }

    // whenever we finished reading a whole row worth of data, call this function
    void SetDataRowFinishedFunc(DBRowFinishedFnc _cb, void* _cbDRF_RowStore, void* _cbDRF_DstObj)
    {
        cbDataRowFinished = _cb;
        cbDRF_userData1 = _cbDRF_RowStore;
        cbDRF_userData2 = _cbDRF_DstObj;
    }

    // in case we intend to use different object types to store arrays
    // we might want to have this object passed on
    void SetDestinationObj(void* newVal) { customDestinationObj = newVal; }

    // Based on known row count, some object should reinit
    void SetInitFunction(DBDestInit _cb) { cbInit = _cb; }

    /// <summary>
    /// Generic way to check if API returned valid results
    /// Also parses the result string into a JSON
    /// </summary>
    static WebApiErrorCodes DBH_APIResultValid(int CurlErr, char* response, yyjson_doc*& jsonResponse, LogSourceGroups sg, const char* func);

    /// <summary>
    /// Generic way to parse ingle row of DB row into form fields
    /// </summary>
    static void DBH_ParseDBRowFromJSON(yyjson_doc* jsonResponse, const char* DataGroupName,
        ExtractDBColumnToBinary* colArr, LogSourceGroups sg);

    // name of the column in the DB 
    const char* DBCol;
    // if there is a "row finished" callback, it will receive these points
    // probably copy source and copy destination pointers
    void* cbDRF_userData1,* cbDRF_userData2;
    // could be a grid or dropdown. this means you have a manually made function for every column
    void* customDestinationObj; 
    // when formatting function generates some custom object other than string
    size_t nSoredDataHasSpecificSize;
    // if we only intend to temp store the JSON source strings because we will process them when the whole row finished extracting
    const char* colValReadOnly;
    // format function will handle the storage. Format function should return NULL in this case
    void* CustomData;
    size_t CustomDataSize;
    bool bDoNotDeallocFormattedString;
    char AllocOnceBufferForValFormat[MAX_DB_STRING_LENGTH];
protected:
    InputTextData* FormField;
    char* FormStrField;
    size_t FormStrFieldSize;
    uint32_t GridCol;
    DBColFormatFnc fmtFnc;
    uint64_t* ullNumericData;
    unsigned char* cNumericData; // Not a string 
    // if data got extracted into a temporary row object, we want to convert that into a final row inside the DB
    // you could store it in an array of strings and do whatever you want with the strings
    DBRowFinishedFnc cbDataRowFinished;
    // grids, dropdown are arrays that get initialized before storing values in them
    DBDestInit cbInit;
    bool bStoredAsReference;
};

