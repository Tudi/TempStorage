using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLFClient.Backend
{
    public partial class ACWinConnectionManager
    {

        private unsafe bool ParseResponseInitialize(byte[] packet)
        {
            /*
04.12.2018 11:52:20:965  ACWinMQ    Socket                    2   >00 00 00 27 52 45 53 00 00 00 01 00 00 00 0A 49 <   ...'RES........I   00000000
04.12.2018 11:52:20:965  ACWinMQ    Socket                    2   >6E 69 74 69 61 6C 69 7A 65 00 00 00 0A 56 32 20 <   nitialize....V2    00000010
04.12.2018 11:52:20:965  ACWinMQ    Socket                    2   >52 34 2E 31 30 2E 30 00 00 00 00 <                  R4.10.0....        00000020
             */
            int BytesRead = 0;
            // read 4 bytes : size of packet
            if (BytesRead + 4 >= packet.Length)
                return false;
            //          int PacketSize = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;

            // read 3 bytes : "RES"
            if (BytesRead + 3 >= packet.Length)
                return false;
            string ResStr = NetworkPacketTools.ByteArrayToStr(packet, BytesRead, 3);
            if (ResStr != ConstantsACWin.ACWIN_TYPE_RES)
                return false;
            BytesRead += 3;

            // read 4 bytes : reference id
            if (BytesRead + 4 >= packet.Length)
                return false;
            //            int ReferenceId = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;

            // read 4 bytes : size of string
            if (BytesRead + 4 >= packet.Length)
                return false;
            int InitializeStrSize = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;

            // read X bytes : string
            if (BytesRead + InitializeStrSize >= packet.Length)
                return false;
            string InitializeStr = NetworkPacketTools.ByteArrayToStr(packet, BytesRead, InitializeStrSize);
            if (InitializeStr != ConstantsACWin.ACWIN_CMD_INITIALIZE)
                return false;
            BytesRead += InitializeStrSize;

            // read 4 bytes : size of string
            if (BytesRead + 4 >= packet.Length)
                return false;
            int VersionStrSize = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;

            // read X bytes : string
            if (BytesRead + VersionStrSize > packet.Length)
                return false;
            //            string VersionStr = NetworkPacketTools.ByteArrayToStr(packet, BytesRead, VersionStrSize);
            BytesRead += VersionStrSize;

            return true;
        }

        private unsafe bool ParseResponseError(byte[] packet)
        {
            /*
04.12.2018 18:05:37:568  ACWinMQ    Socket                    2   >00 00 00 14 45 52 52 00 00 00 01 00 00 00 05 45 <   ....ERR........E   00000000
04.12.2018 18:05:37:568  ACWinMQ    Socket                    2   >72 72 6F 72 00 00 00 00 <                           rror....           00000010
             */
            int BytesRead = 0;
            // read 4 bytes : size of packet
            if (BytesRead + 4 >= packet.Length)
                return false;
            //          int PacketSize = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;

            // read 3 bytes : "ERR"
            if (BytesRead + 3 >= packet.Length)
                return false;
            string ResStr = NetworkPacketTools.ByteArrayToStr(packet, BytesRead, 3);
            if (ResStr != ConstantsACWin.ACWIN_TYPE_ERR)
                return false;
            BytesRead += 3;

            // read 4 bytes : reference id
            if (BytesRead + 4 >= packet.Length)
                return false;
            //            int ReferenceId = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;

            // read 4 bytes : size of string
            if (BytesRead + 4 >= packet.Length)
                return false;
            int ErrorStrSize = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;

            // read X bytes : string
            if (BytesRead + ErrorStrSize >= packet.Length)
                return false;
            string ErrorStr = NetworkPacketTools.ByteArrayToStr(packet, BytesRead, ErrorStrSize);
            if (ErrorStr != ConstantsACWin.ACWIN_CMD_GENERAL_ERROR)
                return false;
            BytesRead += ErrorStrSize;

            // read 4 bytes : 
            if (BytesRead + 4 >= packet.Length)
                return false;
            int SomeValue = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;

            return true;
        }

        private unsafe bool ParseResponseRegister(byte[] packet)
        {
            /*
04.12.2018 11:52:20:967  ACWinMQ    Socket                    2   >00 00 00 17 52 45 53 00 00 00 03 00 00 00 08 52 <   ....RES........R   00000000
04.12.2018 11:52:20:967  ACWinMQ    Socket                    2   >65 67 69 73 74 65 72 00 00 00 01 <                  egister....        00000010
             */
            int BytesRead = 0;
            // read 4 bytes : size of packet
            if (BytesRead + 4 >= packet.Length)
                return false;
            int PacketSize = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;

            // read 3 bytes : "RES"
            if (BytesRead + 3 >= packet.Length)
                return false;
            string ResStr = NetworkPacketTools.ByteArrayToStr(packet, BytesRead, 3);
            if (ResStr != ConstantsACWin.ACWIN_TYPE_RES)
                return false;
            BytesRead += 3;

            // read 4 bytes : reference id
            if (BytesRead + 4 >= packet.Length)
                return false;
            BytesRead += 4;

            // read 4 bytes : size of string
            if (BytesRead + 4 >= packet.Length)
                return false;
            int RegisterStrSize = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;

            // read X bytes : string
            if (BytesRead + RegisterStrSize >= packet.Length)
                return false;
            string RegisterStr = NetworkPacketTools.ByteArrayToStr(packet, BytesRead, RegisterStrSize);
            if (RegisterStr != ConstantsACWin.ACWIN_CMD_REGISTER)
                return false;
            BytesRead += RegisterStrSize;

            // read 4 bytes : size of string
            if (BytesRead + 4 > packet.Length)
                return false;
            int ServiceType = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;

            return true;
        }

        private unsafe bool ParseResponseMakeCall(byte[] packet)
        {
            /*
04.12.2018 11:53:17:511  ACWinMQ    Socket                    2   >00 00 00 19 52 45 53 00 00 00 02 00 00 00 0A 41 <   ....RES........A   00000000
04.12.2018 11:53:17:511  ACWinMQ    Socket                    2   >43 4D 61 6B 65 43 61 6C 6C 00 00 00 00 <            CMakeCall....      00000010
             */
            int BytesRead = 0;
            // read 4 bytes : size of packet
            if (BytesRead + 4 >= packet.Length)
                return false;
            int PacketSize = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;

            // read 3 bytes : "RES"
            if (BytesRead + 3 >= packet.Length)
                return false;
            string ResStr = NetworkPacketTools.ByteArrayToStr(packet, BytesRead, 3);
            if (ResStr != ConstantsACWin.ACWIN_TYPE_RES)
                return false;
            BytesRead += 3;

            // read 4 bytes : reference id
            if (BytesRead + 4 >= packet.Length)
                return false;
            BytesRead += 4;
            if (BytesRead >= packet.Length)
                return false;

            // read 4 bytes : size of string
            if (BytesRead + 4 >= packet.Length)
                return false;
            int RegisterStrSize = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;
            if (BytesRead >= packet.Length)
                return false;

            // read X bytes : string
            if (BytesRead + RegisterStrSize >= packet.Length)
                return false;
            string RegisterStr = NetworkPacketTools.ByteArrayToStr(packet, BytesRead, RegisterStrSize);
            if (RegisterStr != ConstantsACWin.SERVICE_AC_MAKE_CALL)
                return false;
            BytesRead += RegisterStrSize;

            // read 4 bytes : size of string
            if (BytesRead + 4 > packet.Length)
                return false;
            int StatusType = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;

            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "ACWin: Received MakeCall reply code : " + StatusType.ToString());
            return true;
        }

        private bool ParseCallInfo(byte[] packet)
        {
            /*
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 09 9F 52 45 51 00 00 00 01 00 00 00 0C 53 <   ....REQ........S   00000000
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >65 72 76 43 61 6C 6C 49 6E 66 6F 00 00 09 88 31 <   ervCallInfo....1   00000010
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >31 30 33 00 00 00 00 00 00 00 00 00 00 00 00 00 <   103.............   00000020
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000030
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000040
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000050
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000060
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000070
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000080
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000090
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000000A0
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000000B0
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000000C0
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000000D0
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000000E0
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000000F0
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000100
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000110
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000120
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000130
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000140
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000150
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000160
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000170
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000180
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000190
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000001A0
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000001B0
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000001C0
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000001D0
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000001E0
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000001F0
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000200
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000210
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000220
            04.12.2018 11:53:23:264  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000230
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000240
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000250
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000260
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000270
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000280
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000290
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000002A0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000002B0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000002C0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000002D0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000002E0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000002F0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000300
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000310
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000320
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 50 <   ...............P   00000330
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >69 73 74 61 20 42 61 63 73 69 00 00 00 00 00 00 <   ista Bacsi......   00000340
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000350
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000360
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000370
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000380
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000390
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000003A0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000003B0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000003C0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000003D0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000003E0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000003F0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000400
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000410
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000420
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000430
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000440
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000450
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000460
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000470
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000480
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000490
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000004A0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000004B0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000004C0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000004D0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000004E0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000004F0
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000500
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000510
            04.12.2018 11:53:23:265  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000520
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000530
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000540
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000550
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000560
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000570
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000580
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000590
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000005A0
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000005B0
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000005C0
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000005D0
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000005E0
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000005F0
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000600
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000610
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000620
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000630
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000640
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000650
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000660
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000670
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000680
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000690
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000006A0
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000006B0
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000006C0
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000006D0
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000006E0
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000006F0
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000700
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000710
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000720
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000730
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000740
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000750
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000760
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000770
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000780
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000790
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000007A0
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000007B0
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000007C0
            04.12.2018 11:53:23:266  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000007D0
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000007E0
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000007F0
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000800
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000810
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000820
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000830
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000840
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000850
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000860
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000870
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000880
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000890
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000008A0
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000008B0
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000008C0
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000008D0
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000008E0
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   000008F0
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000900
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000910
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000920
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000930
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000940
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000950
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000960
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000970
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 01 00 00 00 0D 00 00 00 00 00 00 00 02 00 <   ................   00000980
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 05 00 00 00 00 00 00 00 00 00 00 00 00 00 <   ................   00000990
            04.12.2018 11:53:23:267  ACWinMQ    Socket                    2   >00 00 00 <                                          ...                000009A0
             */
            int BytesRead = 0;
            // read 4 bytes : size of packet
            if (BytesRead + 4 >= packet.Length)
                return false;
            //          int PacketSize = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;

            // read 3 bytes : "RES"
            if (BytesRead + 3 >= packet.Length)
                return false;
            string ResStr = NetworkPacketTools.ByteArrayToStr(packet, BytesRead, 3);
            if (ResStr != ConstantsACWin.ACWIN_TYPE_REQ)
                return false;
            BytesRead += 3;

            // read 4 bytes : reference id
            if (BytesRead + 4 >= packet.Length)
                return false;
            //            int ReferenceId = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;

            // read 4 bytes : size of string
            if (BytesRead + 4 >= packet.Length)
                return false;
            int ServerCallInfoStrSize = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;

            // read X bytes : string
            if (BytesRead + ServerCallInfoStrSize >= packet.Length)
                return false;
            string ServerCallInfoStr = NetworkPacketTools.ByteArrayToStr(packet, BytesRead, ServerCallInfoStrSize);
            if (ServerCallInfoStr != ConstantsACWin.SERVICE_SERV_CALL_INFO)
                return false;
            BytesRead += ServerCallInfoStrSize;

            // read 4 bytes : size of rest of the packet
            if (BytesRead + 4 >= packet.Length)
                return false;
            int PacketSize2 = NetworkPacketTools.ByteArrayToInt(packet, BytesRead);
            BytesRead += 4;

            // read X bytes : extension string
            // .....................

            return true;
        }

    }
}
