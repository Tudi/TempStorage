using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Timers;

namespace BLFClient.Backend
{
    /// <summary>
    /// Store the forward status of a specific extension
    /// </summary>
    public class PersPortDataStore
    {
        public string Extension;
        public string Name;

        public PersPortDataStore(string Ext, string name)
        {
            Extension = Ext;
            Name = name;
        }
    }

    public class PersPortManager
    {
        System.Timers.Timer UpdateTimer = null;
        DateTime LastRequestSentStamp = DateTime.Now.Subtract(new TimeSpan(24, 0, 0));
        List<PersPortDataStore> ServerExtensions = new List<PersPortDataStore>();
        long FetchStartTime;
        bool InterruptParsing = false;

        private static string GetPersportPath()
        {
            return Globals.GetFullAppPath("Database\\Persport.txt");
        }

        public void Load()
        {
            //parse the file in the background. If server decides we need a new one, we will interrrupt parsing and reparse the new one
            Task mytask2 = Task.Run(() => { ReParsePersportTXT(); });
            Task mytask = Task.Run(() => { _Load(); });
        }

        private void _Load()
        {
            // wait for index cards and extension to load up. Also wait for a connection we can query
            while (Globals.ConnectionManager == null || Globals.ConnectionManager.IsConnected() == false || Globals.IndexCardsLoaded == false)
                System.Threading.Thread.Sleep(100);

            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Async extension forward status manager has started");
            //create a timer to periodically query extension forwarding status and issue callbacks to all cell cards on status change
#if DEBUG
            UpdateTimer = new System.Timers.Timer(1000);
#else
            UpdateTimer = new System.Timers.Timer(5 * 60 * 1000);
#endif
            UpdateTimer.Enabled = true;
            UpdateTimer.Elapsed += new ElapsedEventHandler(PeriodicStatusUpdate);
        }

        private void PeriodicStatusUpdate(object source, ElapsedEventArgs arg)
        {
            //nothing to be done for now
            if (Globals.ConnectionManager == null || Globals.ConnectionManager.IsConnected() == false)
                return;

            // do not spam requests. Wait until we get a reply
            if (DateTime.Now.Subtract(LastRequestSentStamp).TotalMinutes <= 5)
                return;
            LastRequestSentStamp = DateTime.Now;

            FetchStartTime = Environment.TickCount;
            //check if our pesrport.txt is up to date
            NetworkClientBuildPacket.DbChecksumRequest();
        }

        public void SaveChunkToFile(bool Reset, byte[] buff, int offset, int count)
        {
            FileStream fileStream;
            if (Reset != true)
                fileStream = new FileStream(GetPersportPath(), FileMode.Append, FileAccess.Write, FileShare.Read);
            else
                fileStream = new FileStream(GetPersportPath(), FileMode.Create, FileAccess.Write, FileShare.Read);
            fileStream.Write(buff, offset, count);
            fileStream.Close();
        }

        public ushort GetChecksum()
        {
            if (File.Exists(GetPersportPath()) == false)
                return 0;
            int XorChk16 = 0;
            FileStream fileStream = new FileStream(GetPersportPath(), FileMode.Open, FileAccess.Read, FileShare.Read);
            if (fileStream.Length == 0)
                return 0;
            //calc CRC
            byte[] buf = new byte[2];
            while (fileStream.Read(buf, 0, 2) > 0)
            {
                int Temp = ((((int)buf[1]) << 8) | ((int)buf[0]));
                XorChk16 = (XorChk16 ^ Temp);
                buf[0] = 0;
                buf[1] = 0;
            }
            fileStream.Close();
            return (ushort)XorChk16;
        }

        public void ReParsePersportTXT()
        {
            long StartTime = Environment.TickCount;

            //if we are parsing already, ditch all that data
            InterruptParsing = true;
            lock (this)
            {
                InterruptParsing = false;
                //ditch old list and create a new one
                ServerExtensions = new List<PersPortDataStore>();
                //if there is nothing to parse, than skip parsing it :P
                if (File.Exists(GetPersportPath()) == false)
                    return;
                //open file for reading
                var lines = File.ReadLines(GetPersportPath());
                int SkipLines = 1;
                foreach (var line in lines)
                {
                    //server probably said our crc is wrong and we should refetch from serer again
                    if (InterruptParsing == true)
                    {
                        InterruptParsing = false;
                        break;
                    }
                    //first row of the CVC file is the column names
                    if(SkipLines > 0)
                    {
                        SkipLines--;
                        continue;
                    }
                    string Name = "";
                    if (line[0] != ',')
                    {
                        int i = line.IndexOf("*,");
                        if (i > 0)
                            Name = line.Substring(0, i);
                    }
                    int LastComma = line.LastIndexOf(',', line.Length - 2);
                    string Ext = line.Substring(LastComma + 1, line.Length - LastComma - 2);
                    AddPersportData(Ext, Name);
                }
            }
            //sort list to be extension based ordered
            ServerExtensions = ServerExtensions.OrderBy(o => o.Extension).ToList();
            //all done. Curious how bad it was
            long Endtime = Environment.TickCount;
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Refetched from server Persport.txt in " + (StartTime - FetchStartTime) + " ms. Parsed in " + (Endtime - StartTime) + " ms");
        }

        private void AddPersportData(string Extension, string Name)
        {
            if (Extension.Length == 0)
                return;
            if (Name == null)
                Name = "";
            //check if we already have it
            foreach (var i in ServerExtensions)
                if (i.Extension == Extension)
                    return;
            PersPortDataStore t = new PersPortDataStore(Extension, Name);
            ServerExtensions.Add(t);

            Globals.ExtensionManager.OnServerExtensionNameReceive(Extension, Name);
        }

        public string GetServerExtensionName(string Extension)
        {
            foreach (var i in ServerExtensions)
                if (i.Extension == Extension)
                    return i.Name;
            return null;
        }
       
        public System.Collections.ObjectModel.ReadOnlyCollection<PersPortDataStore> GetServerExtensions()
        {
            return new System.Collections.ObjectModel.ReadOnlyCollection<PersPortDataStore>(ServerExtensions);
        }
    }
}
