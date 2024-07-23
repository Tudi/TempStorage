using Microsoft.Diagnostics.Tracing.Parsers;
using Microsoft.Diagnostics.Tracing.Session;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CMAAgent.Tools
{
    public class AppRunningMonitor
    {
        private TraceEventSession traceEventSessionAppRunning = null;
        public event ApplicationClosedEventHandler ApplicationClosedEvents;

        public void StartCatchProcessDown(string sMonitoredProcessName)
        {
            traceEventSessionAppRunning = new TraceEventSession("MyStartAppSession");
            traceEventSessionAppRunning.EnableKernelProvider(KernelTraceEventParser.Keywords.Process);

            traceEventSessionAppRunning.Source.Dynamic.All += traceEvent =>
            {
//                if(traceEvent.EventName == "Thread/SetName") return;
//                if (traceEvent.EventName == "Thread/Start") return;
//                if (traceEvent.EventName == "Thread/DCStart") return;
//                if (traceEvent.EventName == "Thread/End") return;
                if ( traceEvent.EventName == "Process/Terminate" || traceEvent.EventName == "Process/Stop")
                {
                    string processName = traceEvent.ProcessName;
                    if (processName.Equals(sMonitoredProcessName, StringComparison.OrdinalIgnoreCase))
                    {
                        LogWriter.WriteLog($"Target process '{sMonitoredProcessName}' stopped.");
                        ApplicationClosedEvents?.Invoke(this, new ApplicationClosedEventArgs());
                    }
                }
            };

            LogWriter.WriteLog("Listening for process close events...");
            traceEventSessionAppRunning.Source.Process();
        }

        public void StopCatchProcessDown()
        {
            if (traceEventSessionAppRunning != null)
            {
                traceEventSessionAppRunning.Stop();
                traceEventSessionAppRunning.Dispose();
                traceEventSessionAppRunning = null;
            }
        }
    }
}
