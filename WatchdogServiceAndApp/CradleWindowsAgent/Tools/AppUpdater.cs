using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace CradleWindowsAgent.Tools
{
    public class AppUpdater
    {
        public event EventHandler<DownloadProgressChangedEventArgs> DownloadProgressHandler;
        public event EventHandler<AsyncCompletedEventArgs> AsyncCompletedEventArgsHandler;

        private string _UpdateFolderPath;
        public string _UpdateHash;
        private string _DownloadUpdateURL;
        private string _DownloadUpdateFileName;
        public bool ForceUpdate;

        public AppUpdater()
        {
            this._UpdateFolderPath = Global.AppDataDirPath;
        }

        public async Task<bool> CheckForUpdate()
        {
            LogWriter.WriteLog("Task<bool> CheckForUpdate()");
            try
            {
                string iniFileURL = "http://jaininfotech.com/temp/cma/update.ini";
                string updateINIPath = Path.Combine(Global.AppDataDirPath, "update.ini");

                HttpCommunication http = new HttpCommunication();
                if (await http.Download(iniFileURL, updateINIPath, false))
                {
                    //If update is available the True will be returned;
                    return ReadUpdateDetails(updateINIPath);
                }
                return false;
            }
            catch (Exception ex)
            {
                LogWriter.WriteLog("Task<bool> CheckForUpdate() Exception - " + ex.Message);
                return false;
            }
        }

        private bool ReadUpdateDetails(string updateINIPath)
        {
            LogWriter.WriteLog("ReadUpdateDetails()");

            try
            {
                INIParser objIniParser = new INIParser(updateINIPath);
                string section = "application";
                string versionInsideINIFile = objIniParser.ReadValue(section, "program_version");

                _DownloadUpdateFileName = "CMASetup.exe";

                _DownloadUpdateURL = objIniParser.ReadValue(section, "program_url");

                Version updateVersion = null;
                Version.TryParse(versionInsideINIFile, out updateVersion);
                if (updateVersion != null && Global.AppVersion != null && updateVersion > Global.AppVersion)
                {
                    LogWriter.WriteLog("Update available");
                    return true;
                }
            }
            catch (Exception ex)
            {
                LogWriter.WriteLog("ReadUpdateDetails() EX - " + ex.Message);
            }

            return false;
        }

        public async Task<bool> DownloadUpdate()
        {
            HttpCommunication http = new HttpCommunication();
            if (!ForceUpdate)
            {
                http.WebClientDownloadProgressHandler += Http_DownloadProgressChanged;
            }


            http.AsyncCompletedEventArgsHandler += Http_AsyncCompletedEventArgsHandler;

            var res = await http.Download(_DownloadUpdateURL, _UpdateFolderPath + "\\" + _DownloadUpdateFileName, true);
            if (!res)
                return false;

            LogWriter.WriteLog("Update Downloaded successfully");
            return true;
        }

        private void Http_AsyncCompletedEventArgsHandler(object sender, System.ComponentModel.AsyncCompletedEventArgs e)
        {
            AsyncCompletedEventArgsHandler?.Invoke(this, e);
        }

        private void Http_DownloadProgressChanged(object sender, System.Net.DownloadProgressChangedEventArgs e)
        {
            DownloadProgressHandler?.Invoke(this, e);
        }
    }
}
