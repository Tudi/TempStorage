using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Formatters.Binary;
using System.Text;
using System.Threading.Tasks;

namespace BLFClient.Backend
{
    /*    public const int MAX_ERR_CAT_CHARNO = 64;
        public const int MAX_ERR_VAL_CHARNO = 96;
        // in case of Dbchunk. 1024b payload + 7 byte hader+terminator
        public const int MAX_PACKET_LEN = 1031;
        // Packet begin=1,size=2,end=1, lenght in bytes
        public const int PCK_HDR_TRM_LEN = 4;
        public const int SNAPSHOTDEVICE = 5;
        */

    class ServerSidePhoneStatusCodes
    {
        public const int ZS_KONFERENZ = 0x0001;
        public const int ZS_RUF = 0x0002;
        public const int ZS_GESPRAECH = 0x0003;
        public const int ZS_RUHE = 0x0004;   //Idle
        public const int ZS_WAHLAUFNAHME = 0x0005;
        public const int ZS_BESETZT = 0x0006;    //Busy
        public const int ZS_BLOCKIERT = 0x0007;
        public const int ZS_WARTE = 0x0008;
        public const int ZS_RUECKFRAGE = 0x0009;
        public const int ZS_EXTERNVERBINDUNG = 0x000A;   //External call
        public const int ZS_WARTE_AUF_PLATZ = 0x000B;
        public const int ZS_AO_WARTE = 0x000C;
        public const int ZS_AO_PLATZ_FREI = 0x000D;
        public const int ZS_NACHRUECKEN = 0x000E;
        public const int ZS_GESPERRT = 0x000F;
        public const int ZS_DOESNOT = 0x0010;    //Does not exist
        public const int ZS_STARTED = 0x0011;
        public const int ZS_DOOS = 0x0012;   //Device Out of service
        public const int ZS_UMLEITUNG = 0x0100; // Forward
        public const int ZS_EXTERNAL = 0x1000;   //External call
        public const int ZS_NOT_FWD = 0xF0FF;    //Removing forwarding state 
        public const int ZS_NOT_EXT = 0x0FFF;    //Removing external call state 
    };

    class MakeCallPrompTypes
    {
        public const short make_call_prompt = 0;
        public const short make_call_doNotPrompt = 1;
    };

    class ClientSidePhoneStatusCodes
    {
    };

    class Constants
    {
        public const int MAX_STN_CHARNO = 32;
        public const int MAX_ERR_CAT_CHARNO = 64;
        public const int MAX_ERR_VAL_CHARNO = 96;
    }

    class PacketTypes
    {
        public const int SNAPSHOTDEVICE = 5;
        public const int QUERYDEVICEFORWARDING = 6;
        public const int MONITORSTART = 7;
        public const int MONITORSTOP = 8;
        public const int SETFEATUREFORWARDING = 9;
        public const int CONSULTATIONCALL = 10;
        public const int TRANSFERCALL = 11;
        public const int SINGLESTEPTRANSFER = 12;
        public const int MAKECALL = 13;
        public const int DBGET_CHK_REQUEST = 14;
        public const int DBSEND_ACK = 15;
        public const int ALIVETEST = 16;
        public const int REQSYSTEMSTATUS = 17;


        public const int ON_CONNECTED = 101;
        public const int ON_DISCONNECTED = 102;
        public const int SYSTEMSTATUSSERVICEREQUEST = 103;
        public const int UNIVERSALERRORRESPONSE = 104;
        public const int SNAPSHOTDEVICERESPONSE = 105;
        public const int QUERYDEVICEFORWARDINGRESPONSE = 106;
        public const int MONITORSTARTRESPONSE = 107;
        public const int MONITORSTOPRESPONSE = 108;
        public const int SETFEATUREFORWARDINGRESPONSE = 109;
        public const int CONSULTATIONCALLRESPONSE = 110;
        public const int TRANSFERCALLRESPONSE = 111;
        public const int SINGLESTEPTRANSFERRESPONSE = 112;
        public const int MAKECALLRESPONSE = 113;
        public const int UPDATEDEVICESTATE = 114;
        public const int UPDATEFORWARDSTATESTR = 115;
        public const int ESTABLISHEDEVENT = 116;
        public const int NETWORKREACHEDEVENT = 117;
        public const int CONNECTIONCLEAREDEVENT = 118;
        public const int MONITORSTOPREQUEST = 119;
        public const int DIVERTEDEVENT = 120;
        public const int DBSENDCHK = 121;
        public const int DBCHUNK = 122;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct BLFWinHeader
    {
        public byte BEG;
        public short Length;
        public byte Type;
    };

    //#define	SNAPSHOTDEVICE			5
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct SnapshotDeviceStr
    {
        public byte BEG;
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
        public byte END;
    };

    //#define QUERYDEVICEFORWARDING	6
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct QueryDeviceForwardingStr         //Message Coder Helper Structure
    {
        public byte BEG;
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
        public byte END;
    };

    //#define MONITORSTART			7
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct MonitorStartStr            //Message Coder Helper Structure
    {
        public byte BEG;
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
        public byte END;
    };

    //#define MONITORSTOP				8
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct MonitorStopStr
    {
        public byte BEG;
        public short Size;
        public byte Cmd;
        public fixed byte  XrefId[Constants.MAX_STN_CHARNO];
        public byte END;
    };

    //#define SETFEATUREFORWARDING	9
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct SetFeatureForwardingStr        //Message Coder Helper Structure
    {
        public byte BEG;
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
        public fixed byte  Fwd_dn[Constants.MAX_STN_CHARNO];
        public short Type;
        public short ShOn;
        public byte END;
    };

    //#define CONSULTATIONCALL		10
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct ConsultationCallStr        //Message Coder Helper Structure
    {
        public byte BEG;
        public short Size;
        public byte Cmd;
        public fixed byte  Held_device_id[Constants.MAX_STN_CHARNO];
        public fixed byte  Called_device_id[Constants.MAX_STN_CHARNO];
        public fixed byte  Held_call_id[Constants.MAX_STN_CHARNO];
        public fixed byte  Pvt_uu_data[Constants.MAX_STN_CHARNO];
        public short Pvt_uu_data_len;
        public byte END;
    };

    //#define TRANSFERCALL			11
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct TransferCallStr        //Message Coder Helper Structure
    {
        public byte BEG;
        public short Size;
        public byte Cmd;
        public fixed byte  Held_device_id[Constants.MAX_STN_CHARNO];
        public fixed byte  Active_device_id[Constants.MAX_STN_CHARNO];
        public fixed byte  Held_call_id[Constants.MAX_STN_CHARNO];
        public fixed byte  Active_call_id[Constants.MAX_STN_CHARNO];
        public byte END;
    };

    //#define SINGLESTEPTRANSFER		12
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct SingleStepTransferStr      //Message Coder Helper Structure
    {
        public byte BEG;
        public short Size;
        public byte Cmd;
        public fixed byte  Active_call_id[Constants.MAX_STN_CHARNO];
        public fixed byte  Active_device_id[Constants.MAX_STN_CHARNO];
        public fixed byte  Transfered_to_device_id[Constants.MAX_STN_CHARNO];
        public short Pvt_uu_data_len;
        public fixed byte  Pvt_uu_data[Constants.MAX_STN_CHARNO];
        public byte END;
    };

    //#define MAKECALL				13
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct MakeCallStr            //Message Coder Helper Structure
    {
        public byte BEG;
        public short Size;
        public byte Cmd;
        public fixed byte  Calling_device[Constants.MAX_STN_CHARNO];
        public fixed byte  Called_directory_number[Constants.MAX_STN_CHARNO];
        public short ShPrompt;
        public byte END;
    };

    /*
    #define CLEARCONNECTION			15
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct ClearConnectionStr			//Message Coder Helper Structure
    {
        public byte BEG;		
        public short Size;
        public byte Cmd;
        char Device_id[Constants.MAX_STN_CHARNO];
        char Call_id[Constants.MAX_STN_CHARNO];
        public byte END;		
    };
    */
    /*#define GETDATABASE		14
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct GetDatabaseStr
    {
        public byte BEG;
        public short Size;
        public byte Cmd;

        public byte END;
    };*/

    //OSFOURK-6654
    //#define DBGET_CHK_REQUEST 14 /* request to server to send the current DB XOR16 Checksum */
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct DbGetChkRequestStr
    {
        public byte BEG;
        public short Size;
        public byte Cmd;
        public byte END;
    };

    //OSFOURK-6654
    //To start the DB download, you have to make an ACK with Chunk value set to 0xffff.
    //Send the Ack for the current chunk. Client send to the server as response for DbChunkHdrStr
    //#define DBSEND_ACK 15 /* request to server to send the current Database */
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct DbSendAckStr
    {
        public byte BEG;               //Message beginning 0xaa
        public short Size;          //Message size from field CMD to last byte of the current chunk
        public byte Cmd;
        public ushort Chunk;         //Xor Checksum of the DB
        public byte END;               //Message terminator 0x55
    };

    //#define ALIVETEST		16
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct AliveTestStr
    {
        public byte BEG;
        public short Size;
        public byte Cmd;
        public long NewState;
        public byte END;
    };

    //#define REQSYSTEMSTATUS 17
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct ReqSystemSatusStr
    {
        public byte BEG;
        public short Size;
        public byte Cmd;
        public byte END;
    };

    //#define ON_CONNECTED				101
    //#define ON_DISCONNECTED				102
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct ConnectedStr
    {
        public short Size;
        public byte Cmd;
    };

    //~~~~~~~~~//

    //#define SYSTEMSTATUSSERVICEREQUEST	103
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct SystemStatusServiceRequestStr
    {
        public short Size;
        public byte Cmd;
        public short Type;
    };

    //#define UNIVERSALERRORRESPONSE		104
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct UniversalErrorResponseStr
    {
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
        public fixed byte  ErrorCategory[Constants.MAX_ERR_CAT_CHARNO];
        public fixed byte  ErrorValue[Constants.MAX_ERR_VAL_CHARNO];
    };

    //#define SNAPSHOTDEVICERESPONSE		105
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct SnapshotDeviceResponseStr
    {
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
        public long State;
    };

    //#define QUERYDEVICEFORWARDINGRESPONSE	106
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct QueryDeviceForwardingResponseStr
    {
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
    };

    //#define MONITORSTARTRESPONSE		107
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct MonitorStartResponseStr
    {
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
        public fixed byte  Xref[Constants.MAX_STN_CHARNO];
    };

    //#define MONITORSTOPRESPONSE			108
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct MonitorStopResponseStr
    {
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
    };

    //#define SETFEATUREFORWARDINGRESPONSE	109
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct SetFeatureForwardingResponseStr
    {
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
    };

    //#define CONSULTATIONCALLRESPONSE	110
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct ConsultationCallResponseStr
    {
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
        public fixed byte  CallID[Constants.MAX_STN_CHARNO];
        public fixed byte  DeviceID[Constants.MAX_STN_CHARNO];
    };

    //#define TRANSFERCALLRESPONSE		111
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct TransferCallResponseStr
    {
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
    };

    //#define SINGLESTEPTRANSFERRESPONSE	112
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct SingleStepTransferResponseStr
    {
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
    };

    //#define MAKECALLRESPONSE			113
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct MakeCallResponseStr
    {
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
    };

    //#define UPDATEDEVICESTATE			114
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct UpdateDeviceStateStr
    {
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
        public short State;
    };

    //#define UPDATEFORWARDSTATESTR 115
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct UpdateForwardStateStr
    {
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
        public fixed byte  pvt_fwd_dn[Constants.MAX_STN_CHARNO];
        public fixed byte  pvt_type[Constants.MAX_STN_CHARNO];
        public short shForwardDefault;
        public short shForwardStatus;
    };

    //#define ESTABLISHEDEVENT			116
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct EstablishedEventStr
    {
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
        public fixed byte  CallId[Constants.MAX_STN_CHARNO];
        public fixed byte  DeviceId[Constants.MAX_STN_CHARNO];
        public fixed byte  AnsweringDevice[Constants.MAX_STN_CHARNO];
        public fixed byte  CallingDevice[Constants.MAX_STN_CHARNO];
        public fixed byte  CalledDevice[Constants.MAX_STN_CHARNO];
        public fixed byte  LastRedirectionDevice[Constants.MAX_STN_CHARNO];
        public short shLCS;
        public short shCause;
        public short shExternal;
    };

    //#define NETWORKREACHEDEVENT			117
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct NetworkReachedEventStr
    {
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
        public fixed byte  called_device_id[Constants.MAX_STN_CHARNO];
    };

    //#define CONNECTIONCLEAREDEVENT		118
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct ConnectionClearedEventStr
    {
        public short Size;
        public byte Cmd;
        public fixed byte  Device[Constants.MAX_STN_CHARNO];
        public fixed byte  DroppedCall[Constants.MAX_STN_CHARNO];
        public fixed byte  DroppedDevice[Constants.MAX_STN_CHARNO];
        public fixed byte  ReleasingDevice[Constants.MAX_STN_CHARNO];
        public short shLocalConnectionState;
        public short shEventCause;
    };

    //#define MONITORSTOPREQUEST			119	
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct MonitorStopRequestStr
    {
        public short Size;
        public byte Cmd;
        public fixed byte  XREFId[Constants.MAX_STN_CHARNO];
    };

    //#define DIVERTEDEVENT				120
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct DivertedEventStr
    {
        public short Size;
        public byte Cmd;
        public fixed byte  XREFId[Constants.MAX_STN_CHARNO];
    };

    //OSFOURK-6654
    //#define DBSENDCHK	121
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct DbSendChkStr
    {
        public short Size;          //Message size from field CMD to last byte of the current chunk
        public byte Cmd;
        public ushort Xchk;          //Xor Checksum of the DB
    };

    //OSFOURK-6654
    //#define DBCHUNK	122
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct DbChunkHdrStr
    {
        public short Size;          //Message size from field CMD to last byte of the current chunk
        public byte Cmd;
        public ushort MaxChunk; //Max chunks
        public ushort Chunk; //current chunk
                            //unsigned char data[0];	// (Size-5)bytes
    };
















    // used these to pretest the system. Can ignore now
    public enum PacketType
    {

        NoTypeDefined = 0, //it's nothing on this code.
        login
    }

    //this should be present in all packets that we send or receive
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    class PacketHeader
    {
        public byte BEG;
        public short Length;
        public int Type;
        public PacketHeader()
        {
            PacketHeader pkt = new PacketHeader(1);
            Length = (short)System.Runtime.InteropServices.Marshal.SizeOf(pkt);
            Type = (int)PacketType.NoTypeDefined;
        }
        private PacketHeader(int NoInit) { }
    }

    //example of an extended packet. Always derived from packetheader
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    class ExamplePacket2 : PacketHeader
    {
        public int TheSkyIsBlue;
        public ExamplePacket2()
        {
            ExamplePacket2 pkt = new ExamplePacket2(1);
            Length = (short)System.Runtime.InteropServices.Marshal.SizeOf(pkt);
            Type = (int)PacketType.login;
        }
        private ExamplePacket2(int NoInit) { }
    }

}
