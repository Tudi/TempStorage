using CMAAgent.Tools;
using System;
using System.Collections.Generic;
using System.Linq;
using System.ServiceProcess;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace CMAAgent
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        static void Main()
        {
            const string monitoredAppName = "cradlewindowsagent.exe"; // todo : make it configurable
            const string monitoredProcessName = "CradleWindowsAgent"; // todo : make it configurable
#if DEBUG
            const string monitoredAppPath = "d:\\GitHub\\GauravMishra\\GIT\\windows-client\\CradleWindowsAgent\\bin\\Debug\\";
#else
            const string monitoredAppPath = "d:\\GitHub\\GauravMishra\\GIT\\windows-client\\CradleWindowsAgent\\bin\\Release\\";
#endif
            // create the main coordinator of events
            CMAService appService = new CMAService(monitoredAppName, monitoredProcessName, monitoredAppPath);

#if DEBUG
            appService.OnStartTest();
            char tmp = 'q';
            while (Console.Read() != tmp) Thread.Sleep(100);
#else
            ServiceBase[] ServicesToRun;
            ServicesToRun = new ServiceBase[]
            {
                appService
            };
            ServiceBase.Run(ServicesToRun);
#endif

        }
    }
}
