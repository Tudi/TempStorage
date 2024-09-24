using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FileExplorer
{
    public static class Plugable_DeviceManager
    {
        public static void OnDevicePluggedIn(string UDID)
        {
            Task.Run(() =>
            {
                LogWriter.WriteLog("Plugable_DeviceManager : clearing device " + UDID + " status on JAMF");
                
                DeviceStatusCache.OnDevicePluggedIn(UDID);

                List<(int JamfEAId, string NewValue)> eas = new List<(int, string)>
                {
                    ((int)JAMF_EA.EA_PI_STATE, "YES"), // device is not used by any user
                    ((int) JAMF_EA.EA_Location, JMF_MDM.HubLocation), // device is plugged in into this hub
                    ((int) JAMF_EA.EA_User, "") // clear assigned user
                };
                JMF_MDM.UpdateDeviceEAValue(UDID, eas); 
            });
        }

        public static void OnRFIDCardPresented(string RFID_UDID)
        {
            // is this card registered ?
            string cardHolder = RFIDCardManager.GetCardUser(RFID_UDID);
            if (cardHolder.Length == 0)
            {
                LogWriter.WriteLog("Card " + RFID_UDID + " is not a registered card ");
                return;
            }
            LogWriter.WriteLog(RFID_UDID + " was presented ");

            // get a plugged in device with highest battery life
            string bestDevice = DeviceStatusCache.GetHighestBatteryDevice();
            if (bestDevice.Length == 0)
            {
                LogWriter.WriteLog("Card " + RFID_UDID + " can't unlock any devices as there is non plugged in ");
                return;
            }

            // let JAMF know that we assigned this device to this user
            JMF_MDM.UpdateDeviceEAValue(bestDevice, JAMF_EA.EA_User, cardHolder);

            // create a timer to reset the user back to unasigned after X seconds
            DeviceStatusCache.StartRFIDDeviceAutoUnlink(bestDevice);
        }

        public static void OnRFIDDeviceUnpairTimeout(string UDID)
        {
            // let JAMF know that we unassigned this device from this user
            JMF_MDM.UpdateDeviceEAValue(UDID, JAMF_EA.EA_User, "");
        }

        public static void OnDeviceUnPlugged(string UDID)
        {
            Task.Run(() =>
            {
                LogWriter.WriteLog("Plugable_DeviceManager : device " + UDID + " status is set to unplugged on JAMF");
                DeviceStatusCache.OnDeviceUnPlugged(UDID);
                List<(int JamfEAId, string NewValue)> eas = new List<(int, string)>
                {
                    ((int)JAMF_EA.EA_PI_STATE, "NO"), // device is in use
                    ((int) JAMF_EA.EA_Location, "In Use") // device is not plugged in into this hub                    
                };
                JMF_MDM.UpdateDeviceEAValue(UDID, eas);
            });
        }
    }
}
