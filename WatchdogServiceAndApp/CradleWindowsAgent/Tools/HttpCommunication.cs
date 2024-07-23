using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Net;
using System.Threading.Tasks;
 

namespace CradleWindowsAgent.Tools
{
    class HttpCommunication : WebClient
    {
        public event EventHandler<DownloadProgressChangedEventArgs> WebClientDownloadProgressHandler;
        public event EventHandler<AsyncCompletedEventArgs> AsyncCompletedEventArgsHandler;
        public event EventHandler<DownloadFileEventArgs> WebClientDownloadProgressHandlerTimeOut;
 
        public async Task<bool> Download(string URL, string Location, bool shouldReportProgress)
        {
            try
            {
                if (shouldReportProgress)
                {
                    this.DownloadProgressChanged += Client_DownloadProgressChanged;
                    this.DownloadFileCompleted += Client_DownloadFileCompleted;
                }


                await DownloadFileTaskAsync(URL, Location);
                return true;
            }
            catch (WebException ex)
            {
                LogWriter.WriteLog("HttpCommunication Download ex -" + ex.Message);
                if (ex != null)
                {
                    if (!string.IsNullOrEmpty(ex.Message))
                    {
                        Debug.WriteLine(ex.Message);
                    }
                }
            }
            return false;
        }

        private void Client_ProgressChanged(long? totalFileSize, long totalBytesDownloaded, double? progressPercentage)
        {
            WebClientDownloadProgressHandlerTimeOut?.Invoke(this, new DownloadFileEventArgs { BytesDownloaded = totalBytesDownloaded });
        }

        public async Task<string> DownloadStringData(string URL, string Location, bool shouldReportProgress)
        {
            try
            {
                WebClient client = new WebClient();

                if (!shouldReportProgress)
                {
                    client.DownloadProgressChanged += Client_DownloadProgressChanged;
                    client.DownloadFileCompleted += Client_DownloadFileCompleted;
                }

                return await client.DownloadStringTaskAsync(URL);
            }
            catch (WebException ex)
            {
                LogWriter.WriteLog("DownloadStringData WebException -" + ex.Message);
            }
            catch (Exception ex)
            {
                LogWriter.WriteLog("DownloadStringData ex -" + ex.Message);
            }
            return "";
        }

        private void Client_DownloadFileCompleted(object sender, System.ComponentModel.AsyncCompletedEventArgs e)
        {
            AsyncCompletedEventArgsHandler?.Invoke(this, e);
        }

        private void Client_DownloadProgressChanged(object sender, DownloadProgressChangedEventArgs e)
        {
            WebClientDownloadProgressHandler?.Invoke(this, e);
        }

    }

    public class DownloadFileEventArgs : EventArgs
    {
        public double BytesDownloaded;
    }
}
