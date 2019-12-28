using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Mail;
using System.Text;
using System.Threading.Tasks;

using EASendMail;

namespace ReadFortrade1
{
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
                oMail.Subject = "test email from gmail account";

                // Set email body
                oMail.TextBody = "this is a test email sent from c# project with gmail.";

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
}
