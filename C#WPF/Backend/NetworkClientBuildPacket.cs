using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLFClient.Backend
{
    public class NetworkClientBuildPacket
    {

        static bool SendMessageToSocket(object pkt) 
        {
            if (Globals.ConnectionManager == null || Globals.ConnectionManager.IsConnected() == false)
                return false;
            Globals.AntiFloodManager.OnPacketSent(); // some of our managers shoudl be able queue up packets without flooding the server
            return Globals.ConnectionManager.SendPacket(pkt);
        }

        public static bool SnapshotDevice(string device)
        {
            SnapshotDeviceStr Msg = new SnapshotDeviceStr();
            Msg.BEG = 0xaa;
            Msg.Size = (short)(System.Runtime.InteropServices.Marshal.SizeOf(Msg) - NetworkPacketTools.PCK_HDR_TRM_LEN());
	        Msg.Cmd = PacketTypes.SNAPSHOTDEVICE;
            Msg.END = 0x55;
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(device, Msg.Device); }
            if (SendMessageToSocket(Msg))
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "send msg type : SNAPSHOTDEVICE, device " + device);
                return true;
            }
            return false;
        }

        public static bool QueryDeviceForwarding(string device)
        {
            QueryDeviceForwardingStr Msg = new QueryDeviceForwardingStr();
            Msg.BEG = 0xaa;
	        Msg.Size = (short)(System.Runtime.InteropServices.Marshal.SizeOf(Msg) - NetworkPacketTools.PCK_HDR_TRM_LEN());
            Msg.Cmd = PacketTypes.QUERYDEVICEFORWARDING;
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(device, Msg.Device); }
            Msg.END = 0x55;
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "send msg type : QUERYDEVICEFORWARDING, device " + device);
            return SendMessageToSocket(Msg);
        }

        public static bool MonitorStart(string device)
        {
            MonitorStartStr Msg = new MonitorStartStr();
            Msg.BEG = 0xaa;
            Msg.Size = (short)(System.Runtime.InteropServices.Marshal.SizeOf(Msg) - NetworkPacketTools.PCK_HDR_TRM_LEN());
            Msg.Cmd = PacketTypes.MONITORSTART;
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(device, Msg.Device); }
            Msg.END = 0x55;
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "send msg type : MONITORSTART, device " + device);
            return SendMessageToSocket(Msg);
        }

        public static bool MonitorStop(string xrefId)
        {
            MonitorStopStr Msg = new MonitorStopStr();
            Msg.BEG = 0xaa;
            Msg.Size = (short)(System.Runtime.InteropServices.Marshal.SizeOf(Msg) - NetworkPacketTools.PCK_HDR_TRM_LEN());
            Msg.Cmd = PacketTypes.MONITORSTOP;
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(xrefId, Msg.XrefId); }
            Msg.END = 0x55;
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "send msg type : MONITORSTOP, xrefid " + xrefId);
            return SendMessageToSocket(Msg);
        }

        public static bool SetFeatureForwarding(string device, string fwd_dn, short type, short shOn)
        {
            SetFeatureForwardingStr Msg = new SetFeatureForwardingStr();
            Msg.BEG = 0xaa;
            Msg.Size = (short)(System.Runtime.InteropServices.Marshal.SizeOf(Msg) - NetworkPacketTools.PCK_HDR_TRM_LEN());
            Msg.Cmd = PacketTypes.SETFEATUREFORWARDING;
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(device, Msg.Device); }
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(fwd_dn, Msg.Fwd_dn); }
            Msg.Type = type;
	        Msg.ShOn = shOn;
	        Msg.END = 0x55;
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "send msg type : SETFEATUREFORWARDING, device " +device + " fwd_dn " + fwd_dn + " type "+ type.ToString() + " shon "+ shOn.ToString());
            return SendMessageToSocket(Msg);
        }

        public static bool ConsultationCall(string held_device_id, string called_device_id, string held_call_id, string pvt_uu_data, short pvt_uu_data_len)
        {
            ConsultationCallStr Msg = new ConsultationCallStr();
            Msg.BEG = 0xaa;
            Msg.Size = (short)(System.Runtime.InteropServices.Marshal.SizeOf(Msg) - NetworkPacketTools.PCK_HDR_TRM_LEN());
        	Msg.Cmd = PacketTypes.CONSULTATIONCALL;
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(held_device_id, Msg.Held_device_id); }
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(called_device_id, Msg.Called_device_id); }
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(held_call_id, Msg.Held_call_id); }
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(pvt_uu_data, Msg.Pvt_uu_data); }
            Msg.Pvt_uu_data_len = pvt_uu_data_len;
        	Msg.END = 0x55;
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "send msg type : CONSULTATIONCALL, held_device_id " + held_device_id + " called_device_id " + called_device_id + " Held_called_id " + held_call_id + " pvt_uu_data " + pvt_uu_data);
            return SendMessageToSocket(Msg);
        }

        public static bool TransferCall(string held_device_id, string active_device_id, string held_call_id, string active_call_id)
        {
            TransferCallStr Msg = new TransferCallStr();
            Msg.BEG = 0xaa;
            Msg.Size = (short)(System.Runtime.InteropServices.Marshal.SizeOf(Msg) - NetworkPacketTools.PCK_HDR_TRM_LEN());
            Msg.Cmd = PacketTypes.TRANSFERCALL;
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(held_device_id, Msg.Held_device_id); }
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(active_device_id, Msg.Active_device_id); }
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(held_call_id, Msg.Held_call_id); }
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(active_call_id, Msg.Active_call_id); }
            Msg.END = 0x55;
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "send msg type : TRANSFERCALL, held_device_id " + held_device_id + " active_device_id "+ active_device_id + " held_call_id "+ held_call_id + " active_call_id "+ active_call_id);
            return SendMessageToSocket(Msg);
        }

         public static bool SingleStepTransfer(string active_call_id, string active_device_id, string transfered_to_device_id, string pvt_uu_data, short pvt_uu_data_len)
        {
            SingleStepTransferStr Msg = new SingleStepTransferStr();
            Msg.BEG = 0xaa;
            Msg.Size = (short)(System.Runtime.InteropServices.Marshal.SizeOf(Msg) - NetworkPacketTools.PCK_HDR_TRM_LEN());
            Msg.Cmd = PacketTypes.SINGLESTEPTRANSFER;
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(active_call_id, Msg.Active_call_id); }
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(active_device_id, Msg.Active_device_id); }
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(transfered_to_device_id, Msg.Transfered_to_device_id); }
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(pvt_uu_data, Msg.Pvt_uu_data); }
            Msg.Pvt_uu_data_len = pvt_uu_data_len;
	        Msg.END = 0x55;
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "send msg type : SINGLESTEPTRANSFER, active_call_id " + active_call_id.ToString() + " active_device_id "+ active_device_id.ToString() + " transfer_to_device_id "+ transfered_to_device_id.ToString() + " pvt_uu_data " + pvt_uu_data);
            return SendMessageToSocket(Msg);
        }

        public static bool MakeCall(string calling_device, string called_directory_number, short shPrompt)
        {
            MakeCallStr Msg = new MakeCallStr();
            Msg.BEG = 0xaa;
            Msg.Size = (short)(System.Runtime.InteropServices.Marshal.SizeOf(Msg) - NetworkPacketTools.PCK_HDR_TRM_LEN());
            Msg.Cmd = PacketTypes.MAKECALL;
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(calling_device, Msg.Calling_device); }
            unsafe { NetworkPacketTools.StrinToBytes_MAX_STN_CHARNO(called_directory_number, Msg.Called_directory_number); }
            Msg.ShPrompt = shPrompt;
	        Msg.END = 0x55;
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "send msg type : MAKECALL, device " + calling_device.ToString() + " dir number " + called_directory_number.ToString() + " shpromt " + shPrompt.ToString());
            return SendMessageToSocket(Msg);
        }

        //OSFOURK-6654
        public static bool DbChecksumRequest()
        {
            DbGetChkRequestStr Msg = new DbGetChkRequestStr();
            Msg.BEG = 0xaa;
            Msg.Size = (short)(System.Runtime.InteropServices.Marshal.SizeOf(Msg) - NetworkPacketTools.PCK_HDR_TRM_LEN());
            Msg.Cmd = PacketTypes.DBGET_CHK_REQUEST;
        	Msg.END = 0x55;
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "send msg type : DBGET_CHK_REQUEST");
            return SendMessageToSocket(Msg);
        }
        //OSFOURK-8927
        public static bool ReqSystemStatus()
        {
            ReqSystemSatusStr Msg = new ReqSystemSatusStr();
            Msg.BEG = 0xaa;
            Msg.Size = (short)(System.Runtime.InteropServices.Marshal.SizeOf(Msg) - NetworkPacketTools.PCK_HDR_TRM_LEN());
            Msg.Cmd = PacketTypes.REQSYSTEMSTATUS;
        	Msg.END = 0x55;
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "send msg type : REQSYSTEMSTATUS");
            return SendMessageToSocket(Msg);
        }

        public static bool AliveTest(long newState)
        {
            AliveTestStr Msg = new AliveTestStr();
            Msg.BEG = 0xaa;
            Msg.Size = (short)(System.Runtime.InteropServices.Marshal.SizeOf(Msg) - NetworkPacketTools.PCK_HDR_TRM_LEN());
            Msg.Cmd = PacketTypes.ALIVETEST;
	        Msg.NewState = newState;
	        Msg.END = 0x55;
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "send msg type : ALIVETEST, new state " + newState.ToString());
            return SendMessageToSocket(Msg);
        }

        public static bool DBRequest(ushort chunk)
        {
            DbSendAckStr Msg = new DbSendAckStr();
            Msg.BEG = 0xaa;
            Msg.Size = (short)(System.Runtime.InteropServices.Marshal.SizeOf(Msg) - NetworkPacketTools.PCK_HDR_TRM_LEN());
            Msg.Cmd = PacketTypes.DBSEND_ACK;
        	Msg.Chunk = chunk; 
        	Msg.END = 0x55;
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "send msg type : DBSEND_ACK, chunk " + chunk.ToString());
            return SendMessageToSocket(Msg);
        }
    }
}
