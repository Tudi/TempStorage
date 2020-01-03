using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Mail;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

using EASendMail;

namespace ReadFortrade1
{
    public class NotificationParams
    {
        public NotificationParams()
        {
            SellPriceReached = 0;
            BuyPriceReached = 0;
            LastNotificationStamp = new DateTime(1970); // should trigger a notification
        }
        public string InstrumentName;
        public string MyAction; // in what state is my purchased instrument ?
        public double SellPriceReached;
        public double BuyPriceReached;
        public double NotifyEveryXPriceUnitChange;
        public double BoughtAtPrice;
        public double BoughtAmount;
        public double ProfitExceeded;
        DateTime LastNotificationStamp;
    }
    public class SendNotification
    {
        public static void SendMessage(string msg)
        {
            try
            {
                SmtpMail oMail = new SmtpMail("TryIt");

                // Your gmail email address
                oMail.From = "jb2.lords@gmail.com";

                // Set recipient email address
                oMail.To.Add(new EASendMail.MailAddress("0742237492@orangemail.ro"));
                oMail.To.Add(new EASendMail.MailAddress("jozsab1@gmail.com"));

                // Set email subject
                oMail.Subject = "Fortrade Notification";

                // Set email body
                oMail.TextBody = msg;

                // Gmail SMTP server address
                SmtpServer oServer = new SmtpServer("smtp.gmail.com");

                // Gmail user authentication
                // For example: your email is "gmailid@gmail.com", then the user should be the same
                oServer.User = "jb2.lords@gmail.com";
                oServer.Password = "jcdyjypuawklpafy";

                // If you want to use direct SSL 465 port,
                // please add this line, otherwise TLS will be used.
                // oServer.Port = 465;

                // set 587 TLS port;
                oServer.Port = 587;

                // detect SSL/TLS automatically
                oServer.ConnectType = SmtpConnectType.ConnectSSLAuto;

                //Console.WriteLine("start to send email over SSL ...");

                EASendMail.SmtpClient oSmtp = new EASendMail.SmtpClient();
                oSmtp.SendMail(oServer, oMail);

                //Console.WriteLine("email was sent successfully!");
            }
            catch (Exception ep)
            {
                Console.WriteLine("failed to send email with the following error:");
                Console.WriteLine(ep.Message);
            }
        }
    }
    public class NotificationWatchdog
    {
        public bool WatchodRunning = true;
        List<NotificationParams> WatchedEvents = new List<NotificationParams>();
        DateTime LastEmailSend = new DateTime(1970);
        public void StartPriceChangeWatchdog()
        {
            while (WatchodRunning == true)
            {
                string MailToSend = "";
                //for each watched instrument event, check status
                foreach(NotificationParams itr in WatchedEvents)
                {
                    StockDataHistory iv = Globals.vHistory.GetInstrumentStore(itr.InstrumentName);
                    //can't process until we start getting soem values for this instrument
                    if (iv == null)
                        continue;
                    //send alert on sell price reached ?
                    if(itr.MyAction == "Sell" )
                    {
                        if (iv.PrevSellPrice < itr.SellPriceReached)
                            MailToSend += iv.Name + " SellPrice is " + iv.PrevSellPrice + " watched price is " + itr.SellPriceReached;
                        if(itr.ProfitExceeded != 0)
                        {
                            double ExpectedProfit = (iv.PrevBuyPrice - itr.BoughtAtPrice) * itr.BoughtAmount;
                            if(ExpectedProfit > itr.ProfitExceeded)
                                MailToSend += iv.Name + " profit is " + ExpectedProfit + " exceeded limit " + itr.ProfitExceeded;
                        }
                    }
                    if (itr.MyAction == "Buy")
                    {
                        if (iv.PrevSellPrice < itr.SellPriceReached)
                            MailToSend += iv.Name + " Buy is " + iv.PrevSellPrice + " watched price is " + itr.SellPriceReached;
                        if (itr.ProfitExceeded != 0)
                        {
                            double ExpectedProfit = (iv.PrevBuyPrice - itr.BoughtAtPrice) * itr.BoughtAmount;
                            if (ExpectedProfit > itr.ProfitExceeded)
                                MailToSend += iv.Name + " profit is " + ExpectedProfit + " exceeded limit " + itr.ProfitExceeded;
                        }
                    }
                }
                Thread.Sleep(100);
            }
        }
    }
}
