using System.ServiceProcess;
using System.Timers;
using System.ComponentModel;
using System.Configuration.Install;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Microsoft.Win32;
using System;

namespace WatchdogService
{
    internal static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        static void Main()
        {
            ServiceBase[] ServicesToRun;
            ServicesToRun = new ServiceBase[]
            {
                new ServiceMonitor()
            };
            ServiceBase.Run(ServicesToRun);
        }
    }

    public partial class ServiceMonitor : ServiceBase
    {
        [DllImport("user32.dll")]
        static extern bool ExitWindowsEx(uint uFlags, uint dwReason);

        private readonly string _serviceToMonitor = "MinuteCounterService";
        private Timer _timer;
        private int _redFlagCounter;

        private RegistryKey _registryKey1;
        private RegistryKey _registryKey2;
        private const string RegistryPath1 = "SOFTWARE\\WOW6432Node\\MinCounter";
        private const string RegistryValueName1 = "CounterHeartBeat";
        private const string RegistryPath2 = "SOFTWARE\\WOW6432Node\\MinCounter";
        private const string RegistryValueName2 = "UIHeartBeat";

        public ServiceMonitor()
        {
            ServiceName = "Watchdog";
            _redFlagCounter = 0;
        }

        private const int _updatePeriodMS = 5 * 60 * 1000;
        protected override void OnStart(string[] args)
        {

            _registryKey1 = Registry.LocalMachine.OpenSubKey(RegistryPath1, true) ?? Registry.LocalMachine.CreateSubKey(RegistryPath1);
            _registryKey2 = Registry.LocalMachine.OpenSubKey(RegistryPath2, true) ?? Registry.LocalMachine.CreateSubKey(RegistryPath2);

            // Create a timer that fires every minute
            _timer = new Timer(_updatePeriodMS);
            _timer.Elapsed += Timer_Elapsed;
            _timer.Start();

            // Check service status immediately when starting
            MonitorService();
        }

        private void Timer_Elapsed(object sender, ElapsedEventArgs e)
        {
            // one of the heartbeats is missing
            long value1 = (long)_registryKey1.GetValue(RegistryValueName1, 0);
            long value2 = (long)_registryKey2.GetValue(RegistryValueName2, 0);
            long stamp = DateTimeOffset.UtcNow.ToUnixTimeSeconds();
            if (stamp > value1 + 15 * 60 || stamp > value2 + 15 * 60)
            {
                EventLog.WriteEntry(ServiceName, $"stamp={stamp},value1={value1},value2={value2},_redFlagCounter={_redFlagCounter}", EventLogEntryType.Information);
                _redFlagCounter++;
            }

            // Check service status every minute
            MonitorService();

            // if for 15 minutes the monitor app was still not up. Restart the PC
            if (_redFlagCounter > 3)
            {
                EventLog.WriteEntry(ServiceName, $"triggered force shutdown _redFlagCounter={_redFlagCounter}", EventLogEntryType.Information);
                //                ExitWindowsEx(0x00000008, 0);
                Process.Start("shutdown", "/a");
                Process.Start("shutdown", "/f /s");
            }
            else if (_redFlagCounter > 2)
            {
                EventLog.WriteEntry(ServiceName, $"triggered shutdown _redFlagCounter={_redFlagCounter}", EventLogEntryType.Information);
//                ExitWindowsEx(0x00000008, 0);
                Process.Start("shutdown", "/f /s /t 900");
            }
        }

        private void MonitorService()
        {
            ServiceController service = new ServiceController(_serviceToMonitor);
            if (service.Status != ServiceControllerStatus.Running)
            {
                EventLog.WriteEntry(ServiceName, "Restarting target service", EventLogEntryType.Information);
                service.Start();
            }

            Process[] processes = Process.GetProcessesByName("ShowAvailableMinutes");
            if (processes.Length <= 0)
            {
                _redFlagCounter++;
                EventLog.WriteEntry(ServiceName, "Monitoring app was not found", EventLogEntryType.Information);
            }
        }

        protected override void OnStop()
        {
            EventLog.WriteEntry(ServiceName, $"Stopping service", EventLogEntryType.Information);
            _registryKey1.Close();
            _registryKey2.Close();

//            _timer.Stop();
//            _timer.Dispose();
        }
    }
}

[RunInstaller(true)]
public class MyServiceInstaller : Installer
{
    private ServiceInstaller _serviceInstaller;
    private ServiceProcessInstaller _processInstaller;

    public MyServiceInstaller()
    {
        // Set up the service installer
        _serviceInstaller = new ServiceInstaller();
        _serviceInstaller.ServiceName = "Watchdog";
        _serviceInstaller.Description = "Checks if service is running";
        _serviceInstaller.StartType = ServiceStartMode.Automatic;

        // Set up the process installer
        _processInstaller = new ServiceProcessInstaller();
        _processInstaller.Account = ServiceAccount.LocalSystem;

        // Add the installers to the collection
        Installers.Add(_processInstaller);
        Installers.Add(_serviceInstaller);
    }
}