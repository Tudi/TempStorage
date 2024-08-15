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
    }

    public class JMF_MDM
    {
        // Hardcoded. Should be some secure/encrypted data
        const string clientURL = "pioneersquarenfr";
        const string username = "USBtest";
        const string password = "testingUSB2024";

        // will be obtained once needed
        static string bearerToken = "";

        // loaded from config file
        public static string HubLocation = ConfigReader.GetConfigValue("HubLocation");

        public static string GetBearerToken()
        {
            Tuple<string, string> res = GetBearerToken(clientURL, username, password);
            if(res != null)
            {
                bearerToken = res.Item1;
                return res.Item1;
            }
            return "";
        }

        public static string GetMobileDeviceData(string UDID)
        {
            using (HttpClient client = new HttpClient())
            {
                var request = new HttpRequestMessage(HttpMethod.Get, "https://" + clientURL + ".jamfcloud.com/JSSResource/mobiledevices/udid/" + UDID);
                request.Headers.Accept.Add(new MediaTypeWithQualityHeaderValue("application/json"));
                request.Headers.Authorization = new AuthenticationHeaderValue("Bearer", bearerToken);

                try
                {
                    var response = client.SendAsync(request).Result;

                    if (response.IsSuccessStatusCode)
                    {
                        string responseBody = response.Content.ReadAsStringAsync().Result;
                        Console.WriteLine("Response:");
                        Console.WriteLine(responseBody);
                        return responseBody;
                    }
                    else
                    {
                        Console.WriteLine($"Failed to get resource. Status code: {response.StatusCode}");
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Exception occurred: {ex.Message}");
                }
            }

            return "";
        }

        public static bool GetMobileDeviceExtensionAttributes(int DeviceId)
        {
            using (HttpClient client = new HttpClient())
            {
                var request = new HttpRequestMessage(HttpMethod.Get, "https://" + clientURL + ".jamfcloud.com/JSSResource/mobiledeviceextensionattributes/id/" + DeviceId);
                request.Headers.Accept.Add(new MediaTypeWithQualityHeaderValue("application/json"));
                request.Headers.Authorization = new AuthenticationHeaderValue("Bearer", bearerToken);

                try
                {
                    var response = client.SendAsync(request).Result;

                    if (response.IsSuccessStatusCode)
                    {
                        string responseBody = response.Content.ReadAsStringAsync().Result;
                        Console.WriteLine("Response:");
                        Console.WriteLine(responseBody);
                    }
                    else
                    {
                        Console.WriteLine($"Failed to get resource. Status code: {response.StatusCode}");
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Exception occurred: {ex.Message}");
                }
            }

            return false;
        }

        public static string GetMobileDeviceExtensionAttributes2(string DeviceUDId)
        {
            using (HttpClient client = new HttpClient())
            {
                var request = new HttpRequestMessage(HttpMethod.Get, "https://" + clientURL + ".jamfcloud.com/JSSResource/mobiledevices/udid/" + DeviceUDId + "/subset/extensionattributes");
                request.Headers.Accept.Add(new MediaTypeWithQualityHeaderValue("application/json"));
                request.Headers.Authorization = new AuthenticationHeaderValue("Bearer", bearerToken);

                try
                {
                    var response = client.SendAsync(request).Result;
                    //HttpResponseMessage response = client.GetAsync(clientURL + "/JSSResource/mobiledevices/udid/" + UDID).Result;

                    if (response.IsSuccessStatusCode)
                    {
                        string responseBody = response.Content.ReadAsStringAsync().Result;
                        Console.WriteLine("Response:");
                        Console.WriteLine(responseBody);
                        return responseBody;
                    }
                    else
                    {
                        Console.WriteLine($"Failed to get resource. Status code: {response.StatusCode}");
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Exception occurred: {ex.Message}");
                }
            }

            return "";
        }

        public static bool UpdateDeviceEAValue(string DeviceUDID, string NewStatus, JAMF_EA ea, bool NoRetry = false)
        {
            using (HttpClient client = new HttpClient())
            {
//                var request = new HttpRequestMessage(HttpMethod.Put, "https://" + clientURL + ".jamfcloud.com/JSSResource/mobiledevices/udid/" + DeviceUDID);
                var request = new HttpRequestMessage(HttpMethod.Put, "https://" + clientURL + ".jamfcloud.com/JSSResource/mobiledevices/udid/" + DeviceUDID + "/subset/extensionattributes");
                request.Headers.Accept.Add(new MediaTypeWithQualityHeaderValue("text/xml"));
                request.Headers.Authorization = new AuthenticationHeaderValue("Bearer", bearerToken);

                var xmlString_ManuallyMade = "<mobile_device>" +
                        "<extension_attributes>" +
                            "<extension_attribute>" +
                                "<id>" + (int)ea + "</id>" +
                                "<value>" + NewStatus + "</value>" +
                            "</extension_attribute>" +
                        "</extension_attributes>" +
                    "</mobile_device>";
                var xml = new StringContent(xmlString_ManuallyMade, System.Text.Encoding.UTF8, "text/xml");

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
                        return UpdateDeviceEAValue(DeviceUDID, NewStatus, ea, true);
                    }
                    else
                    {
                        Console.WriteLine($"Failed to get resource. Status code: {response.StatusCode}");
                        return false;
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Exception occurred: {ex.Message}");
                }
            }

            return false;
        }

        static Tuple<string, string> GetBearerToken(string url, string username, string password)
        {
            using (HttpClient client = new HttpClient())
            {
                // Set the basic authentication header
                var authToken = Convert.ToBase64String(Encoding.ASCII.GetBytes(username+":"+ password));
                client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Basic", authToken);

                // Make the POST request
                HttpResponseMessage response = client.PostAsync("https://" + url + ".jamfcloud.com/api/v1/auth/token", null).Result;

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
