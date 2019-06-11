using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Threading;
using System.Timers;
using System.Threading;
using System.Collections.Concurrent;

namespace BLFClient.Backend
{
    public class MonitorStatusStore
    {
        public string Extension;
        public long XrefId;
        public long LastQueryStamp; // in case we need to update the status from time to time
        public ServerPacketStatus PacketStatus;
        public long LastStatusUpdateStamp;  // monitor will periodically send us a status update. If server deadlocks, we will not receive status updates
        public PhoneStatusCodes LastStatus; // on last device query, what did the server say, does this extension exist ?
        public ServerPacketStatus MonitorPacketStatus;
        public long LastMonitorQueryStamp;
        public long CallId; //only valid while having a call
        public long CallPartner;
        public long CallIdConsultation;
        public long CallPartnerConsultation;
        public string ServerIPAndPort; // 127.0.0.1:5050 More than 1 server can have the exact same extension
//        public string ExtensionFull; //Server:Prefix:Extension
        public string Prefix; //this is probably already included in extension ( or not )

        public MonitorStatusStore(string ServerIPAndPort, string Prefix, string Ext)
        {
            Extension = Ext;
//            ExtensionFull = ExtFull;
            OnConnectionLost();
        }

        public bool ShouldQueryStatus()
        {
            //does it have a UI component ? Do not query status of the persport.txt extensions without a reason
            PhoneNumber pn = Globals.ExtensionManager.PhoneNumberGetFirst(ServerIPAndPort, Prefix, Extension, true);
            if (pn == null)
                return false;

            //no query has been issued yet
            if (LastQueryStamp == 0)
            {
                //Globals.Logger.LogString(LogManager.LogLevels.LogLevelDebug, "Query snapshot, since it's first time : " + Extension);
                return true;
            }

            //we issued a query, no need to spam a second one
            if (LastQueryStamp + (long)ServerPacketStatus.PacketTimeOutMS > Environment.TickCount)
                return false;

            //monitor will send updates even without query. In case no update came, try to query status without monitor
            if (HasMonitorSet() == true && LastStatusUpdateStamp + (long)ServerPacketStatus.MonitorTimeOutMS > Environment.TickCount)
                return false;

            //too much time passed since last update for this extension. Should try to set a new monitor or at least a query
            return true;
        }

        public void OnConnectionLost()
        {
            LastQueryStamp = 0; // we never queried this value
            XrefId = 0;
            LastStatusUpdateStamp = 0;
            PacketStatus = ServerPacketStatus.PacketNotSent;
            LastStatus = PhoneStatusCodes.PHONE_DOESNOT;
            MonitorPacketStatus = ServerPacketStatus.PacketNotSent;
            LastMonitorQueryStamp = 0;
            CallId = 0;
            CallPartner = 0;
            CallIdConsultation = 0;
            CallPartnerConsultation = 0;
        }

        public bool HasMonitorSet()
        {
            return (XrefId != 0 && MonitorPacketStatus == ServerPacketStatus.PacketReceived);
        }

        public bool ShouldTryAttachMonitor()
        {
            if (HasMonitorSet())
                return false;

            if (MonitorPacketStatus == ServerPacketStatus.PacketSent && LastMonitorQueryStamp + (long)ServerPacketStatus.MonitorTimeOutMS > Environment.TickCount)
                return false;

            //does it have a UI component ?
            PhoneNumber pn = Globals.ExtensionManager.PhoneNumberGetFirst(ServerIPAndPort, Prefix, Extension, true);
            if (pn == null)
                return false;

            return true;
        }

        public bool ExtensionExists()
        {
            return (LastStatus != PhoneStatusCodes.PHONE_DOESNOT);
        }

        public bool CheckSearchMatch(string pServerAndIP, string pPrefix, string pExtension)
        {
            if (ServerIPAndPort != null && pServerAndIP != null && pServerAndIP != ServerIPAndPort)
                return false;
            if (Prefix != null && pPrefix != null && Prefix != pPrefix)
                return false;
            return Extension == pExtension;
        }
    }

    class PoolQueue<T>
    {
        T[] nodes;
        int current;
        int emptySpot;

        public PoolQueue(int size)
        {
            nodes = new T[size];
            this.current = 0;
            this.emptySpot = 0;
        }

        public void Enqueue(T value)
        {
            nodes[emptySpot % nodes.Length] = value;
            emptySpot++;
        }

        public T Dequeue()
        {
            int ret = current;
            current++;
            return nodes[ret % nodes.Length];
        }

        public bool Empty()
        {
            return emptySpot == current;
        }

        public bool Full()
        {
            return (emptySpot - current == nodes.Length - 1);
        }
    }

    // window resize was slow. So i added this pool in hope to speed it up. But no, the issue seems to be somewhere else
    class PhoneNumberFactory
    {
        int PoolSize = 500;
        PoolQueue<PhoneNumber> PreparedObjects;
        System.Timers.Timer UpdateTimer;

        public PhoneNumberFactory()
        {
            //create a pool
            PreparedObjects = new PoolQueue<PhoneNumber>(PoolSize);

            //fill pool
            PeriodicStatusUpdate(null, null);

            //periodically try to refill pool
            UpdateTimer = new System.Timers.Timer(10);
            UpdateTimer.Enabled = true;
            UpdateTimer.Elapsed += new ElapsedEventHandler(PeriodicStatusUpdate);
        }

        private void PeriodicStatusUpdate(object source, ElapsedEventArgs arg)
        {
            if (App.Current == null)
            {
                UpdateTimer.Dispose();
                return;
            }
            //this will only help us when we create a new index cards
            if (Globals.ConfigLoaded == false || Globals.IndexCardsLoaded == false)
                return;
            if (Globals.Config.GetIniName() == null)
                return;
            UpdateTimer.Stop();
            App.Current.Dispatcher.Invoke(DispatcherPriority.Normal, (Action)(() =>
            {
                int MaxAllocations = 2; // if we block the UI thread for too long, it will become unresponsive
                while (PreparedObjects.Full() == false && MaxAllocations > 0)
                {
                    if (App.Current == null)
                        break;
                    PhoneNumber pn = new PhoneNumber();
                    PreparedObjects.Enqueue(pn);
                    MaxAllocations--;
                }
            }));
            Thread.Sleep(10); //do not deadlock main update thread with our allocations
            UpdateTimer.Start();
        }

        public PhoneNumber Get()
        {
            if (PreparedObjects.Empty())
                return new PhoneNumber();
            return PreparedObjects.Dequeue();
        }
    }

    public class PhoneNumberManager
    {
        static long PhoneNumberGUID = 1;
        ConcurrentBag<PhoneNumber> PhoneNumbers;
        ConcurrentBag<MonitorStatusStore> ExtensionMonitors;
        System.Timers.Timer UpdateTimer = null;
        PhoneNumber LastClickedExtension = null;
        PhoneNumberFactory ExtensionFactory;

        public PhoneNumberManager()
        {
            PhoneNumbers = new ConcurrentBag<PhoneNumber>();
            ExtensionMonitors = new ConcurrentBag<MonitorStatusStore>();
            ExtensionFactory = new PhoneNumberFactory();
        }

        ~PhoneNumberManager()
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
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Async extension status manager has exited");
        }

        //should wait for full load and connection to the server
        public void Load()
        {
            Task mytask = Task.Run(() => { _Load(); });
        }

        private void _Load()
        {
            // wait for index cards and extension to load up. Also wait for a connection we can query
            while ((Globals.ConnectionManager == null || Globals.ConnectionManager.HasAnyActiveConnection() == false || Globals.IndexCardsLoaded == false) && Globals.IsAppRunning == true)
                Thread.Sleep(100);

            if (Globals.IsAppRunning == false)
                return;

            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Async extension status manager has started");
            //create a timer to periodically query extension forwarding status and issue callbacks to all cell cards on status change
            UpdateTimer = new System.Timers.Timer(100);
            UpdateTimer.Enabled = true;
            UpdateTimer.Elapsed += new ElapsedEventHandler(PeriodicStatusUpdate);
        }

        private void PeriodicStatusUpdate(object source, ElapsedEventArgs arg)
        {
            //nothing to be done for now
            if (Globals.ConnectionManager == null || Globals.ConnectionManager.HasAnyActiveConnection() == false)
                return;

            UpdateTimer.Stop();// in case network buffer gets full, the function might block and threadpool might call us multiple times
                               //get the list of all the extensions, query each for their forwarding status
                               /*           HashSet<string> extl = GetExtensions();
                                          foreach (string ext in extl)
                                          {
                                              //avoid burst flooding the server
                                              while (Globals.AntiFloodManager.CanSendNewPacket() == false)
                                                  Thread.Sleep(11); // should not have exact same value as other threads waiting on anti flood manager
                                              //safe to send a new packet
                                              PhoneNumberQueryDeviceStatus(ext);
                                          } */
            foreach (MonitorStatusStore ss in ExtensionMonitors)
            {
                if (ss.ShouldQueryStatus() == true)
                {
                    if (ss.ServerIPAndPort != null && Globals.ConnectionManager != null)
                    {
                        NetworkClient nc = Globals.ConnectionManager.GetCLient(ss.ServerIPAndPort);
                        if (nc != null)
                        {
                            ss.LastQueryStamp = Environment.TickCount; // if there is no reply comming for this extension, we should retry a bit later
                            ss.PacketStatus = ServerPacketStatus.PacketSent;
                            nc.PacketBuilder.SnapshotDevice(ss.Extension);
                        }
                    }
                }
            }
            UpdateTimer.Start();
        }

        private void PhoneNumberQueryDeviceStatus(string ServerIPAndPort, string Prefix, string Extension)
        {
            //ignore dummy extensions
            if (Extension.Length == 0)
                return;

            MonitorStatusStore ss = GetStatusStore(ServerIPAndPort, Prefix, Extension);
            if (ss.ShouldQueryStatus() == true)
            {
                ss.LastQueryStamp = Environment.TickCount; // if there is no reply comming for this extension, we should retry a bit later
                ss.PacketStatus = ServerPacketStatus.PacketSent;
                if (ss.ServerIPAndPort != null && Globals.ConnectionManager != null)
                {
                    NetworkClient nc = Globals.ConnectionManager.GetCLient(ss.ServerIPAndPort);
                    if (nc != null)
                        nc.PacketBuilder.SnapshotDevice(Extension.ToString());
                }
            }
        }

        private MonitorStatusStore GetStatusStore(string ServerIPAndPort, string Prefix, string Extension)
        {
//            string ExtensionFull = GetMultiServerExtension(ServerIPAndPort, Prefix, Extension);
            foreach (MonitorStatusStore it in ExtensionMonitors)
                if (it.CheckSearchMatch(ServerIPAndPort,Prefix,Extension) == true)
                    return it;

            //if we got here, than we need to create a new store
            MonitorStatusStore fw = new MonitorStatusStore(ServerIPAndPort, Prefix, Extension);
            ExtensionMonitors.Add(fw);

            return fw;
        }

/*        private MonitorStatusStore GetStatusStoreMultiServer(string ServerIPAndPort, string Prefix, string Extension)
        {
            foreach (MonitorStatusStore it in ExtensionMonitors)
                if (it.CheckSearchMatch(ServerIPAndPort, Prefix, Extension) == true)
                    return it;

            //if we got here, than we need to create a new store
            MonitorStatusStore fw = new MonitorStatusStore(null, ExtensionFull);
            ExtensionMonitors.Add(fw);

            return fw;
        }*/

        private MonitorStatusStore GetStatusStoreXRef(string ServerIPAndPort, long Xref)
        {
            foreach (MonitorStatusStore it in ExtensionMonitors)
                if (it.XrefId == Xref && it.ServerIPAndPort == ServerIPAndPort)
                    return it;

            return null;
        }

        public void PhoneNumberAdd(PhoneNumber pn)
        {
            pn.SetGUID(PhoneNumberGUID++);
            PhoneNumbers.Add(pn);
        }

        public void PhoneNumberDelete(PhoneNumber pnDeleted)
        {
            foreach (PhoneNumber pn in PhoneNumbers)
                if (pn.GetGUID() == pnDeleted.GetGUID())
                {
                    //            PhoneNumbers.Remove(pn);
                }
        }

        public PhoneNumber PhoneNumberGet(long GUID)
        {
            foreach (PhoneNumber pn in PhoneNumbers)
                if (pn.GetGUID() == GUID)
                    return pn;
            return null;
        }

        public PhoneNumber PhoneNumberGetFirst(string ServerAndIP, string Prefix, string Extension, bool IncludeRanges = false)
        {
            string ExtensionRange = Extension.Substring(0, Extension.Length - 1);
            foreach (PhoneNumber pn in PhoneNumbers)
            {
                //we are looking for server specific UI component
                if (ServerAndIP != null && pn.GetServerIPAndPort() != null && ServerAndIP != pn.GetServerIPAndPort())
                    continue;
                //if we know the prefix of this extension, we might want to check for a match
                if (Prefix != null && pn.GetPrefix() != null && Prefix != pn.GetPrefix())
                    continue;
                if (pn.GetExtension() == Extension || (IncludeRanges == true && pn.IsSubscriberRange() == true && pn.GetExtension() == ExtensionRange))
                    return pn;
            }
            return null;
        }

        public PhoneNumber PhoneNumberGet(int x, int y)
        {
            //there might be more than 1 phone numbers with the same coordinate, we want to get the active index card coordinate
            MainWindow MainObject = (MainWindow)App.Current.MainWindow;
            if (MainObject == null)
                return null;
            IndexCard ic = MainObject.GetVisibleIndexCard();
            if (ic == null)
                return null;
            return ic.PhoneNumberGet(x, y);
        }

        public PhoneNumber PhoneNumberGetByXRef(string ServerIPAndPort, long XRef)
        {            
            MonitorStatusStore ss = GetStatusStoreXRef(ServerIPAndPort, XRef);
            if (ss != null)
                return PhoneNumberGetFirst(ss.ServerIPAndPort, ss.Prefix, ss.Extension, true);
            return null;
        }

        public void OnConnectionChanged(string ServreIPAndPort, bool Connected)
        {
            //                List<PhoneNumber> PhoneNumbers2 = new List<PhoneNumber>(PhoneNumbers); // this might bug out due to concurency. For some reason lock deadlocked. Maybe use a different lock
            foreach (PhoneNumber pn in PhoneNumbers)
                if(pn.GetServerIPAndPort() == ServreIPAndPort)
                {
                    if (App.Current != null)
                        App.Current.Dispatcher.Invoke(DispatcherPriority.Normal, (Action)(() =>
                        {
                            if (App.Current == null)
                                return;

                            MainWindow MainObject = (MainWindow)App.Current.MainWindow;

                            if (MainObject == null)
                                return;
                            pn.OnConnectionChanged(Connected);
                        }));
                }
            //set all monitors to false(even if this is a connect). Maybe server restarted and lost all monitors ?
            foreach (MonitorStatusStore it in ExtensionMonitors)
                if(it.ServerIPAndPort == ServreIPAndPort)
                    it.OnConnectionLost();
        }

        public void OnServerExtensionNameReceive(string ServerIPAndPort, string Prefix, string Extension, string Name )
        {
            OnClientReceivedPacketWithExtension_(ServerIPAndPort, Prefix, Extension);

            string ExtensionRange = Extension.Substring(0, Extension.Length - 1);
            foreach (PhoneNumber pn in PhoneNumbers)
                if (pn.GetExtension() == Extension || (pn.IsSubscriberRange() == true && pn.GetExtension() == ExtensionRange))
                    if (App.Current != null)
                        App.Current.Dispatcher.Invoke(DispatcherPriority.Normal, (Action)(() =>
                        {
                            if (App.Current == null)
                                return;

                            MainWindow MainObject = (MainWindow)App.Current.MainWindow;

                            if (MainObject == null)
                                return;

                            pn.SetPrefix(Prefix);
                            pn.SetName(Name);
                            pn.SetServerIPAndPort(ServerIPAndPort);
                        }));
        }

        public static int Int32Parse(string num,int def)
        {
            try
            {
                return Int32.Parse(num);
            }
            catch
            {
                return def;
            };
        }

        public void OnMonitorStart(string ServerIPAndPort, string Prefix, string Extension, string xRef)
        {
            MonitorStatusStore ss = GetStatusStore(ServerIPAndPort, Prefix, Extension);
            ss.XrefId = Int32Parse(xRef, 0);
            ss.MonitorPacketStatus = ServerPacketStatus.PacketReceived;
        }

        public void OnMonitorStop(string ServerIPAndPort, string Prefix, string pxRef)
        {
            int xRef = Int32Parse(pxRef, -1);
            MonitorStatusStore ss = GetStatusStoreXRef(ServerIPAndPort, xRef);
            if (ss != null)
            {
                ss.XrefId = 0;
                ss.MonitorPacketStatus = ServerPacketStatus.PacketReceived; // it's bad, but the best we can do
                return;
            }
        }

        public void OnSystemStatusOk(string ServerIPAndPort)
        {
            //mark all extensions with valid monitors as they were updated recently
            foreach (MonitorStatusStore it in ExtensionMonitors)
                if (it.XrefId != 0)
                    it.LastStatusUpdateStamp = Environment.TickCount;
        }

        public void OnStatusChange(string ServerIPAndPort, string Prefix, string Extension, PhoneStatusCodes NewStatus)
        {
            //mark that we are alive
            MonitorStatusStore ss = GetStatusStore(ServerIPAndPort, Prefix, Extension);
            ss.LastStatusUpdateStamp = Environment.TickCount;
            ss.PacketStatus = ServerPacketStatus.PacketReceived;
            ss.LastStatus = NewStatus;

            //try to attach a monitor to it
            if( ss.LastStatus != PhoneStatusCodes.PHONE_DOESNOT && ss.ShouldTryAttachMonitor() == true )
            {
                ss.MonitorPacketStatus = ServerPacketStatus.PacketSent;
                ss.LastMonitorQueryStamp = Environment.TickCount;
                if (Globals.ConnectionManager != null)
                {
                    NetworkClient nc = Globals.ConnectionManager.GetCLient(ss.ServerIPAndPort);
                    nc.PacketBuilder.MonitorStart(Extension);
                }
            }

            string ExtensionRange = Extension.Substring(0, Extension.Length - 1);
            foreach (PhoneNumber pn in PhoneNumbers)
                if (pn.GetExtension() == Extension || (pn.IsSubscriberRange() == true && pn.GetExtension() == ExtensionRange))
                {
                    if(App.Current!=null)
                    App.Current.Dispatcher.Invoke(DispatcherPriority.Normal, (Action)(() =>
                    {
                        if (App.Current == null)
                            return;

                        MainWindow MainObject = (MainWindow)App.Current.MainWindow;

                        if (MainObject == null)
                            return;

                        pn.SetStatus(NewStatus, Extension); // extension is provided for ranges
                    }));
                }
    }

        public HashSet<string> GetExtensions()
        {
            HashSet<string> ret = new HashSet<string>();
            foreach (PhoneNumber pn in PhoneNumbers)
                if (pn.GetExtension().Length != 0 && pn.GetGUID() != 0)
                {
                    if (pn.IsSubscriberRange() == false)
                    {
                        ret.Add(pn.GetExtension());
                    }
                    else
                    {
                        for (int i = 0; i < 10; i++)
                            ret.Add(pn.GetExtension() + i.ToString());
                    }
                }
            return ret;
        }

        public HashSet<string> GetExtensionsQueryForward()
        {
            HashSet<string> ret = new HashSet<string>();
            foreach (PhoneNumber pn in PhoneNumbers)
                if (pn.GetExtension().Length > 0 && pn.GetGUID() != 0)
                {
                    MonitorStatusStore ss = GetStatusStore(pn.GetServerIPAndPort(), pn.GetPrefix(), pn.GetExtension());
                    if ( ss.LastStatus != PhoneStatusCodes.PHONE_DOESNOT && ss.ShouldQueryStatus() == false && ss.PacketStatus == ServerPacketStatus.PacketReceived)
                        ret.Add(pn.GetExtension());
                }
            return ret;
        }

        public void OnForwardingStatusUpdate(ForwardStatusStore fs)
        {
            foreach (PhoneNumber pn in PhoneNumbers)
                if (pn.GetExtension() == fs.Extension)
                {
                    App.Current.Dispatcher.Invoke(DispatcherPriority.Normal, (Action)(() =>
                    {
                        if (App.Current == null)
                            return;

                        MainWindow MainObject = (MainWindow)App.Current.MainWindow;

                        if (MainObject == null)
                            return;

                        pn.OnForwardingChange(fs);
                    }));
                }
        }

        public HashSet<string> GetUniqueEmails()
        {
            HashSet<string> ret = new HashSet<string>();
            foreach (PhoneNumber pn in PhoneNumbers)
                if (pn.GetEmail() != null && pn.GetEmail().Length > 0 && pn.GetGUID() != 0)
                    ret.Add(pn.GetEmail());
            return ret;
        }

        public void OnAbsenceStatusUpdate(string Email, bool Available)
        {
            foreach (PhoneNumber pn in PhoneNumbers)
                if (pn.GetEmail() == Email)
                {
                    App.Current.Dispatcher.Invoke(DispatcherPriority.Normal, (Action)(() =>
                    {
                        if (App.Current == null)
                            return;

                        MainWindow MainObject = (MainWindow)App.Current.MainWindow;

                        if (MainObject == null)
                            return;

                        pn.OnAbsenceStatusUpdate(Available);
                    }));
                }
        }

        public bool HasMonitor(string ServerIPAndPort, string Prefix, string Extension)
        {
            MonitorStatusStore ss = GetStatusStore(ServerIPAndPort, Prefix, Extension);
            return ss.HasMonitorSet();
        }

        /// <summary>
        /// An extension can exist and be offline. We can set a monitor on an offline extension. We can NOT set a monitor for an unexisting extension
        /// </summary>
        /// <param name="Extension"></param>
        /// <returns></returns>
        public bool ExtensionExists(string ServerIPAndPort, string Prefix, string Extension)
        {
            MonitorStatusStore ss = GetStatusStore(ServerIPAndPort, Prefix,Extension);
            return ss.ExtensionExists();
        }

        public PhoneStatusCodes GetCachedStatus(string ServerIPAndPort, string Prefix, string Extension)
        {
            MonitorStatusStore ss = GetStatusStore(ServerIPAndPort, Prefix, Extension);
            return ss.LastStatus;
        }

        public void OnPhoneNumberClick(PhoneNumber pn)
        {
            LastClickedExtension = pn;
        }

        public string GetLastSelectedExtension()
        {
            if (LastClickedExtension == null)
                return "";
            foreach (PhoneNumber pn in PhoneNumbers)
                if (LastClickedExtension == pn)
                    return LastClickedExtension.GetExtension();
            LastClickedExtension = null;
            return "";
        }

        public void OnCallIdReceived(string ServerIPAndPort, string Prefix, string CallerDevice, string CallId, string CalledDevice)
        {
            if (CallerDevice[0] == 'N')
                CallerDevice = CallerDevice.Substring(1);
            if (CalledDevice[0] == 'N')
                CalledDevice = CalledDevice.Substring(1);
            if (CallerDevice[0] == '<' && CallerDevice.IndexOf('>') > 0)
                CallerDevice = CallerDevice.Substring(1, CallerDevice.IndexOf('>') - 1);
            if (CalledDevice[0] == '<' && CalledDevice.IndexOf('>') > 0)
                CalledDevice = CalledDevice.Substring(1, CalledDevice.IndexOf('>') - 1);

            //           int CallerDeviceInt = Int32Parse(CallerDevice, 0);
            int CallIdInt = Int32Parse(CallId, 0);
//            int CalledDeviceInt = Int32Parse(CalledDevice, 0);

            MonitorStatusStore ss = GetStatusStore(ServerIPAndPort, Prefix, CallerDevice);
            ss.CallId = CallIdInt;
            ss.CallPartner = Int32Parse(CalledDevice, 0);

            ss = GetStatusStore(ServerIPAndPort, Prefix, CalledDevice);
            ss.CallId = CallIdInt;
            ss.CallPartner = Int32Parse(CallerDevice, 0);
        }

        public void OnCallIdClear(string ServerIPAndPort, string Prefix, string CallerDevice, string CallId, string CalledDevice)
        {
            OnCallIdReceived(ServerIPAndPort, Prefix, CallerDevice, "0", CalledDevice);
        }

        public long GetCallId(string ServerIPAndPort, string Prefix, string Extension)
        {
            MonitorStatusStore ss = GetStatusStore(ServerIPAndPort, Prefix, Extension);
            return ss.CallId;
        }

        public long GetOptisetCallId()
        {
            string OptisetExtenstion = PhoneNumber.GetExtensionFromFullNumber(Globals.Config.GetConfig("Options", "OptisetExtension", ""));
            return GetCallId(null, null, OptisetExtenstion);
        }

        public long GetOptisetCallTarget()
        {
            string OptisetExtenstion = PhoneNumber.GetExtensionFromFullNumber(Globals.Config.GetConfig("Options", "OptisetExtension", ""));
            foreach (MonitorStatusStore it in ExtensionMonitors)
                if (it.Extension == OptisetExtenstion)
                    return it.CallId;
            return 0;
        }

        public void OnConsultaionCallIdReceived(string ServerIPAndPort, string Prefix, string CallerDevice, string CallId, string CalledDevice)
        {
            if (CallerDevice[0] == 'N')
                CallerDevice = CallerDevice.Substring(1);
            if (CalledDevice[0] == 'N')
                CalledDevice = CalledDevice.Substring(1);

//            int CallerDeviceInt = Int32Parse(CallerDevice, 0);
            int CallIdInt = Int32Parse(CallId, 0);
//            int CalledDeviceInt = Int32Parse(CalledDevice, 0);

            MonitorStatusStore ss = GetStatusStore(ServerIPAndPort, Prefix, CallerDevice);
            ss.CallIdConsultation = CallIdInt;
            ss.CallPartnerConsultation = Int32Parse(CalledDevice, 0);

            ss = GetStatusStore(ServerIPAndPort, Prefix, CalledDevice);
            ss.CallIdConsultation = CallIdInt;
            ss.CallPartnerConsultation = Int32Parse(CallerDevice, 0);
        }

        public void OnConsultaionCallIdClear(string ServerIPAndPort, string Prefix, string CallerDevice, string CallId, string CalledDevice)
        {
            OnConsultaionCallIdReceived(ServerIPAndPort, Prefix, CallerDevice, "0", CalledDevice);
        }

        public long GetConsultaionCallId(string ServerIPAndPort, string Prefix, string Extension)
        {
            MonitorStatusStore ss = GetStatusStore(ServerIPAndPort, Prefix, Extension);
            return ss.CallIdConsultation;
        }

        public long GetOptisetConsultaionCallId()
        {
            string OptisetExtenstion = PhoneNumber.GetExtensionFromFullNumber(Globals.Config.GetConfig("Options", "OptisetExtension", ""));
            return GetCallId(null, null, OptisetExtenstion);
        }

        public void CreatePhantomExtension(string ServerIPAndPort, string Prefix, string Extension)
        {
            //            lock (PhoneNumbers)
/*            {
                foreach (PhoneNumber pn in PhoneNumbers)
                    if (pn.GetExtension() == Extension)
                        return;
            }*/
            PhoneNumber pn2 = FactoryNewPhoneNumber();
            pn2.SetExtension(Extension);
        }

        public void RemovePhantomExtension(string Extension)
        {
            foreach (PhoneNumber pn in PhoneNumbers)
                if (pn.GetExtension() == Extension && pn.GetX() == 0 && pn.GetY() == 0)
                    return;
        }

        public void CreateOptisetPhantomExtension()
        {
            //if we are supposed to have an optiset, but it is not yet configured, ask the user for the configuration
            string OptiSet = Globals.Config.GetConfig("Options", "OptisetExtension", null);
            if (OptiSet == null)
            {
                new OptisetNew().Show();
                return;
            }
            //get the extension only from a full phone number
            string OptisetExtenstion = PhoneNumber.GetExtensionFromFullNumberStr(OptiSet);
            CreatePhantomExtension(null, null, OptisetExtenstion);
        }

        //new phone numbers are created too slow, create a background thread to start creating a few of these
        public PhoneNumber FactoryNewPhoneNumber()
        {
            return ExtensionFactory.Get();
        }

        public PhoneNumber FactoryNewPhoneNumber(int X, int Y, PhoneNumberSetupSettings settings, long OwnerGUID)
        {
            PhoneNumber pn = FactoryNewPhoneNumber();
            pn.Init(X, Y, settings, OwnerGUID);
            return pn;
        }

        public static void OnClientReceivedPacketWithExtension(NetworkClient nc, string Extension)
        {
            if (Globals.ExtensionManager != null)
                Globals.ExtensionManager.OnClientReceivedPacketWithExtension_(nc.ServerIPAndPort, "", Extension);
        }

        public static string GetMultiServerExtension(string ServerIPAndPort, string Prefix, string Extension)
        {
            string ret = "";
            if (Extension.IndexOf('-') < 0 && Extension.IndexOf('(') < 0)
                ret = ServerIPAndPort + "@" + Prefix + '-' + Extension;
            else
                ret = ServerIPAndPort + "@" + Extension;
            return ret;
        }

        public void OnClientReceivedPacketWithExtension_(string ServerIPAndPort, string Prefix, string Extension)
        {
            MonitorStatusStore ss = GetStatusStore(ServerIPAndPort, Prefix, Extension);
            ss.ServerIPAndPort = ServerIPAndPort;
            if (ss.Extension == null)
                ss.Extension = Extension;
            if (ss.Prefix == null)
                ss.Prefix = Prefix;
            ss.ServerIPAndPort = ServerIPAndPort;

        }
    }
}
