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
            DeviceStatusCache.OnDevicePluggedIn(UDID);
            JMF_MDM.UpdateDeviceEAValue(UDID, "YES", JAMF_EA.EA_PI_STATE); // device is in use
            JMF_MDM.UpdateDeviceEAValue(UDID, JMF_MDM.HubLocation, JAMF_EA.EA_Location); // device is plugged in into this hub
            JMF_MDM.UpdateDeviceEAValue(UDID, "", JAMF_EA.EA_User); // clear assigned user
        }

        public static void OnRFIDCardPresented(string RFID_UDID)
        {
            // is this card registered ?
            string cardHolder = RFIDCardManager.GetCardUser(RFID_UDID);
            if (cardHolder.Length == 0)
            {
                Console.WriteLine("Card " + RFID_UDID + " is not a registered card ");
                return;
            }
            Console.WriteLine(RFID_UDID);

            // get a plugged in device with highest battery life
            string bestDevice = DeviceStatusCache.GetHighestBatteryDevice();
            if (bestDevice.Length == 0)
            {
                Console.WriteLine("Card " + RFID_UDID + " can't unlock any devices as there is non plugged in ");
                return;
            }

            // let JAMF know that we assigned this device to this user
            JMF_MDM.UpdateDeviceEAValue(bestDevice, cardHolder, JAMF_EA.EA_User);

            // create a timer to reset the user back to unasigned after X seconds
            DeviceStatusCache.StartRFIDDeviceAutoUnlink(bestDevice);
        }

        public static void OnRFIDDeviceUnpairTimeout(string UDID)
        {
            // let JAMF know that we unassigned this device from this user
            JMF_MDM.UpdateDeviceEAValue(UDID, "", JAMF_EA.EA_User);
        }

        public static void OnDeviceUnPlugged(string UDID)
        {
            DeviceStatusCache.OnDeviceUnPlugged(UDID);
            JMF_MDM.UpdateDeviceEAValue(UDID, "NO", JAMF_EA.EA_PI_STATE);
            JMF_MDM.UpdateDeviceEAValue(UDID, "In Use", JAMF_EA.EA_Location);
        }
    }
}
