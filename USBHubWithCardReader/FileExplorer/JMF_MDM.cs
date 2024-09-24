using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Http;
using System.Net.Http.Headers;
using Newtonsoft.Json.Linq;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.StartPanel;
using Newtonsoft.Json;
using System.Xml;
using System.Reflection;
using System.Threading;

namespace FileExplorer
{
    [AttributeUsage(AttributeTargets.Field, Inherited = false, AllowMultiple = false)]
    sealed class StringValueAttribute : Attribute
    {
        public string Value { get; }

        public StringValueAttribute(string value)
        {
            Value = value;
        }
    }

    // might need a more complex dictionary later to store possible values for specific EA
    public enum JAMF_EA
    {
        [StringValue("Plugged-In")]
        EA_PI_STATE = 1,

        [StringValue("HubLocation")]
        EA_Location = 2,

        [StringValue("User")]
        EA_User = 3,
    }

    public static class EnumExtensions
    {
        public static string GetStringValue(this Enum value)
        {
            var type = value.GetType();
            var fieldInfo = type.GetField(value.ToString());
            var attributes = (StringValueAttribute[])fieldInfo.GetCustomAttributes(typeof(StringValueAttribute), false);
            return attributes.Length > 0 ? attributes[0].Value : value.ToString();
        }

        public static TEnum ParseStringToEnum<TEnum>(string input) where TEnum : struct, Enum
        {
            // Iterate through all enum values
            foreach (var enumValue in Enum.GetValues(typeof(TEnum)))
            {
                var enumAsEnum = (Enum)enumValue;
                var stringValue = enumAsEnum.GetStringValue();

                // Compare the string value to the input (case-insensitive)
                if (string.Equals(stringValue, input, StringComparison.OrdinalIgnoreCase))
                {
                    return (TEnum)enumValue;
                }
            }

            // If no match is found, throw an exception or return a default value
            throw new ArgumentException($"No matching enum value found for input: {input}");
        }

    }

    public class JMF_MDM
    {
        // Hardcoded. Should be some secure/encrypted data
        public static string clientURL = ConfigReader.GetConfigValue("JAMF_Client_Url");
        public static string username = ConfigReader.GetConfigValue("JAMF_Username");
        public static string password = ConfigReader.GetConfigValue("JAMF_Password");

        // will be obtained once needed
        static string bearerToken = "";

        // loaded from config file
        public static string HubLocation = ConfigReader.GetConfigValue("HubLocation");

        public static readonly SemaphoreSlim _mutex = new SemaphoreSlim(1, 1);
        public static DateTime _lastCallTime = DateTime.MinValue;
        public static int APIRetryCount = ConfigReader.GetConfigValueInt("JAMF_APIRetryCount", 1);
        public static int WaitBetweenAPICalls = ConfigReader.GetConfigValueInt("JAMF_APICallCooldownTime", 0);

        public static void SyncCallFuncPeriodically(bool bLock, bool bUnlock)
        {
            // feature enabled only if there is an API call cooldown time
            // if feature is disabled, API calls will be executed in parallel
            if(WaitBetweenAPICalls == 0)
            {
                return;
            }

            // lock JAMF API calls at the start of your function
            if (bLock)
            {
                _mutex.Wait();
                TimeSpan timeSinceLastCall = DateTime.Now - _lastCallTime;
                int sleepDuration = WaitBetweenAPICalls - (int)timeSinceLastCall.TotalMilliseconds;
                if (sleepDuration > 0)
                {
                    LogWriter.WriteLog("JMF_MDM: Anti JAMF API call spam waiting " + sleepDuration.ToString() + " ms ");
                    Thread.Sleep(sleepDuration);
                }
            }
            
            // Unlock JAMF API calls at the exit of your function
            if (bUnlock)
            {
                // Update the last call time
                _lastCallTime = DateTime.Now;
                _mutex.Release();
            }
        }

        public static string GetBearerToken()
        {
            for (int retry = 0; retry < APIRetryCount; retry++)
            {
                Tuple<string, string> res = GetBearerToken(clientURL, username, password);
                if (res != null)
                {
                    bearerToken = res.Item1;
                    return res.Item1;
                }
            }
            LogWriter.WriteLog("JMF_MDM: Failed to obtain bearer token. Unable to use JAMF API");
            return "";
        }

        public static string GetMobileDeviceData(string UDID)
        {
            SyncCallFuncPeriodically(true, false);

            for (int retry = 0; retry < APIRetryCount; retry++)
            {
                using (HttpClient client = new HttpClient())
                {
                    var request = new HttpRequestMessage(HttpMethod.Get, clientURL + "/JSSResource/mobiledevices/udid/" + UDID);
                    request.Headers.Accept.Add(new MediaTypeWithQualityHeaderValue("application/json"));
                    request.Headers.Authorization = new AuthenticationHeaderValue("Bearer", bearerToken);

                    try
                    {
                        var response = client.SendAsync(request).Result;

                        if (response.IsSuccessStatusCode)
                        {
                            string responseBody = response.Content.ReadAsStringAsync().Result;
                            return responseBody;
                        }
                        else if (response.StatusCode == System.Net.HttpStatusCode.Unauthorized)
                        {
                            // retry this request
                            GetBearerToken();
                            continue;
                        }
                        else
                        {
                            LogWriter.WriteLog($"JMF_MDM: GetMobileDeviceData Failed to get resource. Status code: {response.StatusCode}");
                            SyncCallFuncPeriodically(false, true);
                            return "";
                        }
                    }
                    catch (Exception ex)
                    {
                        LogWriter.WriteLog($"JMF_MDM : GetMobileDeviceData Exception occurred: {ex.Message}");
                    }
                }
            }

            LogWriter.WriteLog($"JMF_MDM: GetMobileDeviceData Failed to get device " + UDID + " data ");
            SyncCallFuncPeriodically(false, true);
            return "";
        }

        /*
        public static string GetMobileDeviceExtensionAttributes(int DeviceId)
        {
            using (HttpClient client = new HttpClient())
            {
                var request = new HttpRequestMessage(HttpMethod.Get, clientURL + "/JSSResource/mobiledeviceextensionattributes/id/" + DeviceId);
                request.Headers.Accept.Add(new MediaTypeWithQualityHeaderValue("application/json"));
                request.Headers.Authorization = new AuthenticationHeaderValue("Bearer", bearerToken);

                try
                {
                    var response = client.SendAsync(request).Result;

                    if (response.IsSuccessStatusCode)
                    {
                        string responseBody = response.Content.ReadAsStringAsync().Result;
//                        Console.WriteLine("Response:");
//                        Console.WriteLine(responseBody);
                        return responseBody;
                    }
                    else
                    {
                        LogWriter.WriteLog($"Failed to get resource. Status code: {response.StatusCode}");
                    }
                }
                catch (Exception ex)
                {
                    LogWriter.WriteLog($"Exception occurred: {ex.Message}");
                }
            }

            return "";
        } 

        public static string GetMobileDeviceExtensionAttributes2(string DeviceUDId)
        {
            using (HttpClient client = new HttpClient())
            {
                var request = new HttpRequestMessage(HttpMethod.Get, clientURL + "/JSSResource/mobiledevices/udid/" + DeviceUDId + "/subset/extensionattributes");
                request.Headers.Accept.Add(new MediaTypeWithQualityHeaderValue("application/json"));
                request.Headers.Authorization = new AuthenticationHeaderValue("Bearer", bearerToken);

                try
                {
                    var response = client.SendAsync(request).Result;
                    //HttpResponseMessage response = client.GetAsync(clientURL + "/JSSResource/mobiledevices/udid/" + UDID).Result;

                    if (response.IsSuccessStatusCode)
                    {
                        string responseBody = response.Content.ReadAsStringAsync().Result;
//                        Console.WriteLine("Response:");
//                        Console.WriteLine(responseBody);
                        return responseBody;
                    }
                    else
                    {
                        LogWriter.WriteLog($"Failed to get resource. Status code: {response.StatusCode}");
                    }
                }
                catch (Exception ex)
                {
                    LogWriter.WriteLog($"Exception occurred: {ex.Message}");
                }
            }

            return "";
        }*/

        public static bool UpdateDeviceEAValue(string DeviceUDID, JAMF_EA JamfEAId, string NewValue)
        {
            // let JAMF know that we unassigned this device from this user
            List<(int JamfEAId, string NewValue)> eas = new List<(int, string)>
                {
                    ((int)JamfEAId, NewValue) // device is not in use
                };
            return UpdateDeviceEAValue(DeviceUDID, eas);
        }

        public static bool UpdateDeviceEAValue(string DeviceUDID, List<(int JamfEAId, string NewValue)> eas)
        {
            SyncCallFuncPeriodically(true, false);

            string xmlString = "";
            for (int retry = 0; retry < APIRetryCount; retry++)
            {
                using (HttpClient client = new HttpClient())
                {
                    var request = new HttpRequestMessage(HttpMethod.Put, clientURL + "/JSSResource/mobiledevices/udid/" + DeviceUDID + "/subset/extensionattributes");
                    request.Headers.Accept.Add(new MediaTypeWithQualityHeaderValue("text/xml"));
                    request.Headers.Authorization = new AuthenticationHeaderValue("Bearer", bearerToken);

                    xmlString = "<mobile_device>" +
                                        "<extension_attributes>";
                    // Loop through each extension attribute (ea) and construct its XML part
                    foreach (var ea in eas)
                    {
                        xmlString += "<extension_attribute>" +
                                     "<id>" + ea.JamfEAId + "</id>" +
                                     "<value>" + ea.NewValue + "</value>" +
                                     "</extension_attribute>";

                        // also set the expected value
                        if(retry == 0)
                        {
                            JAMF_EA eaName = (JAMF_EA)ea.JamfEAId;
                            DeviceStatusCache.Set_EA_Value(true, DeviceUDID, eaName.GetStringValue(), ea.NewValue);
                        }
                    }

                    // Close the XML tags
                    xmlString += "</extension_attributes>" +
                                 "</mobile_device>";

                    var xml = new StringContent(xmlString, System.Text.Encoding.UTF8, "text/xml");

                    request.Content = xml;

                    try
                    {
                        var response = client.SendAsync(request).Result;

                        if (response.IsSuccessStatusCode)
                        {
                            string responseBody = response.Content.ReadAsStringAsync().Result;
                        }
                        else if (response.StatusCode == System.Net.HttpStatusCode.Unauthorized)
                        {
                            GetBearerToken();
                            continue;
                        }
                        else
                        {
                            LogWriter.WriteLog($"JMF_MDM : UpdateDeviceEAValue Failed. Status code: {response.StatusCode}");
                            SyncCallFuncPeriodically(false, true);
                            return false;
                        }
                    }
                    catch (Exception ex)
                    {
                        LogWriter.WriteLog($"JMF_MDM : UpdateDeviceEAValue Exception occurred: {ex.Message}");
                    }
                }
            }

            LogWriter.WriteLog("JMF_MDM : UpdateDeviceEAValue failed to update using " + xmlString);
            SyncCallFuncPeriodically(false, true);
            return false;
        }

        public static Tuple<string, string> GetBearerToken(string url, string username, string password)
        {
            using (HttpClient client = new HttpClient())
            {
                // Set the basic authentication header
                var authToken = Convert.ToBase64String(Encoding.ASCII.GetBytes(username+":"+ password));
                client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Basic", authToken);

                // Make the POST request
                HttpResponseMessage response = client.PostAsync(url + "/api/v1/auth/token", null).Result;

                // Ensure the request was successful
                if (response.IsSuccessStatusCode)
                {
                    // Read the response content
                    string responseContent = response.Content.ReadAsStringAsync().Result;

                    // Parse the JSON response to extract the token
                    JObject jsonResponse = JObject.Parse(responseContent);
                    string token = jsonResponse["token"].ToString();
                    string expires = jsonResponse["expires"].ToString();

                    return new Tuple<string, string>(token, expires);
                }
                else
                {
                    // Handle failure response
                    return null;
                }
            }
        }
    }
}
