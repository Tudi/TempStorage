using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Timers;

namespace BLFClient.Backend
{
    public enum ServerPacketStatus
    {
        PacketNotSent = 0,
        PacketSent = 1,
        PacketReceived = 2,
        PacketTimeOutMS = 5000,         // wait this amount of time when we send out a packet. If no reply came, try to resend it
        MonitorTimeOutMS = 2*30*1000,  // wait this amount of time when we send out a packet. If no reply came, try to resend it
        CallFWDTimeOutMS = 5000,
        HeartBeatTimeOutMS = 2 * 30 * 1000,
    }

    /// <summary>
    /// Store the forward status of a specific extension
    /// </summary>
    public class ForwardStatusStore
    {
        public string Extension;
        public CallForwardingTypes ForwardType;
        public long VoiceMailForward;
        public long DestinationForward;
        public long LastQueryStamp; // in case we need to update the status from time to time
        public ServerPacketStatus PacketStatus;

        public ForwardStatusStore(string Ext)
        {
            Extension = Ext;
            OnConnectionLost();
        }

        /// <summary>
        /// Check if manager should query the forward status of this extension ( phone number )
        /// </summary>
        /// <returns></returns>
        public bool ShouldQueryForwardStatus()
        {
            //if this extension is not available, there is no reason to query it. By default we presume extension forwarding is OFF
            if (Globals.ExtensionManager.ExtensionExists(Extension) == false)
                return false;

            //if we know the status, and there is a monitor set, we no longer need to query this only once for the initial state
            if (PacketStatus == ServerPacketStatus.PacketReceived && Globals.ExtensionManager.HasMonitor(Extension) == true)
                return false;

            //recently sent a packet to the server. No need to flood it
            if (LastQueryStamp + (long)ServerPacketStatus.CallFWDTimeOutMS > Environment.TickCount)
                return false;

            return true;
        }

        public void OnConnectionLost()
        {
            LastQueryStamp = 0; // we never queried this value
            ForwardType = CallForwardingTypes.CallForwardNone;
            VoiceMailForward = 0;
            DestinationForward = 0;
            PacketStatus = ServerPacketStatus.PacketNotSent;
        }
    }

    /// <summary>
    /// Forward manager will track all existing extensions and their forward statuses
    /// All forward queries should go through this manager
    /// As soon as the manager receives the new status of an extension, it will notify all extensions with same number with the status
    /// </summary>
    public class CallForwardManager
    {
        HashSet<ForwardStatusStore> Forwards;
        System.Timers.Timer UpdateTimer = null;

        public CallForwardManager()
        {
            Forwards = new HashSet<ForwardStatusStore>();
        }

        ~CallForwardManager()
        {
            ShutDown();
        }

        public void ShutDown()
        {
            //stop the times
            if (UpdateTimer != null)
            {
                UpdateTimer.Stop();
                UpdateTimer = null;
            }
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Async extension forward status manager has exited");
        }

        //handled server side
        public void Save()
        {
        }

        //should wait for full load and connection to the server
        public void Load()
        {
            Task mytask = Task.Run(() => { _Load(); });
        }

        /// <summary>
        /// Delay the loading of this manager until we have a server connection that we can query
        /// </summary>
        private void _Load()
        {
            // wait for index cards and extension to load up. Also wait for a connection we can query
            while (Globals.ConnectionManager == null || Globals.ConnectionManager.IsConnected() == false || Globals.IndexCardsLoaded == false)
                Thread.Sleep(100);

            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Async extension forward status manager has started");
            //create a timer to periodically query extension forwarding status and issue callbacks to all cell cards on status change
            UpdateTimer = new System.Timers.Timer(100);
            UpdateTimer.Enabled = true; 
            UpdateTimer.Elapsed += new ElapsedEventHandler(PeriodicStatusUpdate);
        }

        /// <summary>
        /// Periodically check for new extensions and see if we need to query their forwarding status
        /// </summary>
        /// <param name="source"></param>
        /// <param name="arg"></param>
        private void PeriodicStatusUpdate(object source, ElapsedEventArgs arg)
        {
            //nothing to be done for now
            if (Globals.ConnectionManager == null || Globals.ConnectionManager.IsConnected() == false)
                return;

            UpdateTimer.Stop(); // in case network buffer gets full, the function might block and threadpool might call us multiple times
            //get the list of all the extensions, query each for their forwarding status
            HashSet<string> extl = Globals.ExtensionManager.GetExtensionsQueryForward();
            foreach (string ext in extl)
            {
                //avoid burst flooding the server
                while (Globals.AntiFloodManager.CanSendNewPacket() == false)
                    Thread.Sleep(9); // should not have exact same value as other threads waiting on anti flood manager
                //safe to send a new packet
                PhoneNumberQueryForwarding(ext);
            }
            UpdateTimer.Start();
        }

        /// <summary>
        /// Get the store for a specific extension ( phone number )
        /// </summary>
        /// <param name="Extension"></param>
        /// <returns></returns>
        private ForwardStatusStore GetStatusStore(string Extension)
        {
            foreach (ForwardStatusStore it in Forwards)
                if (it.Extension == Extension)
                    return it;

            //if we got here, than we need to create a new store
            ForwardStatusStore fw = new ForwardStatusStore(Extension);
            Forwards.Add(fw);
            
            return fw;
        }

        /// <summary>
        /// When a new UI extension is created, this function gets called. If we already know the forward status than we will initiate a callback to update forward status for this extension
        /// </summary>
        /// <param name="NewNumber"></param>
        public void OnExtensionCreate(PhoneNumber NewNumber)
        {
            //if we are still in the loading process, delay this query for now. We will auto query the state as soon as we are done loading
            if (Globals.ConnectionManager.IsConnected() == false || Globals.IndexCardsLoaded == false)
                return;

            // skip dummy placeholder extensions
            if (NewNumber.GetExtension().Length == 0)
                return;

            //if we already have a status queried for this extension than we can return the status directly. Else we will isue an async callback when server replies
            if (NewNumber.IsSubscriberRange() == false)
            {
                ForwardStatusStore fw = GetStatusStore(NewNumber.GetExtension());
                if (fw.ShouldQueryForwardStatus() == false)
                    NewNumber.OnForwardingChange(fw);
            }
        }

        /// <summary>
        /// Internal : check if we should make the query, issue a query to the server
        /// </summary>
        /// <param name="Extension"></param>
        private void PhoneNumberQueryForwarding(string Extension)
        {
            ForwardStatusStore fw = GetStatusStore(Extension);
            if (fw.ShouldQueryForwardStatus() == true)
            {
                fw.LastQueryStamp = Environment.TickCount; // if there is no reply comming for this extension, we should retry a bit later
                fw.PacketStatus = ServerPacketStatus.PacketSent;
                NetworkClientBuildPacket.QueryDeviceForwarding(Extension.ToString());
            }
        }

        /// <summary>
        /// set call forwarding for an extension( phone number ). This is a non blocking call. Result is not guaranteed. Callback function will set the state of extansions
        /// </summary>
        /// <param name="Extension"></param>
        /// <param name="Type"></param>
        /// <param name="VoiceMail"></param>
        /// <param name="Destination"></param>
        public void CallForwardingSet(string Extension, CallForwardingTypes Type, long VoiceMail, long Destination)
        {
            ForwardStatusStore fw = GetStatusStore(Extension);
            short Enable = 0;
            if (Type != CallForwardingTypes.CallForwardNone)
                Enable = 1;
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagCallForwarding, "Client : Set forwarding for extension " + Extension + " to Type " + Type + " to destination " + Destination);
            NetworkClientBuildPacket.SetFeatureForwarding(Extension.ToString(), Destination.ToString(), 0, Enable);
        }

        public void PhoneNumberUpdateForwarding(string Extension, CallForwardingTypes Type, long VoiceMail, long Destination)
        {
            //find the store for this extension
            ForwardStatusStore fw = GetStatusStore(Extension);

            //update it
            fw.ForwardType = Type;
            fw.VoiceMailForward = VoiceMail;
            fw.DestinationForward = Destination;
            fw.PacketStatus = ServerPacketStatus.PacketReceived;

            Globals.Logger.LogString(LogManager.LogLevels.LogFlagCallForwarding, "Server : Set forwarding for extension " + Extension + " to Type " + Type + " to destination " + Destination);
            Globals.ExtensionManager.OnForwardingStatusUpdate(fw);
        }
        
        public ForwardStatusStore ForwardStatusGet(string Extension)
        {
            foreach (ForwardStatusStore it in Forwards)
                if (it.Extension == Extension)
                    return it;
            return null;
        }

        public void OnConnectionChanged(bool Connected)
        {
            foreach (ForwardStatusStore it in Forwards)
                it.OnConnectionLost();
        }
    }
}
