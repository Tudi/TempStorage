using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;
using System.Timers;

namespace FileExplorer
{
    public static class DeviceStatusCache
    {
        public class DeviceInfo
        {
            public string UDID;
            public float batteryLevel;
            public long pluginStamp;
            public long unplugStamp;
            public long nextRefreshStamp;
            public bool isPluggedIn;
            public string JAMFResponse;
            public System.Timers.Timer resetTimerLinkState = null;
        }

        private static Dictionary<string, DeviceInfo> _pluggedInDevices;
        private static System.Windows.Forms.Timer timer = null;

        static DeviceStatusCache()
        {
            _pluggedInDevices = new Dictionary<string, DeviceInfo>();
        }

        public static void OnDevicePluggedIn(string udid)
        {
            if (_pluggedInDevices.ContainsKey(udid) == false)
            {
                _pluggedInDevices[udid] = new DeviceInfo();
                _pluggedInDevices[udid].UDID = udid;
                _pluggedInDevices[udid].nextRefreshStamp = 0;
                _pluggedInDevices[udid].batteryLevel = 0;
            }
            _pluggedInDevices[udid].pluginStamp = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds();
            _pluggedInDevices[udid].isPluggedIn = true;

            // does not mean we need to refresh it. Pushing safety logic inside the function
            RefreshDeviceInfo(udid);
        }

        public static void OnDeviceUnPlugged(string udid)
        {
            if (_pluggedInDevices.ContainsKey(udid) == true)
            {
                _pluggedInDevices[udid].isPluggedIn = false;
                _pluggedInDevices[udid].unplugStamp = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds();
            }
        }

        public static void StartPeriodicPoll()
        {
            if (timer == null)
            {
                timer = new System.Windows.Forms.Timer();
                timer.Interval = Int32.Parse(ConfigReader.GetConfigValue("JAMFDeviceStatusRefreshInterval"));
                timer.Tick += new EventHandler(Timer_Tick);
                timer.Start();
            }
        }

        private static void RefreshDeviceInfo(string udid)
        {
            // invalid device refresh. Should never happen
            if (_pluggedInDevices.ContainsKey(udid) == false)
            {
                return;
            }
            // device got unplugged. No need to refresh
            if (_pluggedInDevices[udid].isPluggedIn == false)
            {
                return;
            }
            // recently refreshed
            long tickNow = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds();
            if(tickNow < _pluggedInDevices[udid].nextRefreshStamp)
            {
                return;
            }
            // mark it refreshed
            _pluggedInDevices[udid].nextRefreshStamp = tickNow + Int32.Parse(ConfigReader.GetConfigValue("JAMFDeviceStatusRefreshInterval"));
            // get new info from JAMF
            string device_data = JMF_MDM.GetMobileDeviceData(udid);
            if (device_data.Length > 0)
            {
                _pluggedInDevices[udid].JAMFResponse = device_data;
            }
            // parse JAMF to get baterry level
            JObject jsonObject = JObject.Parse(device_data);
            int batteryLevel = (int)jsonObject["mobile_device"]["general"]["battery_level"];
            _pluggedInDevices[udid].batteryLevel = batteryLevel;
        }

        static public string GetHighestBatteryDevice()
        {
            string ret = "";
            float bestBattery = 0;
            foreach (DeviceInfo device in _pluggedInDevices.Values)
            {
                if(device.batteryLevel > bestBattery)
                {
                    bestBattery = device.batteryLevel;
                    ret = device.UDID;
                }
            }
            return ret;
        }

        private static void Timer_Tick(object sender, EventArgs e)
        {
            foreach (DeviceInfo device in _pluggedInDevices.Values)
            {
                RefreshDeviceInfo(device.UDID);
            }
        }

        public static void StartRFIDDeviceAutoUnlink(string udid)
        {
            // invalid device refresh. Should never happen
            if (_pluggedInDevices.ContainsKey(udid) == false)
            {
                return;
            }
            int statusResetTimeout = Int32.Parse(ConfigReader.GetConfigValue("RFIDAutoUnpairTimeout"));
            if(statusResetTimeout == 0)
            {
                return;
            }
            if (_pluggedInDevices[udid].resetTimerLinkState == null)
            {
                _pluggedInDevices[udid].resetTimerLinkState = new System.Timers.Timer(statusResetTimeout);
                _pluggedInDevices[udid].resetTimerLinkState.Elapsed += (sender, e) => OnResetLinkTimedEvent(udid);
                _pluggedInDevices[udid].resetTimerLinkState.AutoReset = false; // Make sure the timer runs only once
                _pluggedInDevices[udid].resetTimerLinkState.Start();
            }
        }

        private static void OnResetLinkTimedEvent(string udid)
        {
            if (_pluggedInDevices.ContainsKey(udid) == false)
            {
                return;
            }

            // Dispose the timer
            _pluggedInDevices[udid].resetTimerLinkState.Dispose();

            Plugable_DeviceManager.OnRFIDDeviceUnpairTimeout(udid);
        }
    }
}
