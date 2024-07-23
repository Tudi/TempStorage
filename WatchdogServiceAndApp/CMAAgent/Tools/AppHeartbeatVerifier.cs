using System;
using System.Linq;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.IO;
using System.IO.MemoryMappedFiles;
using System.Diagnostics;
using System.Threading;
using CMAAgent.Tools;
using System.Collections.Generic;
using System.Security.AccessControl;

namespace AppHeartbeatManager
{
    // list of applications we intend to monitor each other
    public enum VerifiedApplicationTypes
    {
        CMAA = 0,
        CradleWindowsAgent = 1,
        MaxUsedValue,
        Reserved = 5, // in case additional services will be added, make sure we do not need to recompile old ones
        MaxValue
    }

    // Have a list of applications that all should report a heartbeat.
    // If an application is not reporting a heartbeat, make sure the application is closed
    [StructLayout(LayoutKind.Explicit, Pack = 1)]
    public class SharedAppData
    {
        [FieldOffset(0)]
        public bool bShouldAppRun;
        [FieldOffset(8)]
        public int nAppHeartBeat;
    }

    // shared memory interface : read write blocks of data in cross aplication thread safe way
    public class SharedMemoryManager : IDisposable
    {
        private const string SharedMemoryName = "Global\\MySharedMemory";
        private const string MutexName = "Global\\MySharedMemoryMutex";
        private readonly int SharedMemorySize = Marshal.SizeOf(typeof(SharedAppData)) * ((int)VerifiedApplicationTypes.MaxValue);

        private MemoryMappedFile memoryMappedFile;
        private MemoryMappedViewAccessor accessor;
        private Mutex mutex;

        public SharedMemoryManager()
        {
            // Adjusting access rights to allow service and user processes to access the shared memory
            var security = new MemoryMappedFileSecurity();
            security.AddAccessRule(new System.Security.AccessControl.AccessRule<MemoryMappedFileRights>(
                new System.Security.Principal.SecurityIdentifier(System.Security.Principal.WellKnownSidType.WorldSid, null),
                MemoryMappedFileRights.FullControl,
                System.Security.AccessControl.AccessControlType.Allow));

            memoryMappedFile = MemoryMappedFile.CreateOrOpen(SharedMemoryName, SharedMemorySize, MemoryMappedFileAccess.ReadWrite, MemoryMappedFileOptions.None, security, System.IO.HandleInheritability.None);
            accessor = memoryMappedFile.CreateViewAccessor();

            // Ensure the mutex has the correct permissions as well
            var mutexSecurity = new MutexSecurity();
            mutexSecurity.AddAccessRule(new MutexAccessRule(new System.Security.Principal.SecurityIdentifier(System.Security.Principal.WellKnownSidType.WorldSid, null), MutexRights.FullControl, System.Security.AccessControl.AccessControlType.Allow));
            mutex = new Mutex(false, MutexName, out bool createdNew, mutexSecurity);
        }

        // write a block of data to a specific shared memory region
        public void WriteData(SharedAppData data, VerifiedApplicationTypes appType)
        {
            var dataBlockSize = Marshal.SizeOf(typeof(SharedAppData));
            byte[] buffer = new byte[dataBlockSize];
            IntPtr ptr = Marshal.AllocHGlobal(dataBlockSize);

            try
            {
                mutex.WaitOne();
                Marshal.StructureToPtr(data, ptr, true);
                Marshal.Copy(ptr, buffer, 0, dataBlockSize);

                var dataBlockOffset = (int)appType * dataBlockSize;
                accessor.WriteArray(dataBlockOffset, buffer, 0, buffer.Length);
            }
            finally
            {
                Marshal.FreeHGlobal(ptr);
                mutex.ReleaseMutex();
            }
        }

        // read shared memory from a specific region
        public SharedAppData ReadData(VerifiedApplicationTypes appType)
        {
            var dataBlockSize = Marshal.SizeOf(typeof(SharedAppData));
            byte[] buffer = new byte[dataBlockSize];
            IntPtr ptr = Marshal.AllocHGlobal(dataBlockSize);

            try
            {
                mutex.WaitOne();
                var dataBlockOffset = (int)appType * dataBlockSize;
                accessor.ReadArray(dataBlockOffset, buffer, 0, buffer.Length);
                Marshal.Copy(buffer, 0, ptr, buffer.Length);
                return (SharedAppData)Marshal.PtrToStructure(ptr, typeof(SharedAppData));
            }
            finally
            {
                Marshal.FreeHGlobal(ptr);
                mutex.ReleaseMutex();
            }
        }

        public void Dispose()
        {
            accessor?.Dispose();
            memoryMappedFile?.Dispose();
            mutex?.Dispose();
        }
    }

    // object to keep track of applications.
    // we expect the applications to implement this class also. Else they will be closed
    // maybe in the future also start applications that are in this circular shared state
    // right now only a single instance is allowed for every app type
    // todo : this is async. It should ask main application if it's deadlocked or not
    public class AppHeartbeatVerifier
    {
        private SharedMemoryManager _sharedMemory;
        private SharedAppData [] _sharedDataExpected;
        private VerifiedApplicationTypes _myAppType;
        private int [] _secondChanceCounter;
        private CancellationTokenSource _cancellationTokenSource = null;
        private Task _backgroundTask;
        private const int _heartbeatPeriod = 4000; // needs to be the same for every app
        public AppHeartbeatVerifier(VerifiedApplicationTypes myType)
        {
            _sharedMemory = new SharedMemoryManager();
            _myAppType = myType;

            // make sure others know we are supposed to be alive
            UpdateMyHeartbeatState();

            _secondChanceCounter = new int[(int)VerifiedApplicationTypes.MaxUsedValue];
            _sharedDataExpected = new SharedAppData[(int)VerifiedApplicationTypes.MaxUsedValue];

            // read current state of all applications
            for (int i = 0; i < (int)VerifiedApplicationTypes.MaxUsedValue; i++)
            {
                _sharedDataExpected[i] = _sharedMemory.ReadData((VerifiedApplicationTypes)i);
            }
        }

        public void UpdateMyHeartbeatState(VerifiedApplicationTypes appType = VerifiedApplicationTypes.MaxValue, bool appIsRunning = true)
        {
            if(appType == VerifiedApplicationTypes.MaxValue)
            {
                appType = _myAppType;
            }

            // make sure we see what they see
            var sharedData = _sharedMemory.ReadData(appType);

            if(appIsRunning != sharedData.bShouldAppRun)
            {
                LogWriter.WriteLog("App " + _myAppType + " state is switching from " + sharedData.bShouldAppRun + " to " + appIsRunning);
            }

            // update self status
            if (appIsRunning == true)
            {
                sharedData.nAppHeartBeat++;
                sharedData.bShouldAppRun = true;
            }
            else
            {
                sharedData.nAppHeartBeat = 0;
                sharedData.bShouldAppRun = false;
            }

            // let others know about our status
            _sharedMemory.WriteData(sharedData, appType);
        }

        public void VerifyApplications()
        {
            // check others are fine
            for (int i = 0;i< (int)VerifiedApplicationTypes.MaxUsedValue; i++)
            {
                // skip self check. Others will do it for us
                if(i == (int)_myAppType)
                {
                    continue;
                }

                // read the next block
                var sharedData = _sharedMemory.ReadData((VerifiedApplicationTypes)i);

                // is the application running ?
                if (sharedData.bShouldAppRun == true)
                {
                    // app is running but for some reason the heartbeat is not increasing
                    if (sharedData.nAppHeartBeat < _sharedDataExpected[i].nAppHeartBeat)
                    {
                        CloseStrangeStateApplication((VerifiedApplicationTypes)i, false);
                    }
                    else
                    {
                        // no need to give second chances next time. reset counter
                        _secondChanceCounter[i] = 0;
                    }
                }
                // we are not expecting this application to run
                else
                {
                    CloseStrangeStateApplication((VerifiedApplicationTypes)i, true);
                }

                // update state for next time
                _sharedDataExpected[i] = sharedData;
                // we are expectint this heartbeat on next cycle
                _sharedDataExpected[i].nAppHeartBeat = sharedData.nAppHeartBeat + 1;
            }
        }

        private void CloseStrangeStateApplication(VerifiedApplicationTypes appType, bool bSkipGracePeriod)
        {
            if (bSkipGracePeriod == false)
            {
                // maybe app is starting up
                const int _msWaitAppStartup = _heartbeatPeriod; // todo : move this to settings
                if (_secondChanceCounter[(int)appType] == 0)
                {
                    LogWriter.WriteLog("Second chance countdown started for " + appType);
                    _secondChanceCounter[(int)appType] = Environment.TickCount + _msWaitAppStartup;
                    return;
                }
                if (_secondChanceCounter[(int)appType] > Environment.TickCount)
                {
                    LogWriter.WriteLog("Second chance countdown ongoing for " + appType);
                    return;
                }
                _secondChanceCounter[(int)appType] = 0;
            }

            LogWriter.WriteLog("Searching to kill " + appType);

            var config = ApplicationConfigurations.Configurations[appType];

            bool bMadeAKill = config.Terminate(appType);

            if (bMadeAKill || bSkipGracePeriod)
            {
                UpdateMyHeartbeatState(appType, false);
            }
        }
        public void StartBackground()
        {
            // this is to support pause/resume functionality
            UpdateMyHeartbeatState(_myAppType, true);

            if (_cancellationTokenSource != null)
            {
                StopBackground();
            }

            _cancellationTokenSource = new CancellationTokenSource();
            var token = _cancellationTokenSource.Token;

            _backgroundTask = Task.Run(async () =>
            {
                while (!token.IsCancellationRequested)
                {
                    UpdateMyHeartbeatState();
                    await Task.Delay(_heartbeatPeriod);
                    VerifyApplications();
                }
            }, token);
        }

        public void StopBackground()
        {
            if (_cancellationTokenSource != null)
            {
                _cancellationTokenSource.Cancel();
                _backgroundTask.Wait();
                _cancellationTokenSource = null;
            }

            // mark this application type as not supposed to be running
            // this is to support pause/resume functionality
            UpdateMyHeartbeatState(_myAppType, false);
        }

        public void Dispose()
        {
            // mark this application type as not supposed to be running
            UpdateMyHeartbeatState(_myAppType, false);
        }
    }

    public class ApplicationConfig
    {
        public string ImgPath { get; set; }
        public string ProcessName { get; set; }
        public string ProcessType { get; set; }
        public Func<VerifiedApplicationTypes, bool> Terminate { get; set; }

        public ApplicationConfig(string imgPath, string processName, string processType, Func<VerifiedApplicationTypes, bool> terminate)
        {
            ImgPath = imgPath;
            ProcessName = processName;
            ProcessType = processType;
            Terminate = terminate;
        }
    }

    public static class ApplicationConfigurations
    {
        // this data is shared between multiple applications and needs to be burnt into the application
        // the paths can be dynamic, but then they need to be configured by the installer
        public static Dictionary<VerifiedApplicationTypes, ApplicationConfig> Configurations = new Dictionary<VerifiedApplicationTypes, ApplicationConfig>
        {
            { VerifiedApplicationTypes.CMAA, new ApplicationConfig("d:\\GitHub\\GauravMishra\\GIT\\windows-client\\CMAAgent\\bin\\Release\\CMAAgent.exe", "CMAAgent.exe", "Service", TerminateService) },
            { VerifiedApplicationTypes.CradleWindowsAgent, new ApplicationConfig("d:\\GitHub\\GauravMishra\\GIT\\windows-client\\CradleWindowsAgent\\bin\\Debug\\CradleWindowsAgent.exe", "CradleWindowsAgent", "Process", TerminateProcess) },
        };

        private static bool TerminateService(VerifiedApplicationTypes appType)
        {
            return TerminateProcess(appType);
        }

        private static bool TerminateProcess(VerifiedApplicationTypes appType)
        {
            ApplicationConfig conf = Configurations[appType];

            bool bMadeAKill = false;

            string sProcessName = conf.ProcessName;
            foreach (var process in Process.GetProcessesByName(sProcessName))
            {
                LogWriter.WriteLog("killing " + sProcessName);
                process.Kill();
                bMadeAKill = true;
            }

            // what if the malicius application switched it's name ?
            string sAppPath = conf.ImgPath;
            string sAppName = Path.GetFileNameWithoutExtension(sAppPath);
            var processes = Process.GetProcesses().Where(p => p.ProcessName.Equals(sAppName, StringComparison.OrdinalIgnoreCase));
            foreach (var process in processes)
            {
                try
                {
                    LogWriter.WriteLog("killing " + sAppName);
                    process.Kill();
                    bMadeAKill = true;
                }
                catch (Exception ex)
                {
                    LogWriter.WriteLog($"Failed to kill process: {process.ProcessName}, ID: {process.Id}. Error: {ex.Message}");
                }
            }

            return bMadeAKill;
        }
    }
}
