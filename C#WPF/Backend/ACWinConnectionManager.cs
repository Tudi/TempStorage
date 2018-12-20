using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace BLFClient.Backend
{
    public enum SCallInfoParamFlags
    {
        CIPS_IDLE = 0x00000001,
        CIPS_ACCEPTED = 0x00000002,
        CIPS_DIALING = 0x00000004,
        CIPS_CONNECTED = 0x00000008,
        CIPS_ONHOLDPENDTRANSFER = 0x00000010,
        CIPS_ONHOLD = 0x00000020,
        CIPS_PARKED = 0x00000040,
        CIPM_INCOMING = 0x00000001,
        CIPM_OUTGOING = 0x00000002,
    }

    public class ConstantsACWin
    {
        public const int MAX_BUFF = 8291;
        public const string ACWIN_TYPE_RES = "RES";
        public const string ACWIN_TYPE_REQ = "REQ";
        public const string ACWIN_TYPE_ERR = "ERR";
        public const string ACWIN_CMD_INITIALIZE = "Initialize";
        public const string ACWIN_CMD_UNINITIALIZE = "Uninitialize";
        public const string ACWIN_CMD_REGISTER = "Register";
        public const string ACWIN_CMD_UNREGISTER = "Unregister";
        public const string ACWIN_CMD_GENERAL_ERROR = "Error";
        public const string SERVICE_AC_MAKE_CALL = "ACMakeCall";
        public const string SERVICE_SERV_CALL_INFO = "ServCallInfo";
        public const int AC_PASSWORD_LENGTH_SOCKET = 40;
        public const int NUM_MAX_BYTES_PER_CHAR_UTF8 = 4;
    }
    /*
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct SCallInfoParamAC
    {
        public long m_lState;
        public long m_lMisc;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct SProto
    {
        public fixed byte m_protocol[ConstantsACWin.MAX_BUFF + ConstantsACWin.MAX_BUFF];       // byte-array holding socket-protocol
//        public int m_length;                           // actualy length of socket-protocol
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct SProtocolMakeCall
    {
        public fixed byte m_protocol[ConstantsACWin.MAX_BUFF + ConstantsACWin.MAX_BUFF];       // byte-array holding socket-protocol
//        public int m_length;                           // actualy length of socket-protocol
    };
    */
    public partial class ACWinConnectionManager
    {
        int m_dwACWinConfig;
//        SCallInfoParamAC m_CallInfoParam;
        private NetworkStream NetStream = null;
        private TcpClient client_tcpClient = null;
        System.Threading.Timer UpdateTimer;
        int PacketSizeCur = 0;
        int PacketSizeRecv = 0;
        byte[] packet = null;

        public ACWinConnectionManager()
        {
            //ACWin not installed ? Nothing to do here
            if (CheckACWin() == false)
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Config says we should have ACWin connection, but ACWin is not installed ?");
                return;
            }
            //update which version of ACWin packetizer are we using
            SetACWinConfigType();

            /*           m_CallInfoParam.m_lState = (long)(SCallInfoParamFlags.CIPS_CONNECTED |
                                                  SCallInfoParamFlags.CIPS_ONHOLDPENDTRANSFER |
                                                  SCallInfoParamFlags.CIPS_ONHOLD |
                                                  SCallInfoParamFlags.CIPS_PARKED);

                       m_CallInfoParam.m_lMisc = (long)SCallInfoParamFlags.CIPM_OUTGOING;*/
            UpdateTimer = new System.Threading.Timer(PeriodicUpdate, null, 1000, 100);
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Created ACWin connection manager");
        }

        ~ACWinConnectionManager()
        {
            // disable update function getting called
            UpdateTimer.Dispose();

            //should wait until the update function exits
            lock(this)
            {
                //dispose of the network connection we had
                if (client_tcpClient != null && client_tcpClient.Connected == true)
                {
                    client_tcpClient.Close();
                    client_tcpClient.Dispose();
                    client_tcpClient = null;
                }

                //should force read / write threads to stop doing stuff
                if (NetStream != null)
                    NetStream.Close();
                if (NetStream != null)
                    NetStream.Dispose();
                NetStream = null;
            }
        }

        private void PeriodicUpdate(object source)
        {
            if (Monitor.TryEnter(this))
            {
                //are you sure we should be still using ACWin connection ?
                if (Globals.Config.GetConfig("Options", "ACWin", "NO") != "YES")
                {
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Disabled ACWin connection manager. Config file changed while program was running");
                    UpdateTimer.Dispose();
                    Monitor.Exit(this);
                    return;
                }
                //check if we have a working connection. If not yet, try to create a new one
                if (client_tcpClient == null || client_tcpClient.Connected == false)
                    CreateNewACWinConnection();
                //if we failed to create a connection, bail out
                if (client_tcpClient == null || client_tcpClient.Connected == false)
                {
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Failed to create ACWin connection");
                    Monitor.Exit(this);
                    return;
                }
                //something must have went very wrong if we are missing a stream
                if (NetStream == null)
                {
                    Monitor.Exit(this);
                    return;
                }
                //new packet ? Read size for it
                if (PacketSizeCur == 0)
                {
                    //check if we have packets to read
                    byte[] SizeBytes = new byte[4];
                    try
                    {
                        int ReadCount = NetStream.Read(SizeBytes, 0, 4);
                        if (ReadCount == 4)
                        {
                            PacketSizeCur = NetworkPacketTools.ByteArrayToInt(SizeBytes, 0);
                            PacketSizeRecv = 0;
                            packet = new byte[PacketSizeCur + 4];
                            packet[0] = SizeBytes[0];
                            packet[1] = SizeBytes[1];
                            packet[2] = SizeBytes[2];
                            packet[3] = SizeBytes[3];
//                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "ACWin recv : " + PacketSizeCur.ToString() + " bytes");
                        }
                    } catch{ };
                }
                //incomplete packet ? Read some more for it
                if (PacketSizeRecv < PacketSizeCur)
                { 
                    int ReadCount = NetStream.Read(packet, 4, PacketSizeCur - PacketSizeRecv);
                    PacketSizeRecv += ReadCount;
                    //parse the packet
                }
                //complete packet ? Parse it
                if (PacketSizeRecv == PacketSizeCur)
                {
                    OnPacketReceived(packet, PacketSizeCur);
                    PacketSizeCur = 0;
                }
                Monitor.Exit(this);
            }
        }

        private void CreateNewACWinConnection()
        {
            NetStream = null;
            client_tcpClient = new TcpClient();
            int OLE_Port = 54700;
            try
            {
                RegistryKey key = Registry.LocalMachine.OpenSubKey("SOFTWARE\\Wow6432Node\\Siemens\\ACWin\\IP\\Connection");
                if (key != null)
                {
                    OLE_Port = PhoneNumberManager.Int32Parse( (string)key.GetValue("socket-port-ole-interface"), OLE_Port );
                    key.Close();
                }
                this.client_tcpClient.Connect("localhost", OLE_Port);
            }
            catch
            {
                // Log error that we were not able to connect to the server
                client_tcpClient = null;
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "Could not connect to ACWin using connection details localhost:" + OLE_Port.ToString());
                return;
            }
            NetStream = client_tcpClient.GetStream();
            NetStream.ReadTimeout = 1000; //1 second timeout should be plenty
            //initialize the AVWinConnection
            InitializeACWinConnection();
        }

        private void OnPacketReceived(byte [] packet, int Size)
        {
            if (ParseResponseMakeCall(packet))
                return;
            if (ParseResponseInitialize(packet))
            {
                //send the register service request
                RegisterService();
                return;
            }
            if (ParseResponseRegister(packet))
                return;
            if(ParseCallInfo(packet))
            {
                CallInfoResponse();
                return;
            }
            if (ParseResponseError(packet))
                return;
        }

        public void MakeCall(string Prefix, string Extension)
        {
            string strExtensionTemp = Prefix + "-" + Extension; 
            switch (m_dwACWinConfig)
            {
                case 1:
                    //1 version: +36(1)19-3156 => 00361193156
                    strExtensionTemp.Replace("(", "");
                    strExtensionTemp.Replace(")", "");
                    strExtensionTemp.Replace("-", "");
                    strExtensionTemp.Replace("+", "00");
                    break;
                case 2:
                    //2. version +36(1)19-3156 => 193156
                    strExtensionTemp.Replace("-", "");
                    strExtensionTemp = strExtensionTemp.Substring(strExtensionTemp.LastIndexOf(')') + 1);
                    break;
                case 3:
                    //3. version +36(1)19-3156 => 3156
                    strExtensionTemp = Extension.ToString();
                    break;
            }

            MakeCall(strExtensionTemp);
        }

        public unsafe void SendPacket(byte [] packet, int length)
        {
            if (NetStream == null)
                return;
            try
            {
                NetStream.Write(packet, 0, length);
            }
            catch(Exception e)
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "Could not send packet to ACWin. Exception " + e.ToString());
            }
        }

        private bool CheckACWin()
        {
            RegistryKey key = Registry.LocalMachine.OpenSubKey("SOFTWARE\\Siemens\\ACWin");
            if (key != null)
            {
                key.Close();
                return true;
            }
            return false;
        }

        private void SetACWinConfigType()
        {
            //if nothing else better, set it to a fixed value
            m_dwACWinConfig = 3;
            //try to load it from config file
            int dwACWinConfig = Globals.Config.GetConfigInt("Options", "AcWinConfigType", -1);
            if(dwACWinConfig != -1)
            {
                m_dwACWinConfig = dwACWinConfig;
                return;
            }
            // try to read it from registry
            RegistryKey key = Registry.LocalMachine.OpenSubKey("SOFTWARE\\Siemens\\BLF-Win 3.0 Client");
            if (key == null)
                return;
            if (key.GetValue("AcWinConfigType") == null)
                return;
            m_dwACWinConfig = (int)key.GetValue("AcWinConfigType");
            //remember for next time
            Globals.Config.SetConfig("Options", "AcWinConfigType", m_dwACWinConfig.ToString());
            Globals.Config.SaveIni();
        }
    }
}
