using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime;
using System.Runtime.CompilerServices;
using System.Security.Policy;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Timers;

namespace FileExplorer
{
    public static class DeviceStatusCache
    {
        public class EAInfo
        {
            public string name;
            public string value;
            public long updateStamp;
        }

        public class DeviceInfo
        {
            public string UDID;
            public float batteryLevel;
            public long pluginStamp;
            public long unplugStamp;
            public long nextRefreshStamp;
            public bool isPluggedIn;
            public string JAMFResponse;
            public Dictionary<string, EAInfo> JAMF_EA_Remote_Values;
            public Dictionary<string, EAInfo> JAMF_EA_Local_Values;
            public System.Timers.Timer resetTimerLinkState = null;
        }

        private static Dictionary<string, DeviceInfo> _pluggedInDevices;
        private static System.Timers.Timer timer = null;

        static DeviceStatusCache()
        {
            _pluggedInDevices = new Dictionary<string, DeviceInfo>();
        }

        public static void OnDevicePluggedIn(string udid)
        {
            if (_pluggedInDevices.ContainsKey(udid) == false)
            {
                LogWriter.WriteLog("DeviceStatusCache : first time seen device : " + udid );
                _pluggedInDevices[udid] = new DeviceInfo();
                _pluggedInDevices[udid].UDID = udid;
                _pluggedInDevices[udid].nextRefreshStamp = 0;
                _pluggedInDevices[udid].batteryLevel = 0;
                _pluggedInDevices[udid].JAMF_EA_Local_Values = new Dictionary<string, EAInfo>();
                _pluggedInDevices[udid].JAMF_EA_Remote_Values = new Dictionary<string, EAInfo>();
            }
            else
            {
                LogWriter.WriteLog("DeviceStatusCache : device plugged back into hub: " + udid);
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
                LogWriter.WriteLog("DeviceStatusCache : device unplugged : " + udid);
                _pluggedInDevices[udid].isPluggedIn = false;
                _pluggedInDevices[udid].unplugStamp = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds();
            }
        }

        public static void StartPeriodicPoll()
        {
            if (timer == null)
            {
                timer = new System.Timers.Timer(ConfigReader.GetConfigValueInt("JAMFDeviceStatusRefreshInterval", 1000));
                timer.Elapsed += Timer_Tick;
                timer.AutoReset = true;
                timer.Enabled = true;
                timer.Start();
            }
        }

        private static void RefreshDeviceInfo(string udid)
        {
            Task.Run(() =>
            {
                LogWriter.WriteLog("DeviceStatusCache : refreshing device info : " + udid);
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
                long PrevTimestamp = _pluggedInDevices[udid].nextRefreshStamp;
                // mark it refreshed
                _pluggedInDevices[udid].nextRefreshStamp = tickNow + ConfigReader.GetConfigValueInt("JAMFDeviceStatusRefreshInterval",1000);
                // get new info from JAMF
                string device_data = JMF_MDM.GetMobileDeviceData(udid);
                if (device_data.Length > 0)
                {
                    _pluggedInDevices[udid].JAMFResponse = device_data;
                    // parse JAMF to get baterry level
                    JObject jsonObject = JObject.Parse(device_data);

                    int batteryLevel = (int)jsonObject["mobile_device"]["general"]["battery_level"];
                    _pluggedInDevices[udid].batteryLevel = batteryLevel;

                    // parse extension attributes to see if we are in sync with JAMF
                    var extensionAttributes = jsonObject["mobile_device"]["extension_attributes"];

                    // Iterate over each attribute and print name=value
                    foreach (var attribute in extensionAttributes)
                    {
                        string name = attribute["name"].ToString();
                        string value = attribute["value"].ToString();
                        Set_EA_Value(false, udid, name, value);
                    }
                }
                else
                {
                    LogWriter.WriteLog("DeviceStatusCache : failed to refreshing device info : " + udid);
                    // refresh failed. Maybe simple http timeout. retry next time
                    _pluggedInDevices[udid].nextRefreshStamp = PrevTimestamp;
                }
            });
        }

        static public string GetHighestBatteryDevice()
        {
            string ret = "";
            float bestBattery = 0;
            foreach (DeviceInfo device in _pluggedInDevices.Values)
            {
                if(device.batteryLevel >= bestBattery && device.isPluggedIn == true)
                {
                    bestBattery = device.batteryLevel;
                    ret = device.UDID;
                }
            }

            LogWriter.WriteLog("DeviceStatusCache : highest battery device UUID = " + ret + " with baterry level = " + bestBattery.ToString());

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
            int statusResetTimeout = ConfigReader.GetConfigValueInt("RFIDAutoUnpairTimeout", 1000);
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

            // the device has been picked up from the charger
            if(_pluggedInDevices[udid].isPluggedIn == false)
            {
                return;
            }

            LogWriter.WriteLog("DeviceStatusCache : device was unlocked with card, but not picked up from hub. Relocking : " + udid);

            // Dispose the timer
            _pluggedInDevices[udid].resetTimerLinkState.Dispose();
            _pluggedInDevices[udid].resetTimerLinkState = null;

            Plugable_DeviceManager.OnRFIDDeviceUnpairTimeout(udid);
        }

        public static void Set_EA_Value(bool bIsLocal, string udid, string ea_name, string ea_value)
        {
            if (_pluggedInDevices.ContainsKey(udid) == false)
            {
                return;
            }

            long tickNow = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds();
            // we intend to have this value on remote
            if (bIsLocal)
            {
                if( _pluggedInDevices[udid].JAMF_EA_Local_Values.ContainsKey(ea_name) )                    
                {
                    if (_pluggedInDevices[udid].JAMF_EA_Local_Values[ea_name].value != ea_value)
                    {
                        _pluggedInDevices[udid].JAMF_EA_Local_Values[ea_name].updateStamp = tickNow;
                        _pluggedInDevices[udid].JAMF_EA_Local_Values[ea_name].value = ea_value;
                    }
                }
                else
                {
                    _pluggedInDevices[udid].JAMF_EA_Local_Values[ea_name] = new EAInfo();
                    _pluggedInDevices[udid].JAMF_EA_Local_Values[ea_name].name = ea_name;
                    _pluggedInDevices[udid].JAMF_EA_Local_Values[ea_name].value = ea_value;
                    _pluggedInDevices[udid].JAMF_EA_Local_Values[ea_name].updateStamp = tickNow;
                }
            }
            // got the value from remote ( JAMF )
            else
            {
                // if the remote value is not the same as the local valye, we can report a desync
                if (_pluggedInDevices[udid].JAMF_EA_Local_Values.ContainsKey(ea_name))
                {
                    if (_pluggedInDevices[udid].JAMF_EA_Local_Values[ea_name].value != ea_value &&
                        _pluggedInDevices[udid].JAMF_EA_Local_Values[ea_name].updateStamp + 5000 <= tickNow)
                    {
                        long timeDiff = tickNow - _pluggedInDevices[udid].JAMF_EA_Local_Values[ea_name].updateStamp;
                        LogWriter.WriteLog("DeviceStatusCache : EA " + ea_name + " might be out of sync for " + 
                            udid + " remote = " + ea_value + " local = " + _pluggedInDevices[udid].JAMF_EA_Local_Values[ea_name].value +
                            " " + timeDiff.ToString() + " ms passed since we tried to update it");

                        // force a server side JAMF update of this EA. It should be the same as on the local 
                        JMF_MDM.UpdateDeviceEAValue(udid, EnumExtensions.ParseStringToEnum<JAMF_EA>(ea_name), _pluggedInDevices[udid].JAMF_EA_Local_Values[ea_name].value);
                    }
                }

                // set the remote value so we can debug it later
                if (_pluggedInDevices[udid].JAMF_EA_Remote_Values.ContainsKey(ea_name))
                {
                    if (_pluggedInDevices[udid].JAMF_EA_Remote_Values[ea_name].value != ea_value)
                    {
                        _pluggedInDevices[udid].JAMF_EA_Remote_Values[ea_name].updateStamp = tickNow;
                        _pluggedInDevices[udid].JAMF_EA_Remote_Values[ea_name].value = ea_value;
                    }
                }
                else
                {
                    _pluggedInDevices[udid].JAMF_EA_Remote_Values[ea_name] = new EAInfo();
                    _pluggedInDevices[udid].JAMF_EA_Remote_Values[ea_name].name = ea_name;
                    _pluggedInDevices[udid].JAMF_EA_Remote_Values[ea_name].value = ea_value;
                    _pluggedInDevices[udid].JAMF_EA_Remote_Values[ea_name].updateStamp = tickNow;
                }
            }
        }
    }
}
