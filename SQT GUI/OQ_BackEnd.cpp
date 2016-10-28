#include "stdafx.h"
#include "OQ_BackEnd.h"
#include "ProjCtxt.h"
#include "Enviromt.h"
#include "Services.h"
#include "Auditor.h"
#include "NodeInfo.h"
#include "mfx.h"
#include "milres2.h"  // For resource DLL

#include "ResponseFormatter.h"
#include "OQAPI.h"
#include "TestBackEnd.h"
#include <conio.h>
#include "OQ_HTTP_API.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
    // Load the resource DLL
 
    static LoadLanguageResourceDLL langResDll;
    langResDll.Load(NULL);
    //bool bNoDatabase = false;

    int nRetCode = 0;
    CString tmp;
    // initialize MFC and print and error on failure
    if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
    {
        nRetCode = 1;
    }

    // Check if our API is working
    RunAllAPITestCases();

    // check HTTP API is working
    // you can test it with examnple : http://localhost:8081
    OQAPI_HTTP_Start( 8081 );

    //wait until key is pressed. This should definetly be changed to something else :P
    printf("Running HTTP server until keypress\n");
    getch();

    //stop http server
    OQAPI_HTTP_Stop();

    langResDll.UnLoad();

    return nRetCode;
}


