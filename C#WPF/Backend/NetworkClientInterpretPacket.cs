using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLFClient.Backend
{
    enum SystemStatusCodes
    {
        SystemStatus_disabled = 4,
        SystemStatus_partiallyDisabled = 8,
        SystemStatus_enabled = 1,
        SystemStatus_initializing = 0,
        messagesLost = 3,
        SystemStatus_normal = 2,
        SystemStatus_overloadImminent = 5,
        SystemStatus_overloadReached = 6,
        SystemStatus_overloadRelieved = 7,
        //This id mark a dummy system status. if the sending of the event broken. the client already disconnected
        SystemStatus_Search_DeadClient = 9
    };

    class NetworkClientInterpretPacket
    {
        public static PhoneStatusCodes PhoneStatusCodeTranslate(long ServerSideState)
        {
            PhoneStatusCodes code = PhoneStatusCodes.NumberOfStatusCodes;
            if (ServerSideState == ServerSidePhoneStatusCodes.ZS_RUF)
                code = PhoneStatusCodes.Ringing;
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_GESPRAECH)
                code = PhoneStatusCodes.Busy;
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_RUHE)
                code = PhoneStatusCodes.Idle;
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_WAHLAUFNAHME)
            { }
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_BLOCKIERT
                    || ServerSideState == ServerSidePhoneStatusCodes.ZS_KONFERENZ
                    || ServerSideState == ServerSidePhoneStatusCodes.ZS_BESETZT)
            {
                // m_nState = (m_nState & PHONE_UMLEITUNG) | PHONE_BESETZT;
                code = PhoneStatusCodes.Busy;
            }
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_BLOCKIERT)
            {
                // m_nState = (m_nState & PHONE_UMLEITUNG) | PHONE_AUSSER_BETRIEB;
            }
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_WARTE)
            { }
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_DOESNOT)
                code = PhoneStatusCodes.PHONE_DOESNOT;
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_DOOS)
                code = PhoneStatusCodes.OutOfService;
            else if (ServerSideState == 0xF0FF)
            {
                //remove forwarding
            }
            else if (ServerSideState == 0x0FFF)
            {
                //remove forwarding to external
            }
            else if (ServerSideState == 11111)
            {
                // #define PHONE_RUF				0x0002//Ringing
            }
            else if (ServerSideState == 1111111)
            {
                // #define PHONE_UMLEITUNG			0x0100//Forward
            }
            else if (ServerSideState == 11111111)
            {
                // #define PHONE_EXTERNAL			0x1000//External call
            }
            else if(ServerSideState == ServerSidePhoneStatusCodes.ZS_EXTERNAL)
                code = PhoneStatusCodes.PHONE_EXTERNAL;

            return code;
        }
        public static PhoneStatusCodes PhoneStatusCodeTranslateMonitor(long ServerSideState)
        {
            PhoneStatusCodes code = PhoneStatusCodes.NumberOfStatusCodes;
            if (ServerSideState == ServerSidePhoneStatusCodes.ZS_RUHE)
                code = PhoneStatusCodes.Idle;
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_RUF)
                code = PhoneStatusCodes.Ringing;
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_GESPRAECH)
                code = PhoneStatusCodes.Busy;
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_BLOCKIERT )
                code = PhoneStatusCodes.Busy;
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_KONFERENZ)
                code = PhoneStatusCodes.Busy;
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_BESETZT)
            {
                // m_nState = (m_nState & PHONE_UMLEITUNG) | PHONE_BESETZT;
                code = PhoneStatusCodes.Busy;
            }
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_DOESNOT)
                code = PhoneStatusCodes.PHONE_DOESNOT;
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_DOOS)
                code = PhoneStatusCodes.OutOfService;
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_WAHLAUFNAHME)
            { }
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_BLOCKIERT)
            {
                // m_nState = (m_nState & PHONE_UMLEITUNG) | PHONE_AUSSER_BETRIEB;
            }
            else if (ServerSideState == ServerSidePhoneStatusCodes.ZS_WARTE)
            { }
            else if (ServerSideState == 0xF0FF)
            {
                //remove forwarding
            }
            else if (ServerSideState == 0x0FFF)
            {
                //remove forwarding to external
            }
            else if (ServerSideState == 11111)
            {
                // #define PHONE_RUF				0x0002//Ringing
            }
            else if (ServerSideState == 1111111)
            {
                // #define PHONE_UMLEITUNG			0x0100//Forward
            }
            else if (ServerSideState == 11111111)
            {
                // #define PHONE_EXTERNAL			0x1000//External call
            }
            return code;
        }

        public static void InterpretMessage(byte [] ReceivedBytes)
        {
            //check for packet integrity
            BLFWinHeader pkt = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<BLFWinHeader>(ReceivedBytes);

            //incomplete packet. Should never happen. TCPIP ...
            if(pkt.Length > ReceivedBytes.Length)
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Incomplete or unknown format packet received. Expecting " + pkt.Length.ToString() + " Got " + ReceivedBytes.Length.ToString());
                return;
            }

            // check if packet length is correct. Are there leftover bytes ?
            int ExpectedSize = ReceivedBytes.Length - System.Runtime.InteropServices.Marshal.SizeOf(typeof(BLFWinHeader));
            if (pkt.Length != ExpectedSize) // + the 0x55 from the end
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Incomplete or unknown format packet received. Expecting " + ExpectedSize.ToString() + " Got " + pkt.Length.ToString());
                return;
            }

            //check for packet delimiters
            if (pkt.BEG != 0xaa || ReceivedBytes[ReceivedBytes.Length - 1] != 0x55) 
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "Packet does not have correct marker");
                return;
            }

            byte[] ReceivedBytes2 = new byte[ReceivedBytes.Length-2];
            Array.Copy(ReceivedBytes, 1, ReceivedBytes2, 0, ReceivedBytes.Length - 2);
            switch ( pkt.Type )
            {
                case PacketTypes.ON_CONNECTED:
                    {
                        //update UI and say that we are online
                        if (App.Current != null && App.Current.MainWindow != null)
                            (App.Current.MainWindow as MainWindow).OnServerConnectionChanged(true);
                        Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "CBLF_w32Dlg::ConnectionOpened");
                    }
                    break;
                case PacketTypes.ON_DISCONNECTED:
                    {
                        //update UI and say that we are offline
                        if (App.Current != null && App.Current.MainWindow != null)
                            (App.Current.MainWindow as MainWindow).OnServerConnectionChanged(false);
                        Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "CBLF_w32Dlg::ConnectionClosed");
                    }
                    break;
                case PacketTypes.SYSTEMSTATUSSERVICEREQUEST:
                    {
                        SystemStatusServiceRequestStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<SystemStatusServiceRequestStr>(ReceivedBytes2);
                        SystemStatusCodes ssc = (SystemStatusCodes)(pkt2.Type);
                        if (ssc == SystemStatusCodes.SystemStatus_Search_DeadClient)
                        {
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : SYSTEMSTATUSSERVICEREQUEST System Status System_Status_DUMMY: " + pkt2.Type + " SystemStatus_Search_DeadClient ");
                        }
                        else if ((ssc == SystemStatusCodes.SystemStatus_normal || ssc == SystemStatusCodes.SystemStatus_initializing))
                        {
                            //Write the problem to the log
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : SYSTEMSTATUSSERVICEREQUEST System Status System_Status_OK: " + pkt2.Type + " SystemStatus_normal/SystemStatus_initializing ");

                            //notify connection manager that the server is up and running
                            Globals.ConnectionManager.OnHeartbeatReceived();
                            Globals.ExtensionManager.OnSystemStatusOk(); // we presume that all monitors function correctly and no need to panic for lack of updates

//                            m_bDisconnected_From_Callbridge = FALSE;

                            //Hide the server problem window
                            //Hide the server OK window
                            //Hide the Connection problem problem window
                            //Hide the Callbridge  OK window
                            //Hide the Callbridge problem window
                            //Start a timer , but this timer will not work if the connection is ok,because 
                            //we killing it before the running
                            //Starting a timer to reconnect if needed
                        }

                        else if (ssc == SystemStatusCodes.SystemStatus_disabled)
                        {
                            //Write the problem to the log
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : SYSTEMSTATUSSERVICEREQUEST System Status System_Status_Disabled: " + pkt2.Type );
                            //Open the switch problem dialog
                            //update status bar
                        }
                        else
                        {
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : SYSTEMSTATUSSERVICEREQUEST System Status:" + pkt2.Type);
                        }
                        //                        m_pWnd->SystemStatusService(pStruct->Type);
                        break;
                    }
                case PacketTypes.UNIVERSALERRORRESPONSE:
                    {
                        UniversalErrorResponseStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<UniversalErrorResponseStr>(ReceivedBytes2);
                        unsafe {
                            string device = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.Device);
                            string errorCategory = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.ErrorCategory);
                            string errorValue = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.ErrorValue);
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : UNIVERSALERRORRESPONSE : device=" + device + ", ErrCat=" + errorCategory + ", Err=" + errorValue);
                            //make sure we set the status of this device
                            if (errorCategory == "Operation" && errorValue == "invalidDeviceID")
                            {
                                Globals.ExtensionManager.OnStatusChange(device, PhoneStatusCodes.PHONE_DOESNOT);
                            }
                            else if (errorCategory == "System Resource Availability" && errorValue == "deviceOutOfService")
                            {
                                Globals.ExtensionManager.OnStatusChange(device, PhoneStatusCodes.OutOfService); 
                            }
                            //                            m_pWnd->UniversalErrorResponse(device, errorCategory, errorValue);
                        }
                        break;
                    }
                case PacketTypes.SETFEATUREFORWARDINGRESPONSE: //nothing to do here
                    {
                        Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : SETFEATUREFORWARDINGRESPONSE");
                        //?! missing in CSink
                        break;
                    }
                case PacketTypes.QUERYDEVICEFORWARDINGRESPONSE: //nothing to do here
                    {
                        Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : QUERYDEVICEFORWARDINGRESPONSE");
                        //?! missing in CSink
                        break;
                    }
                case PacketTypes.CONSULTATIONCALLRESPONSE:
                    {
                        ConsultationCallResponseStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<ConsultationCallResponseStr>(ReceivedBytes2);
                        unsafe
                        {
                            string device = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.Device);
                            string callId = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.CallID);
                            string deviceId = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.DeviceID);
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : CONSULTATIONCALLRESPONSE Device=" + device + " CallId=" + callId + " DeviceId=" + deviceId);

                            //                            m_pWnd->ConsultationCallResponse(device, callId, deviceId);
                            /// !!!!!!!!!!! not finished
                            if ( Globals.Config.GetConfig("Options", "Optiset","NO") == "YES")
                            {
                                string OptiSet = Globals.Config.GetConfig("Options", "OptisetExtension", "");
                                //get the extension only from a full phone number
                                string OptisetExtenstion = PhoneNumber.GetExtensionFromFullNumberStr(OptiSet);
                                long CallId = Globals.ExtensionManager.GetCallId(OptisetExtenstion);
                                if (OptisetExtenstion == device)
                                {
                                    NetworkClientBuildPacket.TransferCall(OptisetExtenstion, OptisetExtenstion, CallId.ToString(), callId);
                                }
                            }
                        }
                        break;
                    }
                case PacketTypes.TRANSFERCALLRESPONSE: //nothing to do here
                    {
                        TransferCallResponseStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<TransferCallResponseStr>(ReceivedBytes2);
                        unsafe
                        {
                            string device = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.Device);
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : TRANSFERCALLRESPONSE for device " + device);
//                            Globals.AppVars.m_bConsultationCall = false;
//                            Globals.AppVars.m_strDeviceIDTemp = "";
//                            Globals.AppVars.m_strCallIDTemp = "";				// BP-3.0.9.0-050816: Feature 'CallID as string': Modified: long m_lCallIDTemp -> CString m_strCallIDTemp
                            //                            m_pWnd->TransferCallResponse(device);
                        }
                        break;
                    }
                case PacketTypes.SINGLESTEPTRANSFERRESPONSE: //nothing to do here
                    {
                        SingleStepTransferResponseStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<SingleStepTransferResponseStr>(ReceivedBytes2);
                        unsafe
                        {
                            string device = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.Device);
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : SINGLESTEPTRANSFERRESPONSE " + device);
                            //                            m_pWnd->SingleStepTransferResponse(device);
                        }
                        break;
                    }
                case PacketTypes.MAKECALLRESPONSE: //nothing to do here
                    {
                        MakeCallResponseStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<MakeCallResponseStr>(ReceivedBytes2);
                        unsafe
                        {
                            string device = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.Device);
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : MAKECALLRESPONSE for device " + device);
                            //                            m_pWnd->MakeCallResponse(device);
                        }
                        break;
                    }
                case PacketTypes.UPDATEDEVICESTATE: //nothing to do here
                    {
                        UpdateDeviceStateStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<UpdateDeviceStateStr>(ReceivedBytes2);
                        unsafe
                        {
                            string device = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.Device);
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : UPDATEDEVICESTATE " + device + " " + pkt2.State.ToString());
                            PhoneStatusCodes code = PhoneStatusCodeTranslateMonitor(pkt2.State);
                            if (code < PhoneStatusCodes.NumberOfStatusCodes)
                                Globals.ExtensionManager.OnStatusChange(device, code);
                            //                            m_pWnd->UpdateDeviceState(device,pkt2.state);
                        }
                        break;
                    }
                case PacketTypes.ESTABLISHEDEVENT:
                    {
                        EstablishedEventStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<EstablishedEventStr>(ReceivedBytes2);
                        unsafe
                        {
                            string device = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.Device);
                            string callId = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.CallId);
                            string deviceId = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.DeviceId);
                            string answeringDevice = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.AnsweringDevice);
                            string callingDevice = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.CallingDevice);
                            string calledDevice = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.CalledDevice);
                            string lastRedirectionDevice = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.LastRedirectionDevice);
                            short isExternal = pkt2.shExternal; //OSFOURK-6659
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : ESTABLISHEDEVENT" + " Device=" + device + " CallId=" + callId + " DeviceId=" + deviceId + " AnswerDevice=" + answeringDevice + " CallingDevice=" + callingDevice + " CalledDevice=" + calledDevice + " LastRedirectionDevice=" + lastRedirectionDevice + " IsExternal=" + isExternal.ToString());
                            Globals.ExtensionManager.OnCallIdReceived(callingDevice, callId, calledDevice);
                            //m_pWnd->EstablishedEvent(device, callId, deviceId, answeringDevice, callingDevice, calledDevice, lastRedirectionDevice, isExternal);
/*                            if (device == Globals.Config.GetConfig("Options", "OptisetExtension", ""))
                            {
                                Globals.AppVars.m_strDeviceIDTemp = deviceId;
                                Globals.AppVars.m_strCallIDTemp = callId;
                            }
                            if((device == Globals.Config.GetConfig("Options", "OptisetExtension", "") || device == Globals.ExtensionManager.GetLastSelectedExtension().ToString()) && Globals.AppVars.m_bConsultationCall == true)
                            {
                                Globals.AppVars.m_bConsultationCall = false;
                                Globals.AppVars.m_strDeviceIDTemp = "";
                                Globals.AppVars.m_strCallIDTemp = "";               // BP-3.0.9.0-050816: Feature 'CallID as string': Modified: long m_lCallIDTemp -> CString m_strCallIDTemp
                            }*/
                            if(isExternal == ServerSidePhoneStatusCodes.ZS_EXTERNVERBINDUNG)
                                Globals.ExtensionManager.OnStatusChange(device, PhoneStatusCodeTranslate(ServerSidePhoneStatusCodes.ZS_EXTERNAL));
                            else
                                Globals.ExtensionManager.OnStatusChange(device, PhoneStatusCodeTranslate(ServerSidePhoneStatusCodes.ZS_GESPRAECH));
                        }
                        break;
                    }
                case PacketTypes.CONNECTIONCLEAREDEVENT:
                    {
                        ConnectionClearedEventStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<ConnectionClearedEventStr>(ReceivedBytes2);
                        unsafe
                        {
                            string device = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.Device);
                            string droppedCall = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.DroppedCall);
                            string droppedDevice = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.DroppedDevice);
                            string releasingDevice = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.ReleasingDevice);
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : CONNECTIONCLEAREDEVENT Device=" + device + " CallId=" + droppedCall + " DroppedDevice=" + droppedDevice + " ReleasingDevice=" + releasingDevice + " State=" + pkt2.shLocalConnectionState.ToString() + " Cause=" + pkt2.shEventCause.ToString());
                            Globals.ExtensionManager.OnCallIdClear(device, droppedCall, droppedDevice);
                            //m_pWnd->ConnectionClearedEvent(device, droppedCall, droppedDevice, releasingDevice, pStruct->shLocalConnectionState, pStruct->shEventCause);
                            PhoneStatusCodes code = PhoneStatusCodeTranslateMonitor(pkt2.shLocalConnectionState);
                            if (code < PhoneStatusCodes.NumberOfStatusCodes )
                                Globals.ExtensionManager.OnStatusChange(device, code);
                            else
                                Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "CONNECTIONCLEAREDEVENT : Ignore handling status update as not recognized state");
                        }
                        break;
                    }
                case PacketTypes.UPDATEFORWARDSTATESTR:
                    {
                        UpdateForwardStateStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<UpdateForwardStateStr>(ReceivedBytes2);
                        unsafe
                        {
                            string device = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.Device);
                            string pvtFwdDn = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.pvt_fwd_dn);
                            string pvtType = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.pvt_type);

                            string Exntension = device;
                            long Fwd = PhoneNumberManager.Int32Parse(pvtFwdDn, 0);

                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : UPDATEFORWARDSTATESTR, device " + device + " forward "+ pvtFwdDn + " fwd type "+ pvtType);

                            if (Fwd != 0)
                                Globals.ForwardManager.PhoneNumberUpdateForwarding(Exntension, CallForwardingTypes.CallForwardDestination, 0, Fwd);
                            else
                                Globals.ForwardManager.PhoneNumberUpdateForwarding(Exntension, CallForwardingTypes.CallForwardNone, 0, Fwd);
                        }
                        break;
                    }
                case PacketTypes.NETWORKREACHEDEVENT:
                    {
                        Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : NETWORKREACHEDEVENT");
                        NetworkReachedEventStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<NetworkReachedEventStr>(ReceivedBytes2);
                        unsafe
                        {
                            string device = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.Device);
                            string calledDeviceId = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.called_device_id);
                            //m_pWnd->NetworkReached(device, calledDeviceId);
                            //folders[i]->UpdateCell(_T((char*)bstrDevice), ZS_EXTERNAL, dc);
                            Globals.ExtensionManager.OnStatusChange(device, PhoneStatusCodeTranslate(ServerSidePhoneStatusCodes.ZS_EXTERNAL));
                        }
                        break;
                    }
                case PacketTypes.MONITORSTARTRESPONSE: //nothing to do here
                    {
                        MonitorStartResponseStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<MonitorStartResponseStr>(ReceivedBytes2);
                        unsafe
                        {
                            string device = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.Device);
                            string xRef = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.Xref);
                            Globals.ExtensionManager.OnMonitorStart(device, xRef);
                            //m_pWnd->MonitorStartResult(device, xRef);
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : MONITORSTARTRESPONSE , device " + device + ", xref " + xRef);
                        }
                        break;
                    }
                case PacketTypes.MONITORSTOPRESPONSE: //nothing to do here
                    {
                        MonitorStopResponseStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<MonitorStopResponseStr>(ReceivedBytes2);
                        unsafe
                        {
                            string device = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.Device);
                            Globals.ExtensionManager.OnMonitorStop(device);
                            //m_pWnd->MonitorStopResult();
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : MONITORSTOPRESPONSE, device " + device);
                        }
                        break;
                    }
                case PacketTypes.SNAPSHOTDEVICERESPONSE:
                    {

//                        Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : SNAPSHOTDEVICERESPONSE");
                        SnapshotDeviceResponseStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<SnapshotDeviceResponseStr>(ReceivedBytes2);
                        unsafe
                        {
                            string device = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.Device);
                            //m_pWnd->SnapshotDeviceResult(device, pStruct->State);

                            PhoneStatusCodes code = PhoneStatusCodeTranslate(pkt2.State);
                            if (code < PhoneStatusCodes.NumberOfStatusCodes)
                            {
                                Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "SNAPSHOTDEVICERESPONSE : device '" + device + "', state '" + pkt2.State + "'");
                                Globals.ExtensionManager.OnStatusChange(device, code);
                            }
                            else
                                Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "SNAPSHOTDEVICERESPONSE : !Ignored! : device '" + device + "', state '" + pkt2.State + "'");
                        }
                        break;
                    }
                case PacketTypes.MONITORSTOPREQUEST:
                    {
                        Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : MONITORSTOPREQUEST");
                        MonitorStopRequestStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<MonitorStopRequestStr>(ReceivedBytes2);
                        unsafe
                        {
                            string xrefId = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.XREFId);
                            //m_pWnd->MonitorStopRequest(xrefId);
                            Globals.ExtensionManager.OnMonitorStop(xrefId);
                        }
                        break;
                    }
                case PacketTypes.DIVERTEDEVENT:
                    {
                        Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : DIVERTEDEVENT");
                        MonitorStopRequestStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<MonitorStopRequestStr>(ReceivedBytes2);
                        unsafe
                        {
                            string xrefId = NetworkPacketTools.BytesToString_MAX_STN_CHARNO(pkt2.XREFId);
                            PhoneNumber pn = Globals.ExtensionManager.PhoneNumberGetByXRef(PhoneNumberManager.Int32Parse(xrefId, 0));
                            if (pn != null)
                            {
                                if (pn.GetStatus() == PhoneStatusCodes.Ringing)
                                {
                                    if (App.Current != null)
                                        App.Current.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)(() =>
                                        {
                                            MainWindow MainObject = (MainWindow)App.Current.MainWindow;

                                            if (MainObject == null)
                                                return;
                                            pn.SetStatus(PhoneStatusCodes.Idle);
                                        }));
                                }
                            }
                            else
                            {
                                // create a new store and set the state as idle
                                // this is not required for us (i think). We will query the state as soon as we have a store for it
                            }
                            //m_pWnd->DivertedEvent(xrefId);
                        }
                        break;
                    }
                case PacketTypes.DBSENDCHK:
                    {
                        //OSFOURK-6654
                        Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : DBSENDCHK");

                        DbSendChkStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<DbSendChkStr>(ReceivedBytes2);
                        ushort checksum = pkt2.Xchk;
                        ushort ClientChecksum = Globals.persPortManager.GetChecksum();
                        //m_pWnd->ValidateChksum(checksum);
                        if (ClientChecksum != checksum)
                        {
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "Checksum mismatch, will request a new persport");
                            NetworkClientBuildPacket.DBRequest((ushort)0xffff);
                        }
                        else
                        {
                            Globals.persPortManager.ReParsePersportTXT();
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Checksum says persport is up to date");
                        }
                        break;
                    }
                case PacketTypes.DBCHUNK:
                    {
                        Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : DBCHUNK");
                        DbChunkHdrStr pkt2 = BLFClient.Backend.NetworkPacketTools.ByteArrayToStructure<DbChunkHdrStr>(ReceivedBytes2);
                        ushort maxChunk = pkt2.MaxChunk;
                        ushort chunk = pkt2.Chunk;
                        int HeaderSize = System.Runtime.InteropServices.Marshal.SizeOf(typeof(DbChunkHdrStr));
                        int chunkLen = pkt2.Size - HeaderSize + 2; // no idea from where that extra 2 comes from. As long as it works ...
                        //save the chunk
                        Globals.persPortManager.SaveChunkToFile(chunk == 0, ReceivedBytes2, HeaderSize, chunkLen);
                        //request the next chunk to be saved
                        if (chunk < maxChunk)
                            NetworkClientBuildPacket.DBRequest(chunk);
                        else
                        {
                            Globals.persPortManager.ReParsePersportTXT();
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Fetched all " + maxChunk.ToString() + " chunks of passport.txt");
                        }
                        break;
                    }
                default:
                {
                        // unhandled packet type
                        Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "recv msg type : Unknown and unhandled, type " + pkt.Type.ToString() + " length " + pkt.Length.ToString());
                }
                break;
            };
        }
    }
}
