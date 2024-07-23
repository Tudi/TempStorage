using CradleWindowsAgent.Firewall;
using CradleWindowsAgent.Tools;
using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;

namespace CradleWindowsAgent
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        public MainWindow wndMain = null;

        private Mutex appMutex;
        private const string MutexGUID = "9BCC8843-064D-4631-AE8E-C2D1902DAD97";
        private const string EventMutex = "927B44FC-097B-4FEC-886E-985C62BEC665";
        private EventWaitHandle appWaitHandle;

        private Thread _monitorThread;
        private void Application_Startup(object sender, StartupEventArgs e)
        {
            LogWriter.WriteLog("App Started");
            LogWriter.WriteLog(string.Format("App Version - {0}", Global.GetCurrentAppVersion()));

            try
            {
                #region Managing Instances
                //Handling running instances
                bool createdNew;
                appMutex = new Mutex(true, MutexGUID, out createdNew);
                appWaitHandle = new EventWaitHandle(false, EventResetMode.AutoReset, EventMutex);

                GC.KeepAlive(this.appMutex);
                if (!createdNew)
                {
                    LogWriter.WriteLog("CradleWindowsAgent is already running, return");
                    appWaitHandle.Set();
                    Application.Current.Shutdown(0);
                    return;
                }
                
                #endregion
            }
            catch (Exception ex)
            {
                LogWriter.WriteLog("Is Mac Agent check ex- " + ex.Message);
            }

            _monitorThread = new Thread(new ThreadStart(MonitorAndRestart));
            _monitorThread.IsBackground = true;
            _monitorThread.Start();

            //CheckForUpdate();

            //Enable Firewall Manager
            // EnableFirewallManager();

            ShutdownMode = ShutdownMode.OnExplicitShutdown;
            wndMain = new MainWindow();
            wndMain.Show();
        }

        private void MonitorAndRestart()
        {
            LogWriter.WriteLog("MonitorAndRestart()");
            while (true)
            {
                try
                {
                    CheckAndRestartProcess("Watcher");
                }
                catch (Exception ex)
                {
                    // Log the exception (use a proper logging framework)
                    LogWriter.WriteLog("MonitorAndRestart ex- " + ex.ToString());
                }
                Thread.Sleep(3000); // Check every 3 seconds
            }
        }
        private void CheckAndRestartProcess(string processName)
        {
            try
            {
                Process[] processes = Process.GetProcessesByName(processName);
                if (processes.Length == 0)
                {
                
                    // Start the main application
                    string sLocalAppData = Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData);
                    string sPath = Path.Combine(sLocalAppData, "{9A025A4A-6FA2-43E0-B8BF-3337488F5314}");
                    sPath = Path.Combine(sPath, "executor.exe");
                            
                    LogWriter.WriteLog("sPath exe = " + sPath);
                            
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
                LogWriter.WriteLog("CheckAndRestartProcess () ex = " + ex.Message);
            }
        }
         

        private void EnableFirewallManager()
        {
            LogWriter.WriteLog("EnableFirewallManager()");
            FirewallManager firewallManager = new FirewallManager();

            try
            {
                // Ensure the firewall is enabled
                firewallManager.EnsureFirewallEnabled();

                // Allow outbound traffic to the current DNS servers
                firewallManager.AllowOutboundTrafficToCurrentDNS();

                // Allow outbound traffic to a specific URL
                firewallManager.AllowOutboundTrafficToUrl("jaininfotech.com");
                firewallManager.AllowOutboundTrafficToUrl("wisemaccare.com");

                // Add rule to block all outbound traffic
                firewallManager.BlockAllOutboundTraffic();

                Console.WriteLine("Firewall rules updated successfully.");

                // Keep the application running to maintain the filters
            }
            finally
            {
                // Clean up firewall rules
                ///firewallManager.RemoveFirewallRules();
            }
        }
        private async void CheckForUpdate()
        {
            LogWriter.WriteLog("CheckForUpdate()");

            await Task.Factory.StartNew(async () =>
            {
                try
                {
                    //check for App update
                    var updater = new AppUpdater();
                    var status = await updater.CheckForUpdate();
                    if (status)
                    {
                        var status1 = await updater.DownloadUpdate();

                        //Now let's Install the update
                        try
                        {
                            //Check if Download is correct  
                            string updateFilePath = System.IO.Path.Combine(Global.AppDataDirPath, "CMASetup.exe");
                            {
                                Application.Current.Shutdown(0);
                                Process.Start(updateFilePath, "/SILENT /VERYSILENT");
                            }
                        }
                        catch (Exception ex)
                        {
                            LogWriter.WriteLog(ex.Message);
                        }
                    }
                    else
                    {
                        return;
                    }
                }
                catch (Exception ex)
                {
                    LogWriter.WriteLog("CheckForUpdate() EX - " + ex.Message);
                }
            });
        }

    }
}
