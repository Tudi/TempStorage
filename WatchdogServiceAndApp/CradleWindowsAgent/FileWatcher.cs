using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Threading;

namespace CradleWindowsAgent
{
    class FileWatcher
    {
        static DateTime FSLastRaised;
        static FileSystemWatcher FSWatcher;
        FileWatcher()
        {

        }
        
        public static void StartWatcher(string sFolderPath)
        {
            FSWatcher = new FileSystemWatcher(sFolderPath, "*.*");
            FSWatcher.BeginInit();
            FSWatcher.NotifyFilter = NotifyFilters.Attributes
                                 | NotifyFilters.CreationTime
                                 | NotifyFilters.DirectoryName
                                 | NotifyFilters.FileName
                                 | NotifyFilters.LastAccess
                                 | NotifyFilters.LastWrite
                                 | NotifyFilters.Security
                                 | NotifyFilters.Size;

            FSWatcher.EnableRaisingEvents = true;
            FSWatcher.IncludeSubdirectories = true;
            FSWatcher.Changed += FS_OnChanged;
            FSWatcher.Created += FS_OnCreated;
            FSWatcher.Deleted += FS_OnDeleted;
            FSWatcher.Renamed += FS_OnRenamed;
            FSWatcher.Error += OnError;

            FSWatcher.EndInit();
        }

        public static void StopTracking()
        {
            FSWatcher.EnableRaisingEvents = false;

            FSWatcher.Changed -= FS_OnChanged;
            FSWatcher.Created -= FS_OnCreated;
            FSWatcher.Deleted -= FS_OnDeleted;
            FSWatcher.Renamed -= FS_OnRenamed;

            FSWatcher.Dispose();
        }
        private static void OnError(object sender, ErrorEventArgs e) =>
        Console.WriteLine(e.GetException());

        #region file created
        private static void FS_OnCreated(object fscreated, FileSystemEventArgs e)
        {
            try
            {
                if (DateTime.Now.Subtract(FSLastRaised).TotalMilliseconds > 1000)
                {
                    //to get the newly created file name and extension and also the name of the event occured in the watching folder
                    string sCreatedFileName = e.FullPath;
                    FileInfo CreatedFile = new FileInfo(sCreatedFileName);
                    string sExtension = CreatedFile.Extension;
                    string sEventOccured = e.ChangeType.ToString();

                    //To note the time of event occured
                    FSLastRaised = DateTime.Now;
                    System.Threading.Thread.Sleep(10);

                    ((App)(Application.Current)).wndMain.AddToList("Newly Created File Name: " + sCreatedFileName + "; Event Occured: " + sEventOccured + "; Created Time: " + DateTime.Now.ToString());
                }

            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
            finally
            {

            }

        }

        #endregion
        #region file changed
        private static void FS_OnChanged(object fschanged, FileSystemEventArgs e)
        {
            if (e.ChangeType != WatcherChangeTypes.Changed)
            {
                return;
            }
            //to get the changed file name and extension and also the name of the event occured in the watching folder
            string sCreatedFileName = e.Name;
            FileInfo CreatedFile = new FileInfo(sCreatedFileName);
            string sExtension = CreatedFile.Extension;
            string sEventOccured = e.ChangeType.ToString();

            if (DateTime.Now.Subtract(FSLastRaised).TotalMilliseconds > 1000)
            {
                FSLastRaised = DateTime.Now;
                try
                {
                    System.Threading.Thread.Sleep(100);
                    ((App)(Application.Current)).wndMain.AddToList("Changed File Name: " + e.FullPath + "; Event Occured: " + e.ChangeType.ToString() + "; Time of Modification: " + DateTime.Now.ToString());
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }
            }
        }
        #endregion
        #region file renamed

        private static void FS_OnRenamed(object fschanged, RenamedEventArgs e)
        {
            try
            {
                if (DateTime.Now.Subtract(FSLastRaised).TotalMilliseconds > 1000)
                {
                    FSLastRaised = DateTime.Now;
                    System.Threading.Thread.Sleep(100);

                    string OldFilePath = e.OldFullPath;
                    ((App)(Application.Current)).wndMain.AddToList("Renamed Object New Name: " + e.FullPath + "; Oldname: " + e.OldFullPath + "; Event Occured: " + e.ChangeType.ToString() + "; Time of Rename: " + DateTime.Now.ToString());
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }
        #endregion
        #region file Deleted

        private static void FS_OnDeleted(object fschanged, FileSystemEventArgs e)
        {
            try
            {
                if (DateTime.Now.Subtract(FSLastRaised).TotalMilliseconds > 10)
                {
                    FSLastRaised = DateTime.Now;
                    System.Threading.Thread.Sleep(100);
                    ((App)Application.Current).wndMain.AddToList("Deleted Object Name: " + e.FullPath + "; Event Occured: " + e.ChangeType.ToString() + "; Time of Deletion: " + DateTime.Now.ToString());
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }
        #endregion
    }
}
