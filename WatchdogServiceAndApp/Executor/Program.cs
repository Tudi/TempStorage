using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Executor
{
    class Program
    {
        static void Main(string[] args)
        {
            //try the main app
            try
            {
                Process[] processes = Process.GetProcessesByName("CradleWindowsAgent");
                if (processes.Length == 0)
                {
                    string programFiles = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86);
                    string sPath = Path.Combine(programFiles, "CradleWindowsAgent");
                    sPath = Path.Combine(sPath, "CradleWindowsAgent.exe");
                    
                    Console.WriteLine("CradleWindowsAgent launch path = " + sPath);

                    Process.Start(sPath);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("CradleWindowsAgent launch ex = " + ex.Message);
            }

            //try the watcher
            try
            {
                Process[] processes = Process.GetProcessesByName("watcher");
                if (processes.Length == 0)
                {
                    // Start the main application
                    string sLocalAppData = Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData);
                    string sPath = Path.Combine(sLocalAppData, "{9A025A4A-6FA2-43E0-B8BF-3337488F5314}");
                    sPath = Path.Combine(sPath, "watcher.exe");

                    Console.WriteLine("watcher path = " + sPath);

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
        }
    }
}
