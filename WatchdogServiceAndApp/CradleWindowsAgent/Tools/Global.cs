using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace CradleWindowsAgent.Tools
{
    public static class Global
    {
        public static Version AppVersion { get; set; }
        public static string AppDataDirPath
        {
            get
            {
                var path = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData) + "\\" + "CMA";
                if (!Directory.Exists(path))
                {
                    Directory.CreateDirectory(path);
                    DirectoryInfo dirInfo = new DirectoryInfo(path);
                    dirInfo.Attributes = FileAttributes.Hidden;
                }
                return path;
            }
        }
        public static string AppDataDirLogsPath
        {
            get
            {
                var path = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData) + "\\" + "CMA\\Logs";
                if (!Directory.Exists(path))
                {
                    Directory.CreateDirectory(path);
                    DirectoryInfo dirInfo = new DirectoryInfo(path);
                    dirInfo.Attributes = FileAttributes.Hidden;
                }
                return path;
            }
        }

        public static string GetCurrentAppVersion()
        {
            LogWriter.WriteLog("GetCurrentAppVersion()");
            try
            {
                var currentVersion = Assembly.GetEntryAssembly().GetName().Version;
                AppVersion = currentVersion;
                return currentVersion.ToString();
            }
            catch (Exception ex)
            {
                LogWriter.WriteLog("GetCurrentAppVersion() EX- " + ex.Message);
            }
            return "";
        }
    }
}
