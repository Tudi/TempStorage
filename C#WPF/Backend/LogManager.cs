using Microsoft.Win32;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Timers;
using System.Windows;

namespace BLFClient.Backend
{
    public class LogManager
    {
        public enum LogLevels
        {
            LogFlagNone = 0,
            LogFlagInfo = 1,
            LogFlagError = 2,
            LogFlagNetwork = 4,
            LogFlagUI = 8,
            LogFlagCallForwarding = 16,
            LogLevelDebug = LogFlagInfo | LogFlagError | LogFlagNetwork | LogFlagUI | LogFlagCallForwarding,
            LogLevelAll = 0xFF
        }

        private LogLevels LogToFile;
        private LogLevels LogToScreen;
        private BlockingCollection<string> WriteQueue = new BlockingCollection<string>();
        Timer UpdateTimer;
        private string FileName;

        public LogManager()
        {
#if DEBUG
            LogToFile = LogLevels.LogLevelDebug;
#else
            LogToFile = LogLevels.LogFlagNone;
#endif
            LogToScreen = LogLevels.LogFlagNone;
            UpdateTimer = new Timer(1000);
            UpdateTimer.Enabled = true; // do not trigger the update event until we become visible
            UpdateTimer.Elapsed += new ElapsedEventHandler(PeriodicFlushMessageQueue);

            try
            {
                using (RegistryKey key = Registry.LocalMachine.OpenSubKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\Unify\\BLF-Win 3.0 Client\\error"))
                {
                    if (key != null)
                    {
                        LogToFile |= LogLevels.LogFlagError;
                    }
                }
            }catch {}

            FileName = Globals.GetFullAppPath("Logs\\");
            FileName += DateTime.Now.Year.ToString() + ".";
            FileName += DateTime.Now.Month.ToString() + ".";
            FileName += DateTime.Now.Day.ToString() + "_";
            FileName += DateTime.Now.Hour.ToString() + ".";
            FileName += DateTime.Now.Minute.ToString() + ".";
            FileName += DateTime.Now.Second.ToString();
        }

        public void SetFileLogLevelsFromConfig(string config)
        {
            //sanity check
            if (config == null || config.Length == 0)
                return;
            //clear old values
            LogToFile = LogLevels.LogFlagNone;

            if (config.Contains(LogLevels.LogFlagError.ToString()))
                LogToFile |= LogLevels.LogFlagError;
            if (config.Contains(LogLevels.LogFlagInfo.ToString()))
                LogToFile |= LogLevels.LogFlagInfo;
            if (config.Contains(LogLevels.LogFlagNetwork.ToString()))
                LogToFile |= LogLevels.LogFlagNetwork;
            if (config.Contains(LogLevels.LogFlagUI.ToString()))
                LogToFile |= LogLevels.LogFlagUI;
            if (config.Contains(LogLevels.LogLevelAll.ToString()))
                LogToFile |= LogLevels.LogLevelAll;
        }

        public void LogString(LogLevels Severity, string What, [CallerFilePath] string filePath = "", [CallerLineNumber] int lineNumber = 0)
        {
            //log to screen ?
            if(((int)LogToScreen & (int)Severity) != 0)
            {
                MessageBox.Show(What);
            }
            //log to file
            if (((int)LogToFile & (int)Severity) != 0)
            {
                //this should be collected and written to file from time to time to reduce lag
                StringBuilder sb = new StringBuilder();
                sb.Append("Severity flag : " + Severity.ToString() + "\n");
                sb.Append("File : " + filePath + ":" + lineNumber.ToString() + "\n");
                sb.Append( Environment.TickCount.ToString() + " Msg : " + What + "\n\n");
                WriteQueue.Add(sb.ToString());
                //should run this in a thread
//                PeriodicFlushMessageQueue();
            }
        }

        public void PeriodicFlushMessageQueue(object source, ElapsedEventArgs arg)
        {
            UpdateTimer.Stop();
            while (WriteQueue.Count != 0)
            {
                string buffer = WriteQueue.Take();
                bool ManagedToWrite; //because our PC at work is so slow even log write fails from time to time
                do
                {
                    ManagedToWrite = true;
                    try
                    {
                        File.AppendAllText(FileName, buffer);
                    }
                    catch
                    {
                        ManagedToWrite = false;
                    }
                } while (ManagedToWrite == false);
            }
            UpdateTimer.Start();
        }
    }
}
