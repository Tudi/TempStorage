using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Watcher
{
    class Program
    {
        private static Mutex appMutex;
        private const string MutexGUID = "3CB7CDF7-5911-4636-9A4F-904105F8D736"; //ok
        private const string EventMutex = "3E969D5A-F7A5-4846-8D2E-9D04489F44D6"; //ok
        private static EventWaitHandle appWaitHandle;

       
        static void Main(string[] args)
        {
            try
            {
                #region Managing Instances
                //Handling running instances
                bool createdNew;
                appMutex = new Mutex(true, MutexGUID, out createdNew);
                appWaitHandle = new EventWaitHandle(false, EventResetMode.AutoReset, EventMutex);

                GC.KeepAlive(appMutex);
                if (!createdNew)
                {
                    Console.WriteLine("App is already running, return");
                    appWaitHandle.Set();
                    return;
                }

                #endregion
            }
            catch (Exception ex)
            {
                Console.WriteLine("Is windows Agent check ex- " + ex.Message);
            }

            while (true)
            {
                try
                {
                    Process[] processes = Process.GetProcessesByName("CradleWindowsAgent");
                    if (processes.Length == 0)
                    {
                        
                        // Start the main application
                        string sLocalAppData = Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData);
                        string sPath = Path.Combine(sLocalAppData, "{9A025A4A-6FA2-43E0-B8BF-3337488F5314}");
                        sPath = Path.Combine(sPath, "executor.exe");

                        Console.WriteLine("sPath executor " + sPath);

                        if (File.Exists(sPath))
                        {
                            var startInfo = new ProcessStartInfo
                            {
                                FileName = sPath,
                                CreateNoWindow = true,
                                WindowStyle = ProcessWindowStyle.Hidden,
                                UseShellExecute = false
                            };
                            Process.Start(startInfo);
                        }
                        else
                        {
                            //handle if file not found
                        }
                          
 
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine("CheckAndRestartProcess () ex = " + ex.Message);
                }

                Thread.Sleep(3000); // Check every 3 seconds
            }
        }
    }
}
