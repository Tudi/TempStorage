using System;
using System.Collections.Generic;
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
    /// <summary>
    /// Global resource to listen to connect, reconnect to the server
    /// Should provide a simple itnerface to obtain data from the server
    /// It is up to the implementation to be connection or connectionless
    /// </summary>
    public class NetworkConnectionManager
    {
        // in case we have a connection based implementation ( TCPIP )
        NetworkClient nclient = null;

        //periodically update the connection statuses
        Thread ConnectionStatusMonitor = null;
        Thread PacketParserThread = null;

        // so we can shut down this resource externally
        bool IsAppRunning = true;

        string ServerURL;
        int ServerPort;
        bool IsConnectedStatus = false;
        int LastHeartBeatStamp = Environment.TickCount;

        public NetworkConnectionManager()
        {
            //create a thread to monitor connection status. If we are disconnected than we should reconnect
            ConnectionStatusMonitor = new Thread(new ThreadStart(MonitorConnectionStatus));
            ConnectionStatusMonitor.Start();
            ShowNoConnectionWindow();
        }

        ~NetworkConnectionManager()
        {
            IsAppRunning = false;
            Disconnect();
            HideNoConnectionWindow();
        }

        private void ShowNoConnectionWindow()
        {
            //shutting down, don't do wnaything
            if (IsAppRunning == false)
                return;
            //maybe connection got reestablished by the other thread while we waited for load
            if (nclient != null && nclient.IsConnected() == true)
                return;
            if (Globals.AppVars.NoConnectionWindow != null)
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
            if (IsAppRunning == false)
                return;
            App.Current.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)(() =>
            {
                MainWindow MainObject = (MainWindow)App.Current.MainWindow;
                if (MainObject != null && (Globals.IsAppRunning != true || Globals.IndexCardsLoaded == false))
                    return;
                if (Application.Current.Windows.OfType<BLFServerConfig>().Any() == false)
                {
                    new BLFServerConfig(ServerURL, ServerPort).Show();
                }
            }));
        }

        public void OnHeartbeatReceived()
        {
            LastHeartBeatStamp = Environment.TickCount;
        }

        public void Shutdown()
        {
            IsAppRunning = false;
//            if(ConnectionStatusMonitor != null)
//                ConnectionStatusMonitor.Abort();
//            if (PacketParserThread != null)
//                PacketParserThread.Abort();
            Disconnect();
        }

        /// <summary>
        /// Connection manager will try to periodically connect to this specific BLF server
        /// </summary>
        /// <param name="Url"></param>
        /// <param name="port"></param>
        public void SetConnectionDetails(string Url, int port)
        {
            //force creating a new connection if connection details have changed
            if (ServerURL != Url || ServerPort != port)
                Disconnect();
            ServerURL = Url;
            ServerPort = port;
        }

        private void UpdateUIConnectionStatus()
        {
            //if we already reported this status, nothing to do here
            if (IsConnectedStatus == false && nclient == null)
                return;
            if (IsConnectedStatus == true && nclient != null)
                return;

            //set new status
            if (nclient != null && nclient.IsConnected())
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
            while(IsAppRunning)
            {
                //if we did not receive a heartbeat for too much time, we should try to reconnect. Or maybe say that server is down ? 
                if (LastHeartBeatStamp + (long)ServerPacketStatus.HeartBeatTimeOutMS < Environment.TickCount)
                    Disconnect();

                //update UI status if we managed to connect to the server
                if (Globals.IsAppRunning == true)
                    UpdateUIConnectionStatus();

                //can we change UI status ? This might take a while, that is why we have the double UI update in 1 loop
                if (nclient == null || nclient.IsConnected() == false && IsAppRunning == true)
                    CreateNewConnection();

                //update UI status if we managed to connect to the server
                if(IsAppRunning == true)
                    UpdateUIConnectionStatus();

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
            while (IsAppRunning)
            {
                if (nclient != null)
                {
                    byte[] reply = nclient.ReadPacket();
                    if (reply != null)
                        NetworkClientInterpretPacket.InterpretMessage(reply);
                }
                else
                    Thread.Sleep(1000);
            }
            PacketParserThread = null;
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Async packet parser has exited");
        }

        /// <summary>
        /// Disconnect existing connections to the server
        /// </summary>
        public void Disconnect()
        {
            //stop reading and writing
            if (nclient != null)
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Disconnected from server");
                nclient.Disconnect();
                nclient = null;
            }
            //notify user that we no longer get updates from the server
            ShowNoConnectionWindow();
        }

        /// <summary>
        /// When connection settings change, this function should be called to create a new connection
        /// </summary>
        public void CreateNewConnection()
        {
            //make sure we ditch old data
            Disconnect();

            if (IsAppRunning == false)
                return;

            //need connection details in order to create a new connection
            if (ServerURL == null || ServerURL.Length == 0)
            {
                ShowServerSettingsWindow();
                return;
            }

            //create a client connection that should persist while the UI is alive
            NetworkClient TClient = new NetworkClient();
            TClient.ConnectToServer(ServerURL, ServerPort);
            if (TClient != null && TClient.IsConnected() == false)
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Failed to created new server connection to " + ServerURL.ToString() + ":" + ServerPort.ToString());
                TClient = null;
                return; //failed to create a connection
            }

            //no more need to show user that there is no connection
            HideNoConnectionWindow();

            //if it is good to use, we use it
            nclient = TClient;
            LastHeartBeatStamp = Environment.TickCount;

            if (PacketParserThread == null)
            {
                PacketParserThread = new Thread(new ThreadStart(AsyncParsePackets));
                PacketParserThread.Start();
            }
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Created new server connection to " + ServerURL.ToString() + ":" + ServerPort.ToString());
        }

        /// <summary>
        /// Interface function. Made in case multiple server connections will be possible. Not the case for now
        /// </summary>
        /// <param name="pkt"></param>
        /// <returns></returns>
        public bool SendPacket(object pkt)
        {
            if(nclient != null)
                nclient.SendPacket(pkt);
            return true;
        }

        /// <summary>
        /// Fetch a packet
        /// </summary>
        /// <returns></returns>
        public byte [] ReadPacket()
        {
            return nclient.ReadPacket();
        }

        public bool IsConnected()
        {
            if (nclient != null && nclient.IsConnected() == true)
                return true;
            return false;
        }

        public ushort GetConnectionChecksum()
        {
            if (nclient != null && nclient.IsConnected() == true)
                return nclient.GetConnectionChecksum();
            return 0;
        }
    }
}
