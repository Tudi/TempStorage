using FileExplorer;
using System;
using System.Threading;
using System.Windows.Forms;

namespace USBDeviceInfoAPI
{
    static class Program
    {
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            // must come after loading ini
            RFIDCardManager.LoadCardDetails();
            // create a keyboard reader
            RFIDReader_keyboard rFIDReader_Keyboard = new RFIDReader_keyboard();
            // now that the keyboard handler is set up, start reading keys
            KeyboardHookManager.Start();
            // start the device cache manager
            DeviceStatusCache.StartPeriodicPoll();

            // init the JMF class to hold a bearer token
            JMF_MDM.GetBearerToken();
//            JMF_MDM.GetMobileDeviceData("00008130-000125563E90001C");
//            JMF_MDM.GetMobileDeviceExtensionAttributes(1);
//            JMF_MDM.GetMobileDeviceExtensionAttributes2("00008130-000125563E90001C");
//            JMF_MDM.UpdateDeviceExtensionVariableValue("1", "1");
//            JMF_MDM.UpdateDeviceEAValue("00008130-000125563E90001C", "YES", JAMF_EA.EA_PI_STATE);
//            JMF_MDM.UpdateDeviceEAValue("00008130-000125563E90001C", JMF_MDM.HubLocation, JAMF_EA.EA_Location);
/*            {
                Plugable_DeviceManager.OnDevicePluggedIn("00008120-0009395A3C62201E");
                Thread.Sleep(2000);
                Plugable_DeviceManager.OnRFIDCardPresented("0008406925");
                Thread.Sleep(6000);
                Plugable_DeviceManager.OnDeviceUnPlugged("00008120-0009395A3C62201E");
            }/**/

            Application.Run(new Form1());

            // stop device status poll
            // stop keyboard hook
            // unload ini
        }
    }
}
