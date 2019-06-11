using System;
using System.Collections.Concurrent;
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
        public string isdn_cc;
        public string isdn_ac;
        public string isdn_lc;
        public string ServerIPAndPort;
        public PersPortDataStore(string name, string pisdn_cc, string pisdn_ac, string pisdn_lc, string Ext,string pServerIPAndPort)
        {
            Name = name;
            isdn_cc = pisdn_cc;
            isdn_ac = pisdn_ac;
            isdn_lc = pisdn_lc;
            Extension = Ext;
            ServerIPAndPort = pServerIPAndPort;
        }
        public string GetPrefix()
        {
            if (isdn_cc == null || isdn_cc.Length == 0)
                return "";
            if (isdn_ac == null || isdn_ac.Length == 0)
                return "";
            if (isdn_lc == null || isdn_lc.Length == 0)
                return "";
            return "+" + isdn_cc + "(" + isdn_ac + ")" + isdn_lc;
        }
        //only return prefix if it is toggled in the UI
        public string GetPrefixIfShown(bool Force = false)
        {
            if(Force == true)
                return GetPrefix() + "-";
            if (App.Current != null && App.Current.MainWindow != null && (App.Current.MainWindow as MainWindow).ShowCannonical() == true)
                return GetPrefix() + "-";
            return "";
        }
    }

    public class PersPortManager
    {
        System.Timers.Timer UpdateTimer = null;
        DateTime LastRequestSentStamp = DateTime.Now.Subtract(new TimeSpan(24, 0, 0));
        Dictionary <string,List<PersPortDataStore>> ServerExtensions = new Dictionary<string, List<PersPortDataStore>>();
        long FetchStartTime;
        bool InterruptParsing = false;

        private static string GetPersportPath(string ServerIPAndPort)
        {
            if (ServerIPAndPort == null)
                ServerIPAndPort = "";
            else
                ServerIPAndPort = "_" + ServerIPAndPort.Replace(':','_').Replace('.','_');
            return Globals.GetFullAppPath("Database\\Persport" + ServerIPAndPort + ".txt");
        }

        public void Load()
        {
            //parse the file in the background. If server decides we need a new one, we will interrrupt parsing and reparse the new one
//            Task mytask2 = Task.Run(() => { ReParsePersportTXT(null); });
            Task mytask = Task.Run(() => { _Load(); });
        }

        private void _Load()
        {
            // wait for index cards and extension to load up. Also wait for a connection we can query
            while ((Globals.ConnectionManager == null || Globals.IndexCardsLoaded == false) && Globals.IsAppRunning == true)
                System.Threading.Thread.Sleep(100);

            if (Globals.IsAppRunning == false)
                return;
            Task mytask2 = Task.Run(() => { ReParsePersportTXT(Globals.ConnectionManager.GetPreferedServerIpAndPort()); });

            while ((Globals.ConnectionManager == null || Globals.ConnectionManager.HasAnyActiveConnection() == false || Globals.IndexCardsLoaded == false) && Globals.IsAppRunning == true)
                System.Threading.Thread.Sleep(100);

            if (Globals.IsAppRunning == false)
                return;

            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Async persport manager has started");
            //create a timer to periodically query extension forwarding status and issue callbacks to all cell cards on status change
#if DEBUG
            UpdateTimer = new System.Timers.Timer(1000);
#else
            UpdateTimer = new System.Timers.Timer(5 * 60 * 1000);
#endif
            UpdateTimer.Enabled = true;
            UpdateTimer.Elapsed += new ElapsedEventHandler(PeriodicStatusUpdate);
            PeriodicStatusUpdate(null, null);
        }

        public void ForceStatusUpdate()
        {
            LastRequestSentStamp = DateTime.Now.Subtract(new TimeSpan(24, 0, 0));
            PeriodicStatusUpdate(null, null);
        }

        private void PeriodicStatusUpdate(object source, ElapsedEventArgs arg)
        {
            //nothing to be done for now
            if (Globals.ConnectionManager == null || Globals.ConnectionManager.HasAnyActiveConnection() == false)
                return;

            // do not spam requests. Wait until we get a reply
            double MinutesPassedSinceLastCheck = DateTime.Now.Subtract(LastRequestSentStamp).TotalMinutes;
            if (MinutesPassedSinceLastCheck <= 5)
                return;
            LastRequestSentStamp = DateTime.Now;

            FetchStartTime = Environment.TickCount;
            //check if our pesrport.txt is up to date
            ConcurrentBag<ServerConnectionStatus> Connections = Globals.ConnectionManager.GetConnections();
            foreach (ServerConnectionStatus sc in Connections)
            {
                if (sc.PendingRemove == true || sc.nclient == null)
                    continue;
                sc.nclient.PacketBuilder.DbChecksumRequest();
            }
        }

        public void SaveChunkToFile(int ChunkIndex, byte[] buff, int offset, int count, string ServerIPAndPort)
        {
            FileStream fileStream;
            if (ChunkIndex != 0)
                fileStream = new FileStream(GetPersportPath(ServerIPAndPort), FileMode.Append, FileAccess.Write, FileShare.Read);
            else
                fileStream = new FileStream(GetPersportPath(ServerIPAndPort), FileMode.Create, FileAccess.Write, FileShare.Read);

            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Adding new persport chunk " + ChunkIndex.ToString() + ". Byte count " + count.ToString() + " Filepath : " + GetPersportPath(ServerIPAndPort));

            fileStream.Write(buff, offset, count);
            fileStream.Close();
        }

        public ushort GetChecksum(string ServerIPAndPort)
        {
            if (File.Exists(GetPersportPath(ServerIPAndPort)) == false)
                return 0;
            int XorChk16 = 0;
            FileStream fileStream = new FileStream(GetPersportPath(ServerIPAndPort), FileMode.Open, FileAccess.Read, FileShare.Read);
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

        public void ReParsePersportTXT(string ServerIPAndPort)
        {
            long StartTime = Environment.TickCount;

            //if we are parsing already, ditch all that data
            InterruptParsing = true;
            lock (this)
            {
                InterruptParsing = false;
                //ditch old list and create a new one
                ServerExtensions[ServerIPAndPort] = new List<PersPortDataStore>();
                //if there is nothing to parse, than skip parsing it :P
                if (File.Exists(GetPersportPath(ServerIPAndPort)) == false)
                    return;
                //open file for reading
                var lines = File.ReadLines(GetPersportPath(ServerIPAndPort));
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

                    string tline;
                    string Name = "";
                    if (line[0] != ',')
                    {
                        int i = line.IndexOf("*,");
                        if (i > 0)
                            Name = line.Substring(0, i);
                        tline = line.Substring(i+2);
                    }
                    else
                        tline = line.Substring(1);
/*                    int LastComma = line.LastIndexOf(',', line.Length - 2);
                    string Ext = line.Substring(LastComma + 1, line.Length - LastComma - 2);
                    */
                    string[] words = tline.Split(',');
                    string isdn_cc = words[0];
                    string isdn_ac = words[1];
                    string isdn_lc = words[2];
                    string Ext = words[4];
                    AddPersportData(Name, isdn_cc, isdn_ac, isdn_lc, Ext, ServerIPAndPort);
                }
            }
            //sort list to be extension based ordered
            ServerExtensions[ServerIPAndPort] = ServerExtensions[ServerIPAndPort].OrderBy(o => o.Extension).ToList();
            
            //all done. Curious how bad it was
            long Endtime = Environment.TickCount;

            //update UI with the newly received data
            foreach (var i in ServerExtensions[ServerIPAndPort])
                if (i.Name != "")
                    Globals.ExtensionManager.OnServerExtensionNameReceive(ServerIPAndPort, i.GetPrefix(), i.Extension, i.Name);

            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "(Re)Parsed Persport.txt in " + (StartTime - FetchStartTime) + " ms. Parsed in " + (Endtime - StartTime) + " ms");
        }

        private void AddPersportData(string Name, string isdn_cc, string isdn_ac, string isdn_lc, string Extension, string ServerIPAndPort)
        {
            if (Extension.Length == 0)
                return;
            if (Name == null)
                Name = "";
            //check if we already have it
            foreach (var i in ServerExtensions[ServerIPAndPort])
                if (i.Extension == Extension && i.ServerIPAndPort == ServerIPAndPort)
                    return;
            PersPortDataStore t = new PersPortDataStore(Name, isdn_cc, isdn_ac, isdn_lc, Extension, ServerIPAndPort);
            ServerExtensions[ServerIPAndPort].Add(t);
        }

 /*       public string GetServerExtensionName(string Extension)
        {
            foreach (var i in ServerExtensions)
                if (i.Extension == Extension)
                    return i.Name;
            return null;
        }*/

        public string GetServerExtensionPrefix(string Extension)
        {
            foreach (var i in ServerExtensions)
                foreach( var j in i.Value)
                    if (j.Extension == Extension)
                        return "+" + j.isdn_cc + "(" + j.isdn_ac + ")" + j.isdn_lc;
            return null;
        }

        public System.Collections.ObjectModel.ReadOnlyCollection<PersPortDataStore> GetServerExtensions()
        {
            //ugnly becase multi server support got added later
            List<PersPortDataStore> templist = new List<PersPortDataStore>();
            foreach (var i in ServerExtensions)
                foreach (var j in i.Value)
                    templist.Add(j);
            return new System.Collections.ObjectModel.ReadOnlyCollection<PersPortDataStore>(templist);
        }
    }
}
