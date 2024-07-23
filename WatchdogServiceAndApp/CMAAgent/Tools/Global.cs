using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace CMAAgent.Tools
{
    public static class Global
    {
        public static Version AppVersion { get; set; }
        public static string CommonAppDataDirPath
        {
            get
            {
                var path = Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData) + "\\" + "CMA";
                if (!Directory.Exists(path))
                {
                    Directory.CreateDirectory(path);
                    DirectoryInfo dirInfo = new DirectoryInfo(path);
                    dirInfo.Attributes = FileAttributes.Hidden;
                }
                return path;
            }
        }
        public static string CommonAppDataDirLogsPath
        {
            get
            {
                var path = Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData) + "\\" + "CMA\\Logs";
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
