using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http.Headers;
using System.Text;
using System.Threading.Tasks;
using CradleWindowsAgent.Tools;
using Newtonsoft.Json.Linq;
using PusherClient;

namespace CradleWindowsAgent.PushNotifications
{
    class PusherHandler
    {
        private static Pusher _pusher;
        private static Channel _privateChannel;
        public PusherHandler()
        {
            _ = InitPusher();
        }
        public static async Task InitPusher()
        {
            LogWriter.WriteLog("InitPusher()");

            _pusher = new Pusher("09dafbaa79b925baeea6", new PusherOptions
            {
                //TODO PRIVATE CHANNELS CAN BE SUBSCRIBED AFTER AUTH IS DONE
                //Authorizer = new HttpAuthorizer("https://cradle.io/pusher/user-auth")
                //{
                //    AuthenticationHeader = new AuthenticationHeaderValue("Bearer", "accessToken"),
                //},

                Cluster = "ap2",
                TraceLogger = new TraceLogger(),
            });

            _pusher.ConnectionStateChanged += PusherConnectionStateChanged;
            _pusher.Error += PusherError;
            _pusher.Subscribed += SubscribedHandler;
            _pusher.CountHandler += CountHandler;
            _pusher.Connected += Connected;
            _pusher.BindAll(EventsListener);

            // Subscribe, Setup private encrypted channel
            try
            {
                string userID = "TEST_CHANNEL";
                //TODO PRIVATE CHANNELS CAN BE SUBSCRIBED AFTER AUTH IS DONE

                //_privateChannel = await _pusher.SubscribeAsync("private-" + userID);
                _privateChannel = await _pusher.SubscribeAsync(userID);
                _privateChannel.BindAll(ChannelEvent);

                LogWriter.WriteLog("Pusher BindAll Done");
            }
            catch (ChannelUnauthorizedException unauthorizedException)
            {
                LogWriter.WriteLog($"Authorization failed for {unauthorizedException.ChannelName}. {unauthorizedException.Message}");
            }
            // Connect
            await _pusher.ConnectAsync().ConfigureAwait(false);

        }


        static void EventsListener(string eventName, PusherEvent eventData)
        {
            LogWriter.WriteLog($"{Environment.NewLine} GeneralListner {eventName} {eventData.Data}");

            //Check if the device id is the same, return if same
            string sData = eventData.Data;
            if (sData.Length > 0)
            {
                JObject ObjData = JObject.Parse(sData);
                string sDeviceID = ObjData.ContainsKey("any") ? ObjData["any"].ToString() : "";
            }

            if (eventName == "settings-updated")
            {
                LogWriter.WriteLog("Pusher settings-updated");

                //Do the needful to with the updated settings
            }
            //Add other push notifications handlers and call functions accordingly

        }

        static void Connected(object sender)
        {
            LogWriter.WriteLog($"Pusher Channel Connected");
        }

        // Subscribed event handler
        static void SubscribedHandler(object sender, Channel channel)
        {
            LogWriter.WriteLog($"Subscribed To Channel {channel.Name}");
        }

        static void CountHandler(object sender, string data)
        {
            Console.WriteLine($"CountHandler {data}");
        }

        static void ChannelEvent(string eventName, PusherEvent eventData)
        {
            LogWriter.WriteLog($"{Environment.NewLine}{eventName} {eventData.Data}");
        }

        static void PusherError(object sender, PusherException error)
        {
            TraceMessage(sender, $"{Environment.NewLine}Pusher Error: {error.Message}{Environment.NewLine}{error}");
        }

        static void PusherConnectionStateChanged(object sender, ConnectionState state)
        {
            TraceMessage(sender, $"Connection state: {state}");
        }

        static void TraceMessage(object sender, string message)
        {
            Pusher client = sender as Pusher;
            LogWriter.WriteLog($"{DateTime.Now:o} - {client.SocketID} - {message}");
        }


    }
}
