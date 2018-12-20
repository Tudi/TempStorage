using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Outlook = Microsoft.Office.Interop.Outlook;

namespace BLFClient.Backend
{
    public enum OutlookResponseStatus
    {
        None = 0,
        Organized = 1,
        Tentative = 2,
        Accepted = 3,
        Declined = 4,
        NotResponded = 5
    }

    public struct OutlookCalendarItem
    {
        public int Duration;
        public DateTime Start;
        public OutlookResponseStatus ResponseType;
    };

    public class OutlookAPI
    {
        Outlook.Application outlookObj = null;
        int SkipSearchingOutlook = 0;  //if we try and fail we presume outlook is not installed

        /// <summary>
        /// Check if we have Outlook installed
        /// </summary>
        /// <returns></returns>
        private bool HasOutlookInstalled()
        {
            if (SkipSearchingOutlook == 0)
            {
                var outlookType = Type.GetTypeFromProgID("Outlook.Application");
                if (outlookType == null)
                    SkipSearchingOutlook = 2;   // outlook does not exist
                else
                    SkipSearchingOutlook = 1;   // outlook does exist
            }
            return SkipSearchingOutlook == 1;
        }

        /// <summary>
        /// If we have Outlook installed, connect to it. This will be a persistant connection
        /// </summary>
        private bool ConnectToServer()
        {
            if (outlookObj != null)
                return true;
            if (HasOutlookInstalled() == false)
                return false;
            lock(this)
            {
                //maybe another thread already initialized us while we waited for the lock
                if (outlookObj != null)
                    return true;
                try
                {
                    outlookObj = new Outlook.Application();
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Outlook query object hs been created successfully");
                    return true;
                }
                catch(Exception e)
                {
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Could not create Outlook object. Error : " + e.ToString());
                    outlookObj = null;
                    return false;
                }
            }
        }

        public bool IsConnected()
        {
            if (HasOutlookInstalled() == false)
                return Globals.ExchangeAPI.ConnectToServer();
            return (outlookObj != null);
        }

        /// <summary>
        /// Wrapper to resolve an email with Outlook or with Exchange server
        /// </summary>
        /// <param name="email"></param>
        /// <returns></returns>
        public bool ResolveEmail(string email)
        {
            if(HasOutlookInstalled() == false)
                return Globals.ExchangeAPI.ResolveName(email);
            return _ResolveEmail(email);
        }

        /// <summary>
        /// Initialize the Outlook object in a background thread
        /// </summary>
        public void InitInBackground()
        {
            Task mytask = Task.Run(() => { _InitInBackground(); });
        }

        /// <summary>
        /// Initialize the outlook Object
        /// </summary>
        private void _InitInBackground()
        {
            if (HasOutlookInstalled() == false)
                Globals.ExchangeAPI.ConnectToServer();
            else if (outlookObj == null)
                ConnectToServer();
        }

        /// <summary>
        /// Check if this email exists in the user directory
        /// </summary>
        /// <param name="email"></param>
        /// <returns></returns>
        private bool _ResolveEmail(string email)
        {
            if (String.IsNullOrEmpty(email))
                return false;
            if (ConnectToServer() == false)
                return false;
            Outlook.Recipient rcp = null;
            try
            {
                rcp = outlookObj.GetNamespace("MAPI").CreateRecipient(email);
                rcp.Resolve();
                if (rcp != null && rcp.Resolved)
                    return true;
            }
            catch
            {
                return false;
            }
            finally
            {
                if (rcp != null)
                    Marshal.ReleaseComObject(rcp);
            }
            return false;
        }

        /// <summary>
        /// Wrapper to get all possible apointments for a specific email for X days
        /// </summary>
        /// <param name="UserName"></param>
        /// <param name="Days"></param>
        /// <returns></returns>
        public List<OutlookCalendarItem> GetAppointmentsInRange(string UserName, int Days)
        {
            if(HasOutlookInstalled() == false)
                return Globals.ExchangeAPI.GetAbsenceInfo(UserName, Days);
            return _GetAppointmentsInRange(UserName, Days);
        }

        /// <summary>
        /// Get all possible apointments for a specific email for X days
        /// </summary>
        /// <param name="UserName"></param>
        /// <param name="Days"></param>
        /// <returns></returns>
        private List<OutlookCalendarItem> _GetAppointmentsInRange(string UserName, int Days)
        {
            List<OutlookCalendarItem> ret = new List<OutlookCalendarItem>();

            if (ConnectToServer() == false)
                return ret;

            Outlook.NameSpace oNS = outlookObj.GetNamespace("mapi");
            oNS.Logon(Missing.Value, Missing.Value, true, true);

            Outlook.Recipient oRecip = (Outlook.Recipient)oNS.CreateRecipient(UserName);
            Outlook.Folder calFolder = null;
            try
            {
                calFolder = (Outlook.MAPIFolder)oNS.GetSharedDefaultFolder(oRecip, Outlook.OlDefaultFolders.olFolderCalendar) as Outlook.Folder;
            }
            catch
            {
                return ret;
            }

            //            Outlook.Folder calFolder = outlookObj.Session.GetDefaultFolder(Outlook.OlDefaultFolders.olFolderCalendar) as Outlook.Folder;            
            DateTime start = new DateTime(DateTime.Now.Year, DateTime.Now.Month, DateTime.Now.Day, 0, 0, 0, 0);
            DateTime end = start.AddDays(Days);
            Outlook.Items rangeAppts = GetAppointmentsInRange(calFolder, start, end);
            if (rangeAppts != null)
            {
                foreach (Outlook.AppointmentItem item in rangeAppts)
                {
                    OutlookCalendarItem ci = new OutlookCalendarItem();
                    ci.Start = item.Start;
                    ci.Duration = item.Duration;
                    ci.ResponseType = (OutlookResponseStatus)item.ResponseStatus;
                    ret.Add(ci);
                }
            }

            return ret;
        }
        
        /// <summary>
        /// Get recurring appointments in date range.
        /// </summary>
        /// <param name="folder"></param>
        /// <param name="startTime"></param>
        /// <param name="endTime"></param>
        /// <returns>Outlook.Items</returns>
        private Outlook.Items GetAppointmentsInRange(Outlook.Folder folder, DateTime startTime, DateTime endTime)
        {
            string filter = "[Start] >= '" + startTime.ToString("g") + "' AND [End] <= '" + endTime.ToString("g") + "'";
            try
            {
                Outlook.Items calItems = folder.Items;
                calItems.IncludeRecurrences = true;
                calItems.Sort("[Start]", Type.Missing);
                Outlook.Items restrictItems = calItems.Restrict(filter);
                if (restrictItems.Count > 0)
                {
                    return restrictItems;
                }
                else
                {
                    return null;
                }
            }
            catch { return null; }
        }

        /// <summary>
        /// Get all possible registered email in Outlook so we can populate "select profile"dropdown in absence manager
        /// </summary>
        /// <returns></returns>
        public List<string> GetAccountList()
        {
            List<string> ret = new List<string>();
            if (ConnectToServer() == false)
                return ret;

            // The Namespace Object (Session) has a collection of accounts.
            Outlook.Accounts accounts = outlookObj.Session.Accounts;

            // Concatenate a message with information about all accounts.
            foreach (Outlook.Account account in accounts)
                ret.Add(account.SmtpAddress);

            return ret;
        }
    }

    /*       public bool IsCurrentUserEmail(string email)
           {
               if(outlookObj == null)
                   outlookObj = new Outlook.Application();
               if (outlookObj != null)
               {
                   //Is it our default primary account ?
                   string UserInfo = GetCurrentUserInfo(outlookObj);
                   if (UserInfo.IndexOf(email) != 0)
                       return true;
                   //is it our secondary account ?
                   string MultiAccountList = DisplayAccountInformation(outlookObj);
                   if (MultiAccountList.IndexOf(email) != 0)
                       return true;
               }
               //for some reason we could not confirm this email
               return false;
           }*/
    /*
            private string GetCurrentUserInfo(Outlook.Application Application)
            {
                string ret = "";
                Outlook.AddressEntry addrEntry = Application.Session.CurrentUser.AddressEntry;
                if (addrEntry.Type == "EX")
                {
                    Outlook.ExchangeUser currentUser = Application.Session.CurrentUser.AddressEntry.GetExchangeUser();
                    if (currentUser != null)
                    {
                        StringBuilder sb = new StringBuilder();
                        sb.AppendLine("Name: "
                            + currentUser.Name);
                        sb.AppendLine("STMP address: "
                            + currentUser.PrimarySmtpAddress);
                        sb.AppendLine("Title: "
                            + currentUser.JobTitle);
                        sb.AppendLine("Department: "
                            + currentUser.Department);
                        sb.AppendLine("Location: "
                            + currentUser.OfficeLocation);
                        sb.AppendLine("Business phone: "
                            + currentUser.BusinessTelephoneNumber);
                        sb.AppendLine("Mobile phone: "
                            + currentUser.MobileTelephoneNumber);
                        //                    Debug.WriteLine(sb.ToString());
                        ret = sb.ToString();
                    }
                }
                return ret;
            }

            public string DisplayAccountInformation(Outlook.Application application)
            {

                // The Namespace Object (Session) has a collection of accounts.
                Outlook.Accounts accounts = application.Session.Accounts;

                // Concatenate a message with information about all accounts.
                StringBuilder builder = new StringBuilder();

                // Loop over all accounts and print detail account information.
                // All properties of the Account object are read-only.
                foreach (Outlook.Account account in accounts)
                {

                    // The DisplayName property represents the friendly name of the account.
                    builder.AppendFormat("DisplayName: {0}\n", account.DisplayName);

                    // The UserName property provides an account-based context to determine identity.
                    builder.AppendFormat("UserName: {0}\n", account.UserName);

                    // The SmtpAddress property provides the SMTP address for the account.
                    builder.AppendFormat("SmtpAddress: {0}\n", account.SmtpAddress);

                    // The AccountType property indicates the type of the account.
                    builder.Append("AccountType: ");
                    switch (account.AccountType)
                    {

                        case Outlook.OlAccountType.olExchange:
                            builder.AppendLine("Exchange");
                            break;

                        case Outlook.OlAccountType.olHttp:
                            builder.AppendLine("Http");
                            break;

                        case Outlook.OlAccountType.olImap:
                            builder.AppendLine("Imap");
                            break;

                        case Outlook.OlAccountType.olOtherAccount:
                            builder.AppendLine("Other");
                            break;

                        case Outlook.OlAccountType.olPop3:
                            builder.AppendLine("Pop3");
                            break;
                    }

                    builder.AppendLine();
                }

                // Display the account information.
    //            System.Windows.Forms.MessageBox.Show(builder.ToString());
                return builder.ToString();
            }*/
    /*
    public List<OutlookCalendarItem> GetAllCalendarItems(int NumberOfDays)
    {
        if (outlookObj == null)
            outlookObj = new Outlook.Application();

        List<OutlookCalendarItem> ret = new List<OutlookCalendarItem>();
        Microsoft.Office.Interop.Outlook.NameSpace mapiNamespace = null;
        Microsoft.Office.Interop.Outlook.MAPIFolder CalendarFolder = null;
        Microsoft.Office.Interop.Outlook.Items outlookCalendarItems = null;

        mapiNamespace = outlookObj.GetNamespace("MAPI");
        CalendarFolder = mapiNamespace.GetDefaultFolder(Microsoft.Office.Interop.Outlook.OlDefaultFolders.olFolderCalendar);
        outlookCalendarItems = CalendarFolder.Items;
        outlookCalendarItems.IncludeRecurrences = true;

        foreach (Microsoft.Office.Interop.Outlook.AppointmentItem item in outlookCalendarItems)
        {
#if CHECK_RECURING_ITEMS
            if (item.IsRecurring)
            {
                Microsoft.Office.Interop.Outlook.RecurrencePattern rp = item.GetRecurrencePattern();
                DateTime cur = new DateTime(DateTime.Now.Year, DateTime.Now.Month, DateTime.Now.Day, item.Start.Hour, item.Start.Minute, 0);
                for (int i=0;i<NumberOfDays;i++)
                {
                    cur.AddDays(1);
                    try
                    {
                        Microsoft.Office.Interop.Outlook.AppointmentItem recur = rp.GetOccurrence(cur);
//                            MessageBox.Show(recur.Subject + " -> " + cur.ToLongDateString());
                    }
                    catch
                    { }
                }
            }
            else
            {
//                    MessageBox.Show(item.Subject + " -> " + item.Start.ToLongDateString());
            }
#endif
            OutlookCalendarItem ci = new OutlookCalendarItem();
            ci.Start = item.Start;
            ci.Duration = item.Duration;
            ci.ResponseType = (OutlookResponseStatus)item.ResponseStatus;
            ret.Add(ci);
        }
        return ret;
    }
    */
}

