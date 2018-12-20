using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Threading;
using System.Timers;
using System.Threading;

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

        public MonitorStatusStore(string Ext)
        {
            Extension = Ext;
            OnConnectionLost();
        }

        public bool ShouldQueryStatus()
        {
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
            return true;
        }

        public bool ExtensionExists()
        {
            return (LastStatus != PhoneStatusCodes.PHONE_DOESNOT);
        }
    }

    public class PhoneNumberManager
    {
        static long PhoneNumberGUID = 1;
        List<PhoneNumber> PhoneNumbers;
        List<MonitorStatusStore> ExtensionMonitors;
        System.Timers.Timer UpdateTimer = null;
        PhoneNumber LastClickedExtension = null;

        public PhoneNumberManager()
        {
            PhoneNumbers = new List<PhoneNumber>();
            ExtensionMonitors = new List<MonitorStatusStore>();
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
            while (Globals.ConnectionManager == null || Globals.ConnectionManager.IsConnected() == false || Globals.IndexCardsLoaded == false)
                Thread.Sleep(100);

            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Async extension status manager has started");
            //create a timer to periodically query extension forwarding status and issue callbacks to all cell cards on status change
            UpdateTimer = new System.Timers.Timer(100);
            UpdateTimer.Enabled = true; 
            UpdateTimer.Elapsed += new ElapsedEventHandler(PeriodicStatusUpdate);
        }

        private void PeriodicStatusUpdate(object source, ElapsedEventArgs arg)
        {
            //nothing to be done for now
            if (Globals.ConnectionManager == null || Globals.ConnectionManager.IsConnected() == false)
                return;

            UpdateTimer.Stop();// in case network buffer gets full, the function might block and threadpool might call us multiple times
            //get the list of all the extensions, query each for their forwarding status
            HashSet<string> extl = GetExtensions();
            foreach (string ext in extl)
            {
                //avoid burst flooding the server
                while (Globals.AntiFloodManager.CanSendNewPacket() == false)
                    Thread.Sleep(11); // should not have exact same value as other threads waiting on anti flood manager
                //safe to send a new packet
                PhoneNumberQueryDeviceStatus(ext);
            }
            UpdateTimer.Start();
        }

        private void PhoneNumberQueryDeviceStatus(string Extension)
        {
            //ignore dummy extensions
            if (Extension.Length == 0)
                return;

            MonitorStatusStore ss = GetStatusStore(Extension);
            if (ss.ShouldQueryStatus() == true)
            {
                ss.LastQueryStamp = Environment.TickCount; // if there is no reply comming for this extension, we should retry a bit later
                ss.PacketStatus = ServerPacketStatus.PacketSent;
                NetworkClientBuildPacket.SnapshotDevice(Extension.ToString());
            }
        }

        private MonitorStatusStore GetStatusStore(string Extension)
        {
            foreach (MonitorStatusStore it in ExtensionMonitors)
                if (it.Extension == Extension)
                    return it;

            //if we got here, than we need to create a new store
            MonitorStatusStore fw = new MonitorStatusStore(Extension);
            ExtensionMonitors.Add(fw);

            return fw;
        }

        private MonitorStatusStore GetStatusStoreXRef(long Xref)
        {
            foreach (MonitorStatusStore it in ExtensionMonitors)
                if (it.XrefId == Xref)
                    return it;

            return null;
        }

        public void PhoneNumberAdd(PhoneNumber pn)
        {
            pn.SetGUID(PhoneNumberGUID++);
            lock (PhoneNumbers)
            {
                PhoneNumbers.Add(pn);
            }
        }

        public void PhoneNumberDelete(PhoneNumber pn)
        {
            lock (PhoneNumbers)
            {
                PhoneNumbers.Remove(pn);
            }
        }

        public PhoneNumber PhoneNumberGet(long GUID)
        {
            lock (PhoneNumbers)
            {
                foreach (PhoneNumber pn in PhoneNumbers)
                    if (pn.GetGUID() == GUID)
                        return pn;
            }
            return null;
        }

        public PhoneNumber PhoneNumberGetFirst(string Extension)
        {
            lock (PhoneNumbers)
            {
                foreach (PhoneNumber pn in PhoneNumbers)
                    if (pn.GetExtension() == Extension)
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

        public PhoneNumber PhoneNumberGetByXRef(long XRef)
        {            
            MonitorStatusStore ss = GetStatusStoreXRef(XRef);
            if (ss != null)
                return PhoneNumberGetFirst(ss.Extension);
            return null;
        }

        public void OnConnectionChanged(bool Connected)
        {
            lock (PhoneNumbers)
            {
//                List<PhoneNumber> PhoneNumbers2 = new List<PhoneNumber>(PhoneNumbers); // this might bug out due to concurency. For some reason lock deadlocked. Maybe use a different lock
                foreach (PhoneNumber pn in PhoneNumbers)
                    pn.OnConnectionChanged(Connected);
            }
            //set all monitors to false(even if this is a connect). Maybe server restarted and lost all monitors ?
            foreach (MonitorStatusStore it in ExtensionMonitors)
                it.OnConnectionLost();
        }

        public void OnServerExtensionNameReceive(string Extension, string Name)
        {
            List<PhoneNumber> PhoneNumbers2;
            lock (PhoneNumbers)
            {
                PhoneNumbers2 = new List<PhoneNumber>(PhoneNumbers); // this might bug out due to concurency. For some reason lock deadlocked. Maybe use a different lock
            }
            foreach (PhoneNumber pn in PhoneNumbers2)
                if (pn.GetExtension() == Extension)
                    if (App.Current != null)
                        App.Current.Dispatcher.Invoke(DispatcherPriority.Normal, (Action)(() =>
                        {
                            if (App.Current == null)
                                return;

                            MainWindow MainObject = (MainWindow)App.Current.MainWindow;

                            if (MainObject == null)
                                return;

                            pn.SetName(Name);
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

        public void OnMonitorStart(string Extension, string xRef)
        {
            MonitorStatusStore ss = GetStatusStore(Extension);
            ss.XrefId = Int32Parse(xRef, 0);
            ss.MonitorPacketStatus = ServerPacketStatus.PacketReceived;
        }

        public void OnMonitorStop(string pxRef)
        {
            int xRef = Int32Parse(pxRef, -1);
            MonitorStatusStore ss = GetStatusStoreXRef(xRef);
            if (ss != null)
            {
                ss.XrefId = 0;
                ss.MonitorPacketStatus = ServerPacketStatus.PacketReceived; // it's bad, but the best we can do
                return;
            }
        }

        public void OnSystemStatusOk()
        {
            //mark all extensions with valid monitors as they were updated recently
            foreach (MonitorStatusStore it in ExtensionMonitors)
                if (it.XrefId != 0)
                    it.LastStatusUpdateStamp = Environment.TickCount;
        }

        public void OnStatusChange(string Extension, PhoneStatusCodes NewStatus)
        {
            //mark that we are alive
            MonitorStatusStore ss = GetStatusStore(Extension);
            ss.LastStatusUpdateStamp = Environment.TickCount;
            ss.PacketStatus = ServerPacketStatus.PacketReceived;
            ss.LastStatus = NewStatus;

            //try to attach a monitor to it
            if( ss.LastStatus != PhoneStatusCodes.PHONE_DOESNOT && ss.ShouldTryAttachMonitor() == true )
            {
                ss.MonitorPacketStatus = ServerPacketStatus.PacketSent;
                ss.LastMonitorQueryStamp = Environment.TickCount;
                NetworkClientBuildPacket.MonitorStart(Extension);
            }

            string ExtensionRange = Extension.Substring(0, Extension.Length - 1);
            List<PhoneNumber> PhoneNumbers2;
            lock (PhoneNumbers)
            {
                PhoneNumbers2 = new List<PhoneNumber>(PhoneNumbers); // this might bug out due to concurency. For some reason lock deadlocked. Maybe use a different lock
            }
            foreach (PhoneNumber pn in PhoneNumbers2)
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
            lock (PhoneNumbers)
            try
            {
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
            }
            catch { }
            return ret;
        }

        public HashSet<string> GetExtensionsQueryForward()
        {
            HashSet<string> ret = new HashSet<string>();
            lock (PhoneNumbers)
            try
            {
                foreach (PhoneNumber pn in PhoneNumbers)
                    if (pn.GetExtension().Length > 0 && pn.GetGUID() != 0)
                    {
                        MonitorStatusStore ss = GetStatusStore(pn.GetExtension());
                        if ( ss.LastStatus != PhoneStatusCodes.PHONE_DOESNOT && ss.ShouldQueryStatus() == false && ss.PacketStatus == ServerPacketStatus.PacketReceived)
                            ret.Add(pn.GetExtension());
                    }
            }
            catch { }
            return ret;
        }

        public void OnForwardingStatusUpdate(ForwardStatusStore fs)
        {
            List<PhoneNumber> PhoneNumbers2;
            lock (PhoneNumbers)
            {
                PhoneNumbers2 = new List<PhoneNumber>(PhoneNumbers); // this might bug out due to concurency. For some reason lock deadlocked. Maybe use a different lock
            }
            foreach (PhoneNumber pn in PhoneNumbers2)
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
            lock (PhoneNumbers)
            try
            {
                foreach (PhoneNumber pn in PhoneNumbers)
                    if (pn.GetEmail() != null && pn.GetEmail().Length > 0 && pn.GetGUID() != 0)
                        ret.Add(pn.GetEmail());
            }
            catch { }
            return ret;
        }

        public void OnAbsenceStatusUpdate(string Email, bool Available)
        {
            List<PhoneNumber> PhoneNumbers2;
            lock (PhoneNumbers)
            {
                PhoneNumbers2 = new List<PhoneNumber>(PhoneNumbers); // this might bug out due to concurency. For some reason lock deadlocked. Maybe use a different lock
            }
            foreach (PhoneNumber pn in PhoneNumbers2)
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

        public bool HasMonitor(string Extension)
        {
            MonitorStatusStore ss = GetStatusStore(Extension);
            return ss.HasMonitorSet();
        }

        /// <summary>
        /// An extension can exist and be offline. We can set a monitor on an offline extension. We can NOT set a monitor for an unexisting extension
        /// </summary>
        /// <param name="Extension"></param>
        /// <returns></returns>
        public bool ExtensionExists(string Extension)
        {
            MonitorStatusStore ss = GetStatusStore(Extension);
            return ss.ExtensionExists();
        }

        public PhoneStatusCodes GetCachedStatus(string Extension)
        {
            MonitorStatusStore ss = GetStatusStore(Extension);
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

        public void OnCallIdReceived(string CallerDevice, string CallId, string CalledDevice)
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

            MonitorStatusStore ss = GetStatusStore(CallerDevice);
            ss.CallId = CallIdInt;
            ss.CallPartner = Int32Parse(CalledDevice, 0);

            ss = GetStatusStore(CalledDevice);
            ss.CallId = CallIdInt;
            ss.CallPartner = Int32Parse(CallerDevice, 0);
        }

        public void OnCallIdClear(string CallerDevice, string CallId, string CalledDevice)
        {
            OnCallIdReceived(CallerDevice, "0", CalledDevice);
        }

        public long GetCallId(string Extension)
        {
            foreach (MonitorStatusStore it in ExtensionMonitors)
                if (it.Extension == Extension)
                    return it.CallId;
            return 0;
        }

        public long GetOptisetCallId()
        {
            string OptisetExtenstion = PhoneNumber.GetExtensionFromFullNumber(Globals.Config.GetConfig("Options", "OptisetExtension", ""));
            return GetCallId(OptisetExtenstion);
        }

        public long GetOptisetCallTarget()
        {
            string OptisetExtenstion = PhoneNumber.GetExtensionFromFullNumber(Globals.Config.GetConfig("Options", "OptisetExtension", ""));
            foreach (MonitorStatusStore it in ExtensionMonitors)
                if (it.Extension == OptisetExtenstion)
                    return it.CallId;
            return 0;
        }

        public void OnConsultaionCallIdReceived(string CallerDevice, string CallId, string CalledDevice)
        {
            if (CallerDevice[0] == 'N')
                CallerDevice = CallerDevice.Substring(1);
            if (CalledDevice[0] == 'N')
                CalledDevice = CalledDevice.Substring(1);

//            int CallerDeviceInt = Int32Parse(CallerDevice, 0);
            int CallIdInt = Int32Parse(CallId, 0);
//            int CalledDeviceInt = Int32Parse(CalledDevice, 0);

            MonitorStatusStore ss = GetStatusStore(CallerDevice);
            ss.CallIdConsultation = CallIdInt;
            ss.CallPartnerConsultation = Int32Parse(CalledDevice, 0);

            ss = GetStatusStore(CalledDevice);
            ss.CallIdConsultation = CallIdInt;
            ss.CallPartnerConsultation = Int32Parse(CallerDevice, 0);
        }

        public void OnConsultaionCallIdClear(string CallerDevice, string CallId, string CalledDevice)
        {
            OnConsultaionCallIdReceived(CallerDevice, "0", CalledDevice);
        }

        public long GetConsultaionCallId(string Extension)
        {
            foreach (MonitorStatusStore it in ExtensionMonitors)
                if (it.Extension == Extension)
                    return it.CallIdConsultation;
            return 0;
        }

        public long GetOptisetConsultaionCallId()
        {
            string OptisetExtenstion = PhoneNumber.GetExtensionFromFullNumber(Globals.Config.GetConfig("Options", "OptisetExtension", ""));
            return GetCallId(OptisetExtenstion);
        }

        public void CreatePhantomExtension(string Extension)
        {
            //            lock (PhoneNumbers)
/*            {
                foreach (PhoneNumber pn in PhoneNumbers)
                    if (pn.GetExtension() == Extension)
                        return;
            }*/
            PhoneNumber pn2 = new PhoneNumber(0, 0, new PhoneNumberSetupSettings(), -1);
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
            CreatePhantomExtension(OptisetExtenstion);
        }
    }
}
