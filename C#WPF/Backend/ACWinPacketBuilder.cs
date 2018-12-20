using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLFClient.Backend
{
    public partial class ACWinConnectionManager
    {
        private unsafe void InitializeACWinConnection( )
        {
            byte [] buff = new byte[ConstantsACWin.MAX_BUFF + ConstantsACWin.MAX_BUFF];
            int BytesWritten = 4; //leave space to write the length as first field
            BytesWritten += NetworkPacketTools.StrToByteArray(ConstantsACWin.ACWIN_TYPE_REQ, buff, BytesWritten, false, false); //packet type
            BytesWritten += NetworkPacketTools.IntoToByteArray(1, buff, BytesWritten); // reference ID
            BytesWritten += NetworkPacketTools.StrToByteArray(ConstantsACWin.ACWIN_CMD_INITIALIZE, buff, BytesWritten, true, false); // command

            BytesWritten += NetworkPacketTools.StrToByteArray("SOCACWTDI", buff, BytesWritten, true, false); // app name
            BytesWritten += NetworkPacketTools.IntoToByteArray(2 * ConstantsACWin.AC_PASSWORD_LENGTH_SOCKET * ConstantsACWin.NUM_MAX_BYTES_PER_CHAR_UTF8, buff, BytesWritten); //size of the extra content
            BytesWritten += NetworkPacketTools.StrToByteArray("ACWTDI", buff, BytesWritten, false, false, ConstantsACWin.AC_PASSWORD_LENGTH_SOCKET * ConstantsACWin.NUM_MAX_BYTES_PER_CHAR_UTF8); // user name
            BytesWritten += NetworkPacketTools.StrToByteArray("oMf9eb", buff, BytesWritten, false, false, ConstantsACWin.AC_PASSWORD_LENGTH_SOCKET * ConstantsACWin.NUM_MAX_BYTES_PER_CHAR_UTF8); // user passw                                                                                                                                                                                                          //            BytesWritten += NetworkPacketTools.StrToByteArray("1.0", ci.m_protocol, BytesWritten, true, false); // this is only written as response and not as request

            NetworkPacketTools.IntoToByteArray(BytesWritten - 4, buff, 0); //update packet size

            SendPacket(buff, BytesWritten);
        }

        private unsafe void RegisterService()
        {
            /*
04.12.2018 11:52:20:966  ACWinMQ    Socket                    2   >00 00 00 23 52 45 51 00 00 00 03 00 00 00 08 52 <   ...#REQ........R   00000000
04.12.2018 11:52:20:966  ACWinMQ    Socket                    2   >65 67 69 73 74 65 72 00 00 00 0C 53 65 72 76 43 <   egister....ServC   00000010
04.12.2018 11:52:20:966  ACWinMQ    Socket                    2   >61 6C 6C 49 6E 66 6F <                              allInfo            00000020
             */
            byte[] buff = new byte[ConstantsACWin.MAX_BUFF + ConstantsACWin.MAX_BUFF];
            int BytesWritten = 4; //leave space to write the length as first field
            BytesWritten += NetworkPacketTools.StrToByteArray(ConstantsACWin.ACWIN_TYPE_REQ, buff, BytesWritten, false, false); //packet type
            BytesWritten += NetworkPacketTools.IntoToByteArray(3, buff, BytesWritten); // reference ID
            BytesWritten += NetworkPacketTools.StrToByteArray(ConstantsACWin.ACWIN_CMD_REGISTER, buff, BytesWritten, true, false); // command

            BytesWritten += NetworkPacketTools.StrToByteArray(ConstantsACWin.SERVICE_SERV_CALL_INFO, buff, BytesWritten, true, false); // service name

            NetworkPacketTools.IntoToByteArray(BytesWritten - 4, buff, 0); //update packet size
            SendPacket(buff, BytesWritten);
        }

        private unsafe void MakeCall(string Extension)
        {
            /*
             04.12.2018 11:53:17:331  ACWinMQ    Socket                    2   >00 00 00 21 52 45 51 00 00 00 02 00 00 00 0A 41 <   ...!REQ........A   00000000
            04.12.2018 11:53:17:331  ACWinMQ    Socket                    2   >43 4D 61 6B 65 43 61 6C 6C 00 00 00 04 31 31 30 <   CMakeCall....110   00000010
            04.12.2018 11:53:17:331  ACWinMQ    Socket                    2   >33 00 00 00 01 <                                    3....              00000020
            */
            byte[] buff = new byte[ConstantsACWin.MAX_BUFF + ConstantsACWin.MAX_BUFF];
            int BytesWritten = 4; //leave space to write the length as first field
            BytesWritten += NetworkPacketTools.StrToByteArray(ConstantsACWin.ACWIN_TYPE_REQ, buff, BytesWritten, false, false ); //packet type
            BytesWritten += NetworkPacketTools.IntoToByteArray(2, buff, BytesWritten); // reference ID
            BytesWritten += NetworkPacketTools.StrToByteArray(ConstantsACWin.SERVICE_AC_MAKE_CALL, buff, BytesWritten, true, false); // command

            BytesWritten += NetworkPacketTools.StrToByteArray(Extension, buff, BytesWritten, true, false); // the phone number we are going to call
            BytesWritten += NetworkPacketTools.IntoToByteArray(1, buff, BytesWritten); //call type

            NetworkPacketTools.IntoToByteArray(BytesWritten - 4, buff, 0); //update packet size

            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "ACWin: Sending MakeCall for extension : " + Extension.ToString());
            SendPacket(buff, BytesWritten);
        }

        private unsafe void CallInfoResponse()
        {
            /*
04.12.2018 11:53:23:268  ACWinMQ    Socket                    2   >00 00 00 1B 52 45 53 00 00 00 01 00 00 00 0C 53 <   ....RES........S   00000000
04.12.2018 11:53:23:268  ACWinMQ    Socket                    2   >65 72 76 43 61 6C 6C 49 6E 66 6F 00 00 00 00 <      ervCallInfo....    00000010
             */
            byte[] buff = new byte[ConstantsACWin.MAX_BUFF + ConstantsACWin.MAX_BUFF];
            int BytesWritten = 4; //leave space to write the length as first field
            BytesWritten += NetworkPacketTools.StrToByteArray(ConstantsACWin.ACWIN_TYPE_RES, buff, BytesWritten, true, false); //packet type
            BytesWritten += NetworkPacketTools.IntoToByteArray(1, buff, BytesWritten); // reference ID
            BytesWritten += NetworkPacketTools.StrToByteArray(ConstantsACWin.SERVICE_SERV_CALL_INFO, buff, BytesWritten, true, false); // command
            BytesWritten += NetworkPacketTools.IntoToByteArray(0, buff, BytesWritten); //

            NetworkPacketTools.IntoToByteArray(BytesWritten - 4, buff, 0); //update packet size
            SendPacket(buff, BytesWritten);
        }

    }
}
