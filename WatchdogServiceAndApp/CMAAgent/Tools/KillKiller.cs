using Microsoft.Diagnostics.Tracing.Parsers;
using Microsoft.Diagnostics.Tracing.Session;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Management;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Security.Principal;
using System.Diagnostics;
using System.IO;
using System.ServiceProcess;
using System.Threading;
using System.Windows;

namespace CMAAgent.Tools
{
    public class KillKillerFilter
    {
        [DllImport("kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool TerminateProcess(IntPtr hProcess, uint uExitCode);

        static ManagementEventWatcher watcher = null;
        public static void InstallTaskExecutionFilter()
        {
            string query = "SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process'";
            ManagementEventWatcher watcher = new ManagementEventWatcher(new WqlEventQuery(query));
            watcher.EventArrived += new EventArrivedEventHandler(OnProcessCreated);
            watcher.Start();

            LogWriter.WriteLog("Listening for taskkill process creation events...");
        }

        public static void RemoveTaskExecutionFilter()
        {
            watcher.Stop();
        }

        static void OnProcessCreated(object sender, EventArrivedEventArgs e)
        {
            var process = (ManagementBaseObject)e.NewEvent["TargetInstance"];
            string processName = process["Name"].ToString();

            if (processName.ToLower().Contains("taskkill"))
            {
                string processId = process["ProcessId"].ToString();
                string commandLine = GetCommandLine(processId);

                LogWriter.WriteLog("Taskkill process detected: " + process["Name"] + " (PID: " + process["ProcessId"] + ")");
                LogWriter.WriteLog("Command Line: " + commandLine);

                // Attempt to terminate the taskkill process
                try
                {
                    Process proc = Process.GetProcessById(Convert.ToInt32(processId));
                    if (TerminateProcess(proc.Handle, 1))
                    {
                        LogWriter.WriteLog("Taskkill process terminated successfully.");
                    }
                    else
                    {
                        LogWriter.WriteLog("Failed to terminate taskkill process.");
                    }
                }
                catch (Exception ex)
                {
                    LogWriter.WriteLog("Error terminating taskkill process: " + ex.Message);
                }
            }
        }

        static string GetCommandLine(string processId)
        {
            string commandLine = "";
            string query = $"SELECT CommandLine FROM Win32_Process WHERE ProcessId = {processId}";

            ManagementObjectSearcher searcher = new ManagementObjectSearcher(query);
            foreach (ManagementObject obj in searcher.Get())
            {
                commandLine = obj["CommandLine"]?.ToString();
            }

            return commandLine;
        }

        static void KillProcess(int ProcessID)
        {
            if (ProcessID <= 0)
            {
                return;
            }
            try
            {
                var process = System.Diagnostics.Process.GetProcessById(ProcessID);
                if (!process.HasExited)
                {
                    process.Kill();
                    LogWriter.WriteLog("Taskkill process terminated successfully.");
                }
            }
            catch (Exception ex)
            {
                LogWriter.WriteLog("Error terminating taskkill process: " + ex.Message);
            }
        }

        public static void CatchTaskkill()
        {
            using (var session = new TraceEventSession("MySession"))
            {
                session.EnableKernelProvider(KernelTraceEventParser.Keywords.Process);

                session.Source.Kernel.ProcessStart += (data) =>
                {
                    if (data.ImageFileName.Equals("taskkill.exe", StringComparison.OrdinalIgnoreCase))
                    {
                        LogWriter.WriteLog("Taskkill process detected: " + data.ImageFileName + " (PID: " + data.ProcessID + ")");
                        LogWriter.WriteLog("Command Line: " + data.CommandLine);

                        // Attempt to terminate the taskkill process
                        KillProcess(data.ProcessID);
                        KillProcess(data.ParentID);
                    }
                };

                LogWriter.WriteLog("Listening for taskkill process creation events...");
                session.Source.Process();
            }
        }

        private TraceEventSession traceEventSessionKillTaskKill = null;
        public void StartCatchTaskKill(string sAppNameProtected)
        {
            traceEventSessionKillTaskKill = new TraceEventSession("MyCloseAppSession");
            traceEventSessionKillTaskKill.EnableKernelProvider(KernelTraceEventParser.Keywords.Process);

            traceEventSessionKillTaskKill.Source.Kernel.ProcessStart += (data) =>
            {
                if (data.ImageFileName.Equals("taskkill.exe", StringComparison.OrdinalIgnoreCase))
                {
                    LogWriter.WriteLog("Taskkill process detected: " + data.ImageFileName + " (PID: " + data.ProcessID + ")");
                    LogWriter.WriteLog("Command Line: " + data.CommandLine);

                    // Attempt to terminate the taskkill process
                    if (data.CommandLine.ToLower().Contains(sAppNameProtected))
                    {
                        KillProcess(data.ProcessID);
                        KillProcess(data.ParentID);
                    }
                }
            };

            LogWriter.WriteLog("Listening for taskkill process creation events...");
            traceEventSessionKillTaskKill.Source.Process();
        }

        public void StopCatchTaskKill()
        {
            if (traceEventSessionKillTaskKill != null)
            {
                traceEventSessionKillTaskKill.Stop();
                traceEventSessionKillTaskKill.Dispose();
                traceEventSessionKillTaskKill = null;
            }
        }
    }
}
