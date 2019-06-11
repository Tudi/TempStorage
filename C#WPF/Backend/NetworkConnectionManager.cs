using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Net.Sockets;
using System.Runtime.Serialization.Formatters.Binary;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Threading;

namespace BLFClient.Backend
{
    public class ServerConnectionStatus
    {
        public ServerConnectionStatus(string pIP, int pPort, int pConfigFileIndex, int pEnabled, string pServerName)
        {
            IP = pIP;
            Port = pPort;
            ConfigFileIndex = pConfigFileIndex;
            Enabled = pEnabled;
            LastHeartBeatStamp = Environment.TickCount;
            nclient = null;
            PendingRemove = false;
            if(pServerName == null || pServerName == "")
                ServerName = pIP + ":" + pPort.ToString();
            else
                ServerName = pServerName;
        }
        public int Enabled;
        public string IP;
        public int Port;
        public int LastHeartBeatStamp;
        public NetworkClient nclient;
        public int ConfigFileIndex;
        public bool PendingRemove;
        public string ServerName;   // human version of the IP. Any string we can use for extensions...
        public string GetServerIPAndPort()
        {
            return IP + ":" + Port.ToString();
        }
    }

    /// <summary>
    /// Global resource to listen to connect, reconnect to the server
    /// Should provide a simple itnerface to obtain data from the server
    /// It is up to the implementation to be connection or connectionless
    /// </summary>
    public class NetworkConnectionManager
    {
        //periodically update the connection statuses
        Thread ConnectionStatusMonitor = null;
        Thread PacketParserThread = null;

        // so we can shut down this resource externally
//        bool IsAppRunning = true;

//        string ServerURL;
//        int ServerPort;
        bool IsConnectedStatus = false;
//        int LastHeartBeatStamp = Environment.TickCount;
// in case we have a connection based implementation ( TCPIP )
//        NetworkClient nclient = null;

        ConcurrentBag<ServerConnectionStatus> ServerConnections = new ConcurrentBag<ServerConnectionStatus>();

        public NetworkConnectionManager()
        {
            //create a thread to monitor connection status. If we are disconnected than we should reconnect
            ConnectionStatusMonitor = new Thread(new ThreadStart(MonitorConnectionStatus));
            ConnectionStatusMonitor.Start();
            ShowNoConnectionWindow();
        }

        ~NetworkConnectionManager()
        {
            //            IsAppRunning = false;
            foreach (ServerConnectionStatus sc in ServerConnections)
            {
                sc.PendingRemove = true;
                Disconnect(sc);
            }
            HideNoConnectionWindow();
        }

        public ConcurrentBag<ServerConnectionStatus> GetConnections()
        {
            return ServerConnections;
        }

        public NetworkClient GetCLient(string ServerIPAndPort)
        {
            foreach (ServerConnectionStatus sc in ServerConnections)
            {
                if (sc.PendingRemove == true)
                    continue;
                if (ServerIPAndPort == null || sc.IP + ":" + sc.Port.ToString() == ServerIPAndPort)
                    return sc.nclient;
            }
            return null;
        }

        public void UpdateConnectionsDetails(ObservableCollection<ServerConnectionRow> NewConnectionList)
        {
            //check whick connections we should remove from existing list
            foreach (ServerConnectionStatus sc in ServerConnections)
            {
                if (sc.PendingRemove == true)
                    continue;
                ServerConnectionRow scrExisting = null;
                foreach (ServerConnectionRow scr in NewConnectionList)
                    if(scr.IP == sc.IP && scr.Port == sc.Port)
                    {
                        scrExisting = scr;
                        break;
                    }
                //should remove from our connection list
                if (scrExisting == null)
                    sc.PendingRemove = true;
                //if connection is disabled, we will try to break an existing connection on next update
                if (scrExisting != null)
                {
                    if (scrExisting.Delete_ != 0)
                        sc.PendingRemove = true;
                    sc.Enabled = scrExisting.Enabled;
                    if (scrExisting.Name != null)
                        sc.ServerName = scrExisting.Name;
                    else
                        sc.ServerName = " ";
                }
            }

            //check for connections we should add
            bool RefreshPersports = false;
            foreach (ServerConnectionRow scr in NewConnectionList)
            {
                //no need to save invalid rows
                if (scr.IP == null || scr.IP == "" || scr.Port <= 0)
                    continue;
                ServerConnectionStatus Existingsc = null;
                foreach (ServerConnectionStatus sc in ServerConnections)
                    if (scr.IP == sc.IP && scr.Port == sc.Port && sc.PendingRemove == false)
                    {
                        Existingsc = sc;
                        break;
                    }
                if (Existingsc != null)
                    continue;
                //create a new connection
                ServerConnections.Add(new ServerConnectionStatus(scr.IP, scr.Port, -1, scr.Enabled, scr.Name));
                //make sure we will refresh the persport for this server as soon as possible
                RefreshPersports = true;
            }
            if (RefreshPersports == true && Globals.persPortManager != null)
                Globals.persPortManager.ForceStatusUpdate();

        }

        public bool HasAnyActiveConnection()
        {
            foreach (ServerConnectionStatus sc in ServerConnections)
                if (sc.nclient != null && sc.PendingRemove == false && sc.nclient.IsConnected())
                    return true;
//            if (nclient != null && nclient.IsConnected() == true)
//                return true;
            return false;
        }

        private void ShowNoConnectionWindow()
        {
            //shutting down, don't do wnaything
            if (Globals.IsAppRunning == false)
                return;
            //maybe connection got reestablished by the other thread while we waited for load
            if (HasAnyActiveConnection())
                return;
            if (Globals.AppVars.NoConnectionWindow != null)
                return;
            if (Globals.IndexCardsLoaded == false)
                return;
            App.Current.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)(() =>
            {
                if (Application.Current.Windows.OfType<NoConnection>().Any() == false)
                {
                    if (Globals.WindowLoaded == true)
                    {
                        Globals.AppVars.NoConnectionWindow = new NoConnection();
                        Globals.AppVars.NoConnectionWindow.Show();
                    }
                    else
                    {
                        Task mytask = Task.Run(() => { Thread.Sleep(500); ShowNoConnectionWindow(); });
                    }
                }
            }));
        }

        private void HideNoConnectionWindow()
        {
            if (Globals.AppVars.NoConnectionWindow == null)
                return;

            NoConnection WindowToClose = Globals.AppVars.NoConnectionWindow;
            Globals.AppVars.NoConnectionWindow = null;
            if (App.Current == null)
            {
                return;
            }
            App.Current.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)(() =>
            {
                try
                {
                    WindowToClose.Close();
                }
                catch { };           
            }));
        }

        private void ShowServerSettingsWindow()
        {
            if (Globals.IsAppRunning == false || Globals.ConfigLoaded == false)
                return;
            App.Current.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)(() =>
            {
                MainWindow MainObject = (MainWindow)App.Current.MainWindow;
                if (MainObject != null && (Globals.IsAppRunning != true || Globals.IndexCardsLoaded == false))
                    return;
                if (Application.Current.Windows.OfType<BLFServerConfig>().Any() == false)
                {
                    new BLFServerConfig().Show();
                }
            }));
        }
/*
        public void OnHeartbeatReceived()
        {
            LastHeartBeatStamp = Environment.TickCount;
        }*/

        public void Shutdown()
        {
            //            IsAppRunning = false;
            //            if(ConnectionStatusMonitor != null)
            //                ConnectionStatusMonitor.Abort();
            //            if (PacketParserThread != null)
            //                PacketParserThread.Abort();
            foreach (ServerConnectionStatus sc in ServerConnections)
            {
                sc.PendingRemove = true;
                Disconnect(sc);
            }
        }

        /// <summary>
        /// Connection manager will try to periodically connect to this specific BLF server
        /// </summary>
        /// <param name="Url"></param>
        /// <param name="port"></param>
        public void SetConnectionDetails(string Url, int port, int Index = 0, int Enabled = 1, string ServerName = "")
        {
            if (Url == null || Url.Length == 0)
                return;
            //do we already have this IP:Port combo ?
            foreach (ServerConnectionStatus s in ServerConnections)
                if (s.IP == Url && s.Port == port && s.PendingRemove == false)
                    return;
            //on periodic update, a new connection will be created
            ServerConnections.Add(new ServerConnectionStatus(Url, port, Index, Enabled, ServerName));
            //force creating a new connection if connection details have changed
//            if (ServerURL != Url || ServerPort != port)
//                Disconnect();
//            ServerURL = Url;
//            ServerPort = port;
        }

        private void UpdateUIConnectionStatus()
        {
            //if we already reported this status, nothing to do here
            if (IsConnectedStatus == false && HasAnyActiveConnection() == false)
                return;
            if (IsConnectedStatus == true && HasAnyActiveConnection() == true)
                return;

            //set new status
            if (HasAnyActiveConnection())
                IsConnectedStatus = true;
            else
                IsConnectedStatus = false;

            if (App.Current == null )
                return;
            App.Current.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)(() =>
            {
                MainWindow MainObject = (MainWindow)App.Current.MainWindow;
                if (MainObject != null && Globals.IsAppRunning == true)
                {
                    MainObject.OnServerConnectionChanged(IsConnectedStatus);
                }
            }));
         }

        /// <summary>
        /// Periodically check the status of our client server connection. Update UI accordingly
        /// </summary>
        private void MonitorConnectionStatus()
        {
            while(Globals.IsAppRunning)
            {
                //remove connections that are pending to be removed ?
                int ConnectionSettingsExist = 0;
                foreach (ServerConnectionStatus sc in ServerConnections)
                {
                    if (sc.PendingRemove == true && sc.nclient != null)
                        Disconnect(sc);

                    if (sc.PendingRemove == true)
                        continue;

                    //if we did not receive a heartbeat for too much time, we should try to reconnect. Or maybe say that server is down ? 
                    if (sc.LastHeartBeatStamp + (long)ServerPacketStatus.HeartBeatTimeOutMS < Environment.TickCount)
                        Disconnect(sc);

                    //update UI status if we managed to connect to the server
                    if (Globals.IsAppRunning == true)
                        UpdateUIConnectionStatus();

                    //can we change UI status ? This might take a while, that is why we have the double UI update in 1 loop
                    if (sc.nclient == null || sc.nclient.IsConnected() == false && Globals.IsAppRunning == true)
                        CreateNewConnection(sc);

                    //update UI status if we managed to connect to the server
                    if (Globals.IsAppRunning == true)
                        UpdateUIConnectionStatus();

                    ConnectionSettingsExist++;
                }

                if (ConnectionSettingsExist == 0)
                    ShowServerSettingsWindow();

                //should create a config for the update interval
                System.Threading.Thread.Sleep(1000);
            }
        }

        /// <summary>
        /// Periodically check packet buffer and try to parse + handle packets
        /// </summary>
        private void AsyncParsePackets()
        {
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Async packet parser has started");
            while (Globals.IsAppRunning)
            {
                int AnyPacketsParsed = 0;
                foreach (ServerConnectionStatus sc in ServerConnections)
                {
                    if (sc.PendingRemove == true)
                        continue;
                    if (sc.nclient != null)
                    {
                        byte[] reply = sc.nclient.ReadPacket();
                        if (reply != null)
                        {
                            sc.LastHeartBeatStamp = Environment.TickCount;
                            sc.nclient.PacketInterpreter.InterpretMessage(reply);
                            AnyPacketsParsed++;
                        }
                    }
                }
                //CPU burner ? Should use blocking sockets and multiple threads !
                if(AnyPacketsParsed == 0)
                    Thread.Sleep(1);
            }
            PacketParserThread = null;
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Async packet parser has exited");
        }

        /// <summary>
        /// Disconnect existing connections to the server
        /// </summary>
        void Disconnect(ServerConnectionStatus css)
        {
            //stop reading and writing
            if (css.nclient != null)
            {
                //update the context menu of each extension to allow call forwarding clicking
                Globals.ExtensionManager.OnConnectionChanged(css.GetServerIPAndPort(), false);

                Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Disconnected from server " + css.IP);
                css.nclient.Disconnect();
                css.nclient = null;
            }
            //notify user that we no longer get updates from the server
            ShowNoConnectionWindow();
        }

        /// <summary>
        /// When connection settings change, this function should be called to create a new connection
        /// </summary>
        public void CreateNewConnection(ServerConnectionStatus css)
        {
            //make sure we ditch old data
            Disconnect(css);

            if (Globals.IsAppRunning == false)
                return;

            //need connection details in order to create a new connection
            if (css.IP == null || css.IP.Length == 0 || css.PendingRemove == true)
            {
                return;
            }

            //create a client connection that should persist while the UI is alive
            NetworkClient TClient = new NetworkClient(css.IP, css.Port);
            TClient.ConnectToServer(css.IP, css.Port);
            if (TClient.IsConnected() == false)
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Failed to created new server connection to " + css.IP + ":" + css.Port.ToString());
                TClient = null;
                return; //failed to create a connection
            }

            //no more need to show user that there is no connection
            HideNoConnectionWindow();

            //if it is good to use, we use it
            css.nclient = TClient;
            css.LastHeartBeatStamp = Environment.TickCount;

            if (PacketParserThread == null)
            {
                PacketParserThread = new Thread(new ThreadStart(AsyncParsePackets));
                PacketParserThread.Start();
            }

            //update the context menu of each extension to allow call forwarding clicking
            Globals.ExtensionManager.OnConnectionChanged(css.GetServerIPAndPort(), true);

            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Created new server connection to " + css.IP.ToString() + ":" + css.Port.ToString());
        }

        public string GetPreferedServerIpAndPort()
        {
            //try to return a valid combo
            foreach (ServerConnectionStatus sc in ServerConnections)
                if (sc.nclient != null && sc.PendingRemove == false && sc.nclient.IsConnected())
                    return sc.GetServerIPAndPort();
            // nothing is online for now. Return the first one
            foreach (ServerConnectionStatus sc in ServerConnections)
                if (sc.IP != null && sc.IP.Length>0 && sc.PendingRemove == false)
                    return sc.GetServerIPAndPort();
            return null;
        }
        /// <summary>
        /// Interface function. Made in case multiple server connections will be possible. Not the case for now
        /// </summary>
        /// <param name="pkt"></param>
        /// <returns></returns>
        public bool SendPacket(object pkt)
        {
            //search for first valid connection
            //!!! need to rewrite this later to be able to send only to a specific connection
            foreach (ServerConnectionStatus sc in ServerConnections)
                if (sc.nclient != null && sc.PendingRemove == false && sc.nclient.IsConnected())
                {
                    sc.nclient.SendPacket(pkt);
                    return true;
                }
            return false;
        }

        /// <summary>
        /// Fetch a packet
        /// </summary>
        /// <returns></returns>
/*        public byte [] ReadPacket()
        {
            return nclient.ReadPacket();
        }*/
/*
        public bool IsConnected()
        {
            if (nclient != null && nclient.IsConnected() == true)
                return true;
            return false;
        }*/

/*        public ushort GetConnectionChecksum()
        {
            if (nclient != null && nclient.IsConnected() == true)
                return nclient.GetConnectionChecksum();
            return 0;
        }*/
    }
}
