using Discord;
using Discord.Commands;
using Discord.WebSocket;
using Discord.Net.Providers.WS4Net;
using Microsoft.Extensions.DependencyInjection;
using System;
using System.Threading.Tasks;
using System.Reflection;
using System.Threading;
using MySql.Data.MySqlClient;
using System.IO;

namespace ChatConnector
{
    class Program
    {
        static void Main(string[] args) => new Program().RunBotAsync().GetAwaiter().GetResult();

        private DiscordSocketClient _client;
        private CommandService _commands;
        private IServiceProvider _services;
        ulong GeneralChannelId;
        int ChatServerInternalID;
        string MysqlConnectionString;
        string BotSecretToken;
        bool BotIsRunning;  //maybe some external command can shut it down

        public async Task RunBotAsync()
        {
            //have to find out how to get these
            GeneralChannelId = Convert.ToUInt64(GetConfigVariable("WatchedChannelId"));
            ChatServerInternalID = Convert.ToInt32(GetConfigVariable("CurrentServerId"));
            MysqlConnectionString = GetConfigVariable("MysqlConnectionString");            
            BotSecretToken = GetConfigVariable("BotSecret");//https://discordapp.com/developers/applications/me
            BotIsRunning = true;

            //in case bot deconnects. Force it to reconnect
            Thread threadBotConnectionMonitor = new Thread(PeriodicReconnectBot);
            threadBotConnectionMonitor.Start();

            //wait for initializations
            while (_client == null || _client.ConnectionState != Discord.ConnectionState.Connected)
                Thread.Sleep(1000);

            Console.WriteLine("Going to connect to channel with id " + GeneralChannelId + ".With name : " + _client.GetChannel(GeneralChannelId).ToString());

            //periodically update our channel what ingame players talked about
            Thread threadWriteChatToDB = new Thread(PeriodicReadFromDB);
            threadWriteChatToDB.Start();

            await Task.Delay(-1);
        }

        public async Task CreateNewDiscordConnection()
        {
            _client = new DiscordSocketClient(new DiscordSocketConfig     // Add the discord client to the service provider
            {
                WebSocketProvider = WS4NetProvider.Instance,
                LogLevel = LogSeverity.Verbose,
                MessageCacheSize = 1000     // Tell Discord.Net to cache 1000 messages per channel
            });
            _commands = new CommandService();
            _services = new ServiceCollection()
                .AddSingleton(_client)
                .AddSingleton(_commands)
                .BuildServiceProvider();

            //event subscription
            _client.Log += Log;

            await RegisterCommandsAsync();

            await _client.LoginAsync(TokenType.Bot, BotSecretToken);

            await _client.StartAsync();
        }

        private void PeriodicReconnectBot()
        {
            while (BotIsRunning)
            {
                //make bot 
                CreateNewDiscordConnection();

                //wait for the bot to connect
                while (_client == null || _client.ConnectionState != Discord.ConnectionState.Connected)
                    Thread.Sleep(1000);

                //if bot is running then it's all good
                while (_client.ConnectionState == Discord.ConnectionState.Connected)
                    Thread.Sleep(1000);
            }
        }

        private string GetConfigVariable(string VariableName)
        {
            string ret = "";
            string path = Directory.GetCurrentDirectory() + "\\ChatConnect.conf";
            FileInfo config_file = new FileInfo(path);
            if (config_file.Exists)
            {
                using (StreamReader sr = new StreamReader(config_file.FullName))
                {
                    String line;
                    while ((line = sr.ReadLine()) != null)
                    {
                        if (line.StartsWith(VariableName + "="))
                        {
                            ret = line.Substring(VariableName.Length + 1);
                        }
                    }
                }
            }
            else
            {
                throw new Exception("config is missing");
            }

            if (ret == "")
                Console.WriteLine("!!!Warning missing variable '" + VariableName + "'in ChatConnect.conf");

            return ret;
        }


        private void InsertNewChatRow(string User, string Msg)
        {
            MySqlConnection _MsqlConnection = new MySqlConnection(MysqlConnectionString);
            _MsqlConnection.Open();
            string Command = "insert into ChatConnector (Stamp,SenderType,Server,Channel,UserName,Msg)values("
            + "@StampInsertedAt,"
            + ChatServerInternalID + ",'Zombiewow','general','"
            + MySqlHelper.EscapeString(User) + "','"
            + MySqlHelper.EscapeString(Msg) + "')";
            //Console.WriteLine(Command);
            using (MySqlCommand cmd = _MsqlConnection.CreateCommand())
            {
                cmd.CommandText = Command;
                cmd.Parameters.AddWithValue("@StampInsertedAt", (Int32)(DateTime.UtcNow.Subtract(new DateTime(1970, 1, 1))).TotalSeconds);
                int RowsAffected = cmd.ExecuteNonQuery();
            }
            _MsqlConnection.Close();
        }

        private string ReplaceHAnyLinkType(string Msg,string LinkType)
        {
            int IndexOfItemTxt = 0;

            //|cff1eff00|Hitem:1391:0:0:0:0:0:0:0:114|h[Riverpaw Mystic Staff]|h|r.  
            //|cff9d9d9d|Hitem:1766:0:0:0:0:0:0:0:114|h[Canvas Cloak]|h|r..
            //|cff71d5ff|Hspell:674|h[Dual Wield]|h|r
            //|cffffd000|Htrade:51302:450:450:23F:e+WFFPCtCvZYtsVAoMAAeIBCDAMAr4hAAAAAAAc8//CAAAAAEAAAAAAAAgwgA4/vK5//////fy/DAAAAwwA8/A8/I0ZD|h[Leatherworking]|h|r
            //|cffffd000|Henchant:64441|h[Enchanting: Enchant Weapon - Blade Ward]|h|r
            //|cffffff00|Hachievement:880:0000000000000429:1:2:24:18:0:0:0:0|h[Swift Zulian Tiger]|h|r

            //parse message to extract links / coloring
            string TextToFind = "|H"+ LinkType+":";
            string EnOfALinkMarker = "]|h|r";
            do
            {
                IndexOfItemTxt = Msg.IndexOf(TextToFind);
                //Console.WriteLine("Found link that beggins at index " + IndexOfItemTxt);
                if (IndexOfItemTxt >= 0)
                {
                    int IndexOfItemId = IndexOfItemTxt + TextToFind.Length;
                    string RemainingStringWithItemId = Msg.Substring(IndexOfItemId);
                    int IndexOfNextNonNumber1 = RemainingStringWithItemId.IndexOf(":");
                    int IndexOfNextNonNumber2 = RemainingStringWithItemId.IndexOf("|");
                    if(IndexOfNextNonNumber1 < IndexOfNextNonNumber2 && IndexOfNextNonNumber1 != -1)
                        RemainingStringWithItemId = RemainingStringWithItemId.Substring(0, IndexOfNextNonNumber1);
                    else
                        RemainingStringWithItemId = RemainingStringWithItemId.Substring(0, IndexOfNextNonNumber2);
                    int IdOfItem = Int32.Parse(RemainingStringWithItemId);
                    //Console.WriteLine("item id from the link is  " + IdOfItem + "from remaining string " + RemainingStringWithItemId);
                    int EndOfTheLink = Msg.IndexOf(EnOfALinkMarker) + EnOfALinkMarker.Length;
                    //                                            string FullLinkReplacement = "[" + TextToFind + IdOfItem.ToString() + "";
                    //new message should be stripped of wow link and replaced by wowhead link
                    //Console.WriteLine("frist part of the msg is " + Msg.Substring(0, IndexOfItemTxt - 10));
                    //Console.WriteLine("last part of the msg is " + Msg.Substring(EndOfTheLink));
                    if(LinkType=="item")
                        Msg = Msg.Substring(0, IndexOfItemTxt - 10) + " http://www.wowhead.com/item=" + IdOfItem + " " + Msg.Substring(EndOfTheLink);
                    else if (LinkType == "achievement")
                        Msg = Msg.Substring(0, IndexOfItemTxt - 10) + " http://www.wowhead.com/achievement=" + IdOfItem + " " + Msg.Substring(EndOfTheLink);
                    else
                        Msg = Msg.Substring(0, IndexOfItemTxt - 10) + " http://www.wowhead.com/spell=" + IdOfItem + " " + Msg.Substring(EndOfTheLink);
                }
            } while (IndexOfItemTxt > 0);

            return Msg;
        }

        private string ReplaceWowLinks(string Msg)
        {
            Msg = ReplaceHAnyLinkType(Msg, "item");
            Msg = ReplaceHAnyLinkType(Msg, "spell");
            Msg = ReplaceHAnyLinkType(Msg, "trade");
            Msg = ReplaceHAnyLinkType(Msg, "enchant");
            Msg = ReplaceHAnyLinkType(Msg, "achievement");
            return Msg;
        }

        private void DeleteOlderMessages(int LastFetchedRowId)
        {
            if (LastFetchedRowId == 0)
                LastFetchedRowId = 0x0FFFFFFF;
            //delete messages we just fetched/processed
            using (MySqlConnection _MsqlConnection = new MySqlConnection(MysqlConnectionString))
            {
                _MsqlConnection.Open();
                if (_MsqlConnection.State == System.Data.ConnectionState.Open)
                {
                    using (MySqlCommand cmd = _MsqlConnection.CreateCommand())
                    {
                        cmd.CommandText = "delete from ChatConnector where SenderType != " + ChatServerInternalID + " and Id <= " + LastFetchedRowId;
                        //Console.WriteLine("Delete sql is :" + cmd.CommandText);
                        //                    cmd.Parameters.AddWithValue("@LastFetchedRowId", LastFetchedRowId);
                        int RetCode = cmd.ExecuteNonQuery();
                    }
                }
            }

        }

        private void PeriodicReadFromDB()
        {
            int LastFetchedRowId = 0;
            DeleteOlderMessages(LastFetchedRowId);
            while (BotIsRunning)
            {
                int PrevFetchedRowId = LastFetchedRowId;
                //fetch new messages to sent by wow
                using (MySqlConnection _MsqlConnection = new MySqlConnection(MysqlConnectionString))
                {
                    _MsqlConnection.Open();
                    if (_MsqlConnection.State == System.Data.ConnectionState.Open)
                    {
                        using (MySqlCommand cmd = _MsqlConnection.CreateCommand())
                        {
                            cmd.CommandText = "select Id,UserName,Msg from ChatConnector where SenderType!=" + ChatServerInternalID + " and id>" + LastFetchedRowId + " order by 268435455 - id desc limit 10";
                            using (MySqlDataReader dr = cmd.ExecuteReader())
                            {
                                while (dr.HasRows && dr.Read())
                                {
                                    LastFetchedRowId = (int)(dr["Id"]);
                                    string User = dr["UserName"].ToString();
                                    string Msg = dr["Msg"].ToString();

                                    //for debug purpuses
                                    Console.WriteLine("W->" + User + " : " + Msg);

                                    Msg = ReplaceWowLinks(Msg);

                                    //send messages to Discord
                                    if (_client.ConnectionState == Discord.ConnectionState.Connected)
                                        ((ISocketMessageChannel)_client.GetChannel(GeneralChannelId)).SendMessageAsync(User + ":" + Msg);
                                }
                            }
                        }
                    }
                    _MsqlConnection.Close();
                }

                if (PrevFetchedRowId != LastFetchedRowId)
                {
                    DeleteOlderMessages(LastFetchedRowId);
                }
            }
        }

        private Task Log(LogMessage arg)
        {
            Console.WriteLine(arg);
            return Task.CompletedTask;
        }

        public async Task RegisterCommandsAsync()
        {
            _client.MessageReceived += HandleCommandAsync;
            await _commands.AddModulesAsync(Assembly.GetEntryAssembly(), _services);
        }

        private async Task HandleCommandAsync(SocketMessage arg)
        {
            var message = arg as SocketUserMessage;

            if (message is null )
                return;

            //Console.WriteLine("From channel " + message.Channel + "(" + message.Channel.Id + ") user : " + message.Author.Username + " id: " + message.Author.Id + " message : " + message.Content);

            if (message.Author.IsBot)
            {
                Console.WriteLine("Command author is a BOT skipping message processing");
                return;
            }

            /*
                        int argPos = 0;
                        if(message.HasStringPrefix("tnt!", ref argPos) || message.HasMentionPrefix(_client.CurrentUser, ref argPos))
                        {
                            var context = new SocketCommandContext(_client, message);

                            var result = await _commands.ExecuteAsync(context, argPos, _services);

                            if (!result.IsSuccess)
                                Console.WriteLine(result.ErrorReason);

            //                await context.Channel.SendMessageAsync("test");
                        }

            */
            if (message.Channel.Name == "general" && GeneralChannelId != message.Channel.Id)
            {
                Console.WriteLine("!! the channel 'general' id is expected to be " + GeneralChannelId + "but it is " + message.Channel.Id);
                Console.WriteLine("Skipping from channel " + message.Channel + " user : " + message.Author.Username + " id: " + message.Author.Id + " message : " + message.Content);
            }
            //            GeneralChannelId = message.Channel.Id;
            //            Console.WriteLine("channel id is : " + GeneralChannelId);

            //only process channel we want to log
            if (message.Channel.Id != GeneralChannelId)
            {
                Console.WriteLine("Message is not from the watched channel");
                return;
            }

            //check if message contains special characters and skip those
            //            if(message.Content)

            //remove emoticons
            string msg = message.Content;
            msg = msg.Replace("??", "");
            msg = msg.Replace("|c", "");
            msg = msg.Replace("|", "");
            if (msg.Length == 0)
            {
                Console.WriteLine("Ignore empty message");
                return;
            }

            //add it to the chat connector
            string Nickname = message.Author.Username;
            //cut out the has and number from the end
            int HashIndex = Nickname.IndexOf('#');
            if (HashIndex > 0 && HashIndex < Nickname.Length)
                Nickname = Nickname.Substring(0, HashIndex);
            try
            {
                if(((IGuildUser)message.Author).Nickname.Length>0)
                    Nickname = ((IGuildUser)message.Author).Nickname;
            }
            catch (Exception) { }

            InsertNewChatRow(Nickname, msg);
            Console.WriteLine("D->" + Nickname + " : " + msg);

            //write message to Db
            //Console.WriteLine("From channel " + message.Channel + " user : " + message.Author.Username + " id: " + message.Author.Id + " message : " + message.Content);
        }
    }
}
