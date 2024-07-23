using CMAAgent.Tools;
using Microsoft.Diagnostics.Tracing.StackSources;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.ServiceProcess;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using AppHeartbeatManager;
using Microsoft.Win32;

namespace CMAAgent
{
    // Define an event args class if needed
    public class ApplicationClosedEventArgs : EventArgs
    {
    }

    // Define a delegate for the event handler
    public delegate void ApplicationClosedEventHandler(object sender, ApplicationClosedEventArgs e);

    public partial class CMAService : ServiceBase
    {
        private Thread _monitorThread;
        private bool _running;
        private bool _MonitoredAppIsDown;
        private bool _MonitoredAppShouldBeUp;
        private string _MonitoredAppPath;
        private string _MonitoredAppName;
        private string _MonitoredProcessName;
        private KillKillerFilter taskKIllerKiller = null;
        private AppRunningMonitor appRunningMonitor = null;
        private AppHeartbeatVerifier appHeartbeatVerifier = null;
        private RegistryValueVerifier _registryMonitor = null;

        public CMAService(string monitoredAppName, string monitoredProcessName, string monitoredAppPath)
        {
            InitializeComponent();

            appHeartbeatVerifier = new AppHeartbeatVerifier(VerifiedApplicationTypes.CMAA);

            _MonitoredAppName = monitoredAppName;
            _MonitoredProcessName = monitoredProcessName;
            _MonitoredAppPath = monitoredAppPath;
            _MonitoredAppIsDown = IsMonitoredAppRunning();
            _MonitoredAppShouldBeUp = true;

            // if someone tries to kill our app with taskKill, we will kill their killer
            taskKIllerKiller = new KillKillerFilter();

            // restart monitored app in case it's down
            appRunningMonitor = new AppRunningMonitor();
            // register event from app closed monitor
            appRunningMonitor.ApplicationClosedEvents += this.OnApplicationClosed;

            // make sure registry values do not change compared to what we expect
            _registryMonitor = new RegistryValueVerifier();
            _registryMonitor.SetupSharedRegistryMonitoredValues();
        }

        internal void OnStartTest()
        {
            OnStart(null);
        }

        protected override void OnStart(string[] args)
        {
            LogWriter.WriteLog("App Started");
            LogWriter.WriteLog(string.Format("App Version - {0}", Global.GetCurrentAppVersion()));

            _running = true;
            _monitorThread = new Thread(new ThreadStart(MonitorAndRestart));
            _monitorThread.IsBackground = true;
            _monitorThread.Start();
        }

        protected override void OnStop()
        {
            _running = false;
            if (_monitorThread != null && _monitorThread.IsAlive)
            {
                _monitorThread.Join();
            }
        }

        private bool IsMonitoredAppRunning()
        {
            Process[] processes = Process.GetProcessesByName(_MonitoredProcessName);
            if (processes.Length == 0)
            {
                return false;
            }
            return true;
        }

        private void MonitorAndRestart()
        {
            LogWriter.WriteLog("MonitorAndRestart()");

            // kill app killer apps. We only accept gracefull closes
            Task.Run(() => taskKIllerKiller.StartCatchTaskKill(_MonitoredAppName)); 
            // if monitored app is down, start it
            Task.Run(() => appRunningMonitor.StartCatchProcessDown(_MonitoredProcessName));
            // check for applications faking to be us
            appHeartbeatVerifier.StartBackground();
            // make sure we monitor registry values
            _registryMonitor.Start();

            // brute force check periodically
            while (_running)
            {
                try
                {
                    CheckAndRestartProcess();
                }
                catch (Exception ex)
                {
                    // Log the exception (use a proper logging framework)
                    EventLog.WriteEntry("CMAService", ex.ToString(), EventLogEntryType.Error);
                }
                Thread.Sleep(3000); // Check every 30 seconds
            }

            // close all "services"
            appHeartbeatVerifier.StopBackground();
            taskKIllerKiller.StopCatchTaskKill();
            appRunningMonitor.StopCatchProcessDown();
            _registryMonitor.Stop();
        }

        // Event handler
        public void OnApplicationClosed(object sender, ApplicationClosedEventArgs e)
        {
            // Take actions when the application is closed
            LogWriter.WriteLog("Application closed. Taking actions...");
            _MonitoredAppIsDown = true;
            // should it be down ?
            if(_MonitoredAppShouldBeUp)
            {
                StartMonitoredApp();
            }
        }

        private void StartMonitoredApp()
        {
            // this might get spammed by "onclose" event. Probably a bug
            if(IsMonitoredAppRunning() == true)
            {
                return;
            }

            // Use a mutex to ensure that only one instance tries to start the process
            using (Mutex mutex = new Mutex(false, "Global\\" + _MonitoredProcessName))
            {
                LogWriter.WriteLog("Aquire mutex to start app.");
                if (mutex.WaitOne(0, false))
                {
                    // Start the main application
                    //string sLocalAppData = Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData);
                    //string sPath = Path.Combine(sLocalAppData, "{9A025A4A-6FA2-43E0-B8BF-3337488F5314}");
                    //sPath = Path.Combine(sPath, "executor.exe");
                    //LogWriter.WriteLog("attempting starting cma : " + sPath);
//                    string sPath = AppDomain.CurrentDomain.BaseDirectory + _MonitoredAppName;
                    string sPath = _MonitoredAppPath + _MonitoredAppName;

                    if (File.Exists(sPath))
                    {
                        try
                        {
                            LogWriter.WriteLog("Path exists. Starting application.");
#if DEBUG
                            System.Diagnostics.Process.Start(sPath);
#else
                            ApplicationLoader.PROCESS_INFORMATION procInfo;
                            var res = ApplicationLoader.StartProcessAndBypassUAC(sPath, out procInfo);
                            LogWriter.WriteLog("starting cwa : " + sPath + " " + res.ToString());
#endif
                            _MonitoredAppIsDown = false;
                        }
                        catch (Exception ex)
                        {
                            LogWriter.WriteLog("Error terminating taskkill process: " + ex.Message);
                        }
                    }
                    else
                    {
                        //handle if file not found
                    }
                }
            }
        }
        private void CheckAndRestartProcess()
        {
            LogWriter.WriteLog("CheckAndRestartProcess () ShouldRunApp = " + _MonitoredAppShouldBeUp + " is Running = " + IsMonitoredAppRunning());
            try
            {
                if (_MonitoredAppShouldBeUp == true && IsMonitoredAppRunning() == false)
                {
                    StartMonitoredApp();
                }
            }
            catch (Exception ex)
            {
                LogWriter.WriteLog("CheckAndRestartProcess () ex = " + ex.Message);
            }
        }
    }
}
