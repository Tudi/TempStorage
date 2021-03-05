#include <windows.h>

#include "TestAPI.h"
#include "Logger.h"

void TestingRunMultipleTests()
{
    bool result = TestingCreateDeviceTwoMonitors();
    // If we failed first test, probably not worth continuing
    if (result == false)
    {
        return;
    }
    TestingWaitForKeypress('x');

    TestingAddMonitors();
    TestingWaitForKeypress('x');

    TestingRemoveAMonitor();
    TestingWaitForKeypress('x');

    TestingReAddSameMonitor();
    TestingWaitForKeypress('x');

    TestChangeMonitorResolution();
    TestingWaitForKeypress('x');

    TestingRemoveAddNewMonitor();
    TestingWaitForKeypress('x');

    TestingReplaceMonitor();
    TestingWaitForKeypress('x');

    TestingUnloadDevice();
}

int __cdecl main(int argc, wchar_t *argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtMemState state;
    _CrtMemCheckpoint(&state);
    
    // Run test in a function to not report temp variables as mem leaks
    TestingRunMultipleTests();

    //check for leaked memory
    //_CrtDumpMemoryLeaks();
    _CrtMemDumpAllObjectsSince(&state);

    return 0;
}