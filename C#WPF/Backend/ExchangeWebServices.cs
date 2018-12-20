using Microsoft.Exchange.WebServices.Data;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLFClient.Backend
{
    public class ExchangeWebServices
    {
        ExchangeService service = null;
        string LoginEmail = null;
        string LoginPassw = null;
        string ExchangeURL = "";

        public string GetEmail()
        {
            return LoginEmail;
        }

        public bool TestConnect(string Email, string Passw, string Url)
        {
            service = null;
            LoginEmail = Email;
            LoginPassw = Passw;
            ExchangeURL = Url;
            ConnectToServer();
            if(service == null)
            {
                LoginEmail = null;
                LoginPassw = null;
                ExchangeURL = "";
                return false;
            }
            return true;
        }

        public bool ConnectToServer()
        {
            if (service != null)
                return true;
            lock (this)
            {
                //maybe another thread initialized us while waiting on the lock
                if (service != null)
                    return true;
                //if we are missing the connection details, ask for them in a popup window
                if (LoginEmail == null || LoginPassw == null)
                {
                    var cw = new ExchangeDetailsRequest();
                    cw.ShowInTaskbar = false;
                    cw.Owner = System.Windows.Application.Current.MainWindow;
                    cw.ShowDialog();
                }
                //still no details ? Abandon login
                if (LoginEmail == null || LoginPassw == null)
                    return false;
                //try to connect
                service = GetServiceRedirected(LoginEmail, LoginPassw, ExchangeURL);
                // could not connect. Something is wrong. Maybe inspect the exchange url ?
                if (service == null)
                    return false;
            }
            return true;
        }

        public bool ResolveName(string Email)
        {
            ConnectToServer();
            if (service == null)
                return false;
            List<FolderId> folders = new List<FolderId>() { new FolderId(WellKnownFolderName.Contacts) };
            NameResolutionCollection coll = service.ResolveName(Email, folders, ResolveNameSearchLocation.ContactsThenDirectory, false);
            if (coll.Count != 0)
                return true;
            return false;
        }

        public List<OutlookCalendarItem> GetAbsenceInfo(string TargetEmail, int NumDays)
        {
            ConnectToServer();
            if (service == null)
                return new List<OutlookCalendarItem>();

            //start collecting from today
            DateTime StartDate = DateTime.Now;
            DateTime EndDate = StartDate.AddDays(NumDays);

            List<OutlookCalendarItem> ret = new List<OutlookCalendarItem>();

            var options = new AvailabilityOptions
            {
                RequestedFreeBusyView = FreeBusyViewType.DetailedMerged
            };

            var attendees = new List<AttendeeInfo>();
            attendees.Add(new AttendeeInfo { SmtpAddress = TargetEmail, AttendeeType = MeetingAttendeeType.Required });

            var results = service.GetUserAvailability(attendees, new TimeWindow(StartDate, EndDate), AvailabilityData.FreeBusy, options);
            foreach (AttendeeAvailability avail in results.AttendeesAvailability)
            {
                foreach (CalendarEvent item in avail.CalendarEvents)
                {
                    OutlookCalendarItem ci = new OutlookCalendarItem();
                    ci.Start = item.StartTime;
                    ci.Duration = (int)item.EndTime.Subtract(item.StartTime).TotalMinutes;
                    ci.ResponseType = (OutlookResponseStatus)item.FreeBusyStatus;
                    ret.Add(ci);
                }
            }
            return ret;
        }

        private static ExchangeService GetServiceRedirected(string email, string passw, string URL)
        {
            //Console.WriteLine("Got params " + email + " pasw " + passw + " url '" + URL + "'");
            ExchangeService service = new ExchangeService(ExchangeVersion.Exchange2007_SP1);
            service.Credentials = new WebCredentials(email, passw);
            if (URL.Length > 0)
            {
                //Console.WriteLine("Will use given url for exchange service instead auto discovery");
                service.Url = new Uri(URL);
            }
            else
            {
                try
                {
                    service.AutodiscoverUrl(email, RedirectionUrlValidationCallback);
                }
                catch { }
                finally
                {
                    service.Url = new Uri("https://outlook.office365.com/EWS/Exchange.asmx");
                }
            }
            return service;
        }

        private static bool RedirectionUrlValidationCallback(string redirectionUrl)
        {
            return true;    // we go with the flow
                            // The default for the validation callback is to reject the URL.
/*            bool result = false;
            Uri redirectionUri = new Uri(redirectionUrl);
            if (redirectionUri.Scheme == "https")
            {
                result = true;
            }
            return result; */
        }
    }
}
