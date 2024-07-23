using Microsoft.Win32;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

namespace CMAAgent.Tools
{
    internal class RegistryValueVerifier
    {
        private Dictionary<string, (object value, RegistryValueKind kind)> _valuesToMonitor;
        private Thread _monitorThread;
        private bool _isRunning;
        const int _updatePeriodMS = 1000; // check every x MS if values are what we expect

        public RegistryValueVerifier()
        {
            _valuesToMonitor = new Dictionary<string, (object value, RegistryValueKind kind)>();
        }

        public void AddValue(string registryPath, object value, RegistryValueKind kind)
        {
            _valuesToMonitor[registryPath] = (value, kind);
        }

        public void Start()
        {
            _isRunning = true;
            _monitorThread = new Thread(MonitorRegistry);
            _monitorThread.IsBackground = true;
            _monitorThread.Start();
        }

        public void Stop()
        {
            _isRunning = false;
            _monitorThread.Join();
        }

        private void MonitorRegistry()
        {
            while (_isRunning)
            {
                foreach (var kvp in _valuesToMonitor)
                {
                    string registryPath = kvp.Key;
                    var (expectedValue, kind) = kvp.Value;

                    using (var key = GetRegistryKey(registryPath, out string valueName))
                    {
                        if (key != null)
                        {
                            // currently stored reg value
                            object currentValue = key.GetValue(valueName, null);

                            // value needs to be restored
                            if (!AreValuesEqual(currentValue, expectedValue, kind))
                            {
                                // Restore the value
                                key.SetValue(valueName, expectedValue, kind);

                                // Handle application path
                                if (kind == RegistryValueKind.String && IsPath((string)expectedValue))
                                {
                                    string currentPath = (string)currentValue;
                                    KillApplicationAtPath(currentPath);
                                }
                            }
                        }
                    }
                }

                Thread.Sleep(_updatePeriodMS); // Check every second
            }
        }

        private static bool AreValuesEqual(object currentValue, object expectedValue, RegistryValueKind kind)
        {
            if (currentValue == null || expectedValue == null)
                return false;

            switch (kind)
            {
                case RegistryValueKind.Binary:
                    return StructuralComparisons.StructuralEqualityComparer.Equals(currentValue, expectedValue);
                default:
                    return currentValue.Equals(expectedValue);
            }
        }

        private static bool IsPath(string value)
        {
            return value.Contains("\\") || value.Contains("/");
        }

        // if a reg key contains an application, and it's not an application we expect, we will kill the old application before we restore the real value
        private static void KillApplicationAtPath(string path)
        {
            foreach (var process in Process.GetProcesses())
            {
                try
                {
                    if (process.MainModule.FileName.Equals(path, StringComparison.OrdinalIgnoreCase))
                    {
                        process.Kill();
                    }
                }
                catch
                {
                    // Ignore any exceptions related to accessing process information
                }
            }
        }

        // open a reg key to get it's value
        private static RegistryKey GetRegistryKey(string fullPath, out string valueName)
        {
            string[] pathParts = fullPath.Split('\\');
            valueName = pathParts[pathParts.Length - 1];
            string keyPath = string.Join("\\", pathParts, 0, pathParts.Length - 1);

            RegistryKey baseKey;
            if (keyPath.StartsWith("HKEY_LOCAL_MACHINE"))
            {
                baseKey = Registry.LocalMachine;
                keyPath = keyPath.Substring("HKEY_LOCAL_MACHINE\\".Length);
            }
            else if (keyPath.StartsWith("HKEY_CURRENT_USER"))
            {
                baseKey = Registry.CurrentUser;
                keyPath = keyPath.Substring("HKEY_CURRENT_USER\\".Length);
            }
            else
            {
                throw new ArgumentException("Unsupported registry hive.");
            }

            return baseKey.OpenSubKey(keyPath, true);
        }

        // this class will be shared among multiple applications
        // each of the applications should be able to monitor the other applications
        public void SetupSharedRegistryMonitoredValues()
        {
            AddValue(@"HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\CMAA\Type", 0x10, RegistryValueKind.DWord);
            AddValue(@"HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\CMAA\Start", 0x02, RegistryValueKind.DWord);
            AddValue(@"HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\CMAA\ErrorControl", 0x1, RegistryValueKind.DWord);
            AddValue(@"HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\CMAA\ObjectName", "LocalSystem", RegistryValueKind.String);
            AddValue(@"HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\CMAA\DisplayName", "CMAA", RegistryValueKind.String);
            AddValue(@"HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\CMAA\ImagePath", @"d:\GitHub\GauravMishra\GIT\windows-client\CMAAgent\bin\Release\CMAAgent.exe", RegistryValueKind.String);
            AddValue(@"HKEY_CURRENT_USER\Software\MyApp\FailureActions", new byte[] { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, RegistryValueKind.Binary);
        }

    }
}
