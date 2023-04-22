using System;
using System.ServiceProcess;
using Microsoft.Win32;
using System.ComponentModel;
using System.Configuration.Install;
using System.Diagnostics;

namespace CountMinutesUsed
{
    public partial class MinuteCounterService : ServiceBase
    {
        private int _value;
        private readonly RegistryKey _registryKey1;
        private readonly RegistryKey _registryKey2;
        private readonly RegistryKey _registryKey3;
        private const string RegistryPath1 = "SOFTWARE\\WOW6432Node\\MinCounter";
        private const string RegistryValueName1 = "SumOf5";
        private const string RegistryPath2 = "SOFTWARE\\WOW6432Node\\MicRosoft\\BackupCounter";
        private const string RegistryValueName2 = "Backup5";
        private const string RegistryPath3 = "SOFTWARE\\WOW6432Node\\MinCounter";
        private const string RegistryValueName3 = "CounterHeartBeat";
        private System.Timers.Timer _timer;

        public MinuteCounterService()
        {
            ServiceName = "MinuteCounterService";

            // Open the registry key
            _registryKey1 = Registry.LocalMachine.OpenSubKey(RegistryPath1, true) ?? Registry.LocalMachine.CreateSubKey(RegistryPath1);
            _registryKey2 = Registry.LocalMachine.OpenSubKey(RegistryPath2, true) ?? Registry.LocalMachine.CreateSubKey(RegistryPath2);
            _registryKey3 = Registry.LocalMachine.OpenSubKey(RegistryPath3, true) ?? Registry.LocalMachine.CreateSubKey(RegistryPath3);

            // Load the value from the registry
            _value = (int)_registryKey1.GetValue(RegistryValueName1, 0);
            int value = (int)_registryKey2.GetValue(RegistryValueName2, 0);
            _value = Math.Max(_value, value);

            // Set the service properties
            CanPauseAndContinue = false;
            CanShutdown = true;

//            EventLog.WriteEntry(ServiceName, "Starting Service", EventLogEntryType.Information);
//            EventLog.WriteEntry(ServiceName, $"Counter value {_value}", EventLogEntryType.Information);
        }

        protected override void OnStart(string[] args)
        {
            // Start a timer to increase the value every 5 minutes
            _timer = new System.Timers.Timer();
            _timer.Elapsed += Timer_Elapsed;
            _timer.Interval = TimeSpan.FromMinutes(5).TotalMilliseconds;
            _timer.Start();
        }

        private void Timer_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            // Increase the value and save it back to the registry
            _value+=5;
            _registryKey1.SetValue(RegistryValueName1, _value, RegistryValueKind.DWord);

            int value = (int)_registryKey2.GetValue(RegistryValueName2, 0);
            _value = Math.Max(_value, value);
            value = Math.Max(_value, value);
            _registryKey2.SetValue(RegistryValueName2, value, RegistryValueKind.DWord);

            _registryKey3.SetValue(RegistryValueName3, DateTimeOffset.UtcNow.ToUnixTimeSeconds(), RegistryValueKind.DWord);

//            EventLog.WriteEntry(ServiceName, $"Counter updated to {_value}", EventLogEntryType.Information);
        }

        protected override void OnStop()
        {
            // Close the registry key when the service stops
            _registryKey1.Close();
            _registryKey2.Close();
            _registryKey3.Close();

//            EventLog.WriteEntry(ServiceName, "Stopping Service", EventLogEntryType.Information);
        }

        protected override void OnPause()
        {
        }

        protected override void OnContinue()
        {
        }

        protected override void OnShutdown()
        {
            // Stop the timer and close the registry key when the service is shut down
            _timer.Stop();
            _registryKey1.Close();
            _registryKey2.Close();
            _registryKey3.Close();
        }

        protected override void Dispose(bool disposing)
        {
            // Dispose of the timer and registry key
            if (disposing)
            {
                _timer.Dispose();
                _registryKey1.Dispose();
                _registryKey2.Dispose();
                _registryKey3.Dispose();
            }
            base.Dispose(disposing);
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
        _serviceInstaller.ServiceName = "MinuteCounterService";
        _serviceInstaller.Description = "Counts minutes";
        _serviceInstaller.StartType = ServiceStartMode.Automatic;

        // Set up the process installer
        _processInstaller = new ServiceProcessInstaller();
        _processInstaller.Account = ServiceAccount.LocalSystem;

        // Add the installers to the collection
        Installers.Add(_processInstaller);
        Installers.Add(_serviceInstaller);
    }
}
