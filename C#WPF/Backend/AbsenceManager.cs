using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Timers;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;
using System.Xml;

namespace BLFClient.Backend
{
    /*
     * Periodically check absence status
     * Update UI with absence status ( if changed )
     * By default it will use outlook
     * If outlook is not available, it will request an Exchange email / passw
    */
    public class AbsenceManager
    {
        bool IsUIVisible;   // no need to update UI if it is not visible
        System.Threading.Timer UpdateTimer;
        System.Timers.Timer TooltipUpdateTimer;
        DateTime LastUpdated;
        string MonitoredEmail;

        public AbsenceManager()
        {
            MonitoredEmail = "";
            UpdateTimer = new System.Threading.Timer(PeriodicStatusUpdate, null, System.Threading.Timeout.Infinite, 1);
            LastUpdated = DateTime.Now;

            //create a timer to periodically query extension forwarding status and issue callbacks to all cell cards on status change
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Async AbsenceStatus manager has started");
        }

        ~AbsenceManager()
        {
            ShutDown();
        }

        public void ShutDown()
        {
            //stop the times
            UpdateTimer.Dispose();
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Async AbsenceStatus manager has exited");
        }

        //should wait for full load and connection to the server
        public void Load()
        {
            Task mytask = Task.Run(() => { _Load(); });
        }

        /// <summary>
        /// Delay the loading of this manager until we have a server connection that we can query
        /// </summary>
        private void _Load()
        {
            // wait for index cards and extension to load up. Also wait for a connection we can query
            while (Globals.IndexCardsLoaded == false)
                Thread.Sleep(100);

            int UpdateIntervalMinutes = Globals.Config.GetConfigInt("AbsenceInfo", "RefreshTime", 5);
            if (UpdateIntervalMinutes == 0)
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Async tooltip AbsenceStatus manager is disabled due to 0 update period");
                return;
            }

            while (Globals.OutlookService.IsConnected() == false)
                Thread.Sleep(100);

            TooltipUpdateTimer = new System.Timers.Timer(UpdateIntervalMinutes * 60 * 1000);
            TooltipUpdateTimer.Enabled = true;
            TooltipUpdateTimer.Elapsed += new ElapsedEventHandler(UpdateExtensionTooltips);
            //create a timer to periodically query extension forwarding status and issue callbacks to all cell cards on status change
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Async tooltip AbsenceStatus manager has started");
        }

        /// <summary>
        /// Called when the user clicks on an extension(phone number). Since the query might take up to 2 minutes, the response will be given by a callback function
        /// </summary>
        /// <param name="NewEmail"></param>
        public void SetMonitoredEmail(string NewEmail)
        {
            string OldEmail = MonitoredEmail;
            MonitoredEmail = NewEmail;
            if (OldEmail != NewEmail)
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Async AbsenceStatus manager  : Started monitoring new email : " + NewEmail + " Old was : " + OldEmail);
            if (NewEmail != null && NewEmail.Length > 0 && (OldEmail == null || NewEmail.CompareTo(OldEmail) != 0))
                Task.Factory.StartNew(() => _UpdateUI(true)); // just run it once in a new thread ( since it lags a lot )
            else
                ClearGridContent();
        }

        /// <summary>
        /// Periodically check if monitored email changed and we need to update the UI with new apointments
        /// </summary>
        /// <param name="source"></param>
        private void PeriodicStatusUpdate(object source)
        {
            if (IsUIVisible == false)
            {
                UpdateTimer.Change(System.Threading.Timeout.Infinite, 1); // how did we even get here ?
                return;
            }
            //update the UI with current selected user
            _UpdateUI();
        }

        /// <summary>
        /// Add a new row in the UI window where a new apointment entry can be stored
        /// </summary>
        /// <param name="Date"></param>
        /// <param name="Duration"></param>
        /// <param name="Status"></param>
        /// <param name="IsHeader"></param>
        private void AddGridRow(string Date, string Duration, string Status, bool IsHeader = false)
        {
            if (App.Current != null)
                App.Current.Dispatcher.Invoke(DispatcherPriority.Normal, (Action)(() =>
                {
                    MainWindow MainObject = (MainWindow)App.Current.MainWindow;

                    if (MainObject == null)
                        return;

                    int RowCount = MainObject.AbsenceView.DataGrid.RowDefinitions.Count;
                    //add a new row where we will store data
                    RowDefinition rd = new RowDefinition();
                    rd.Height = new System.Windows.GridLength(25);
                    MainObject.AbsenceView.DataGrid.RowDefinitions.Add(rd);

                    //date
                    Label lDate = new Label();
                    if (IsHeader == true)
                        lDate.Background = System.Windows.Media.Brushes.LightGray;
                    lDate.Content = Date;
                    Grid.SetColumn(lDate, 0);
                    Grid.SetRow(lDate, RowCount);
                    MainObject.AbsenceView.DataGrid.Children.Add(lDate);

                    //time
                    Label lDur = new Label();
                    if (IsHeader == true)
                        lDur.Background = System.Windows.Media.Brushes.LightGray;
                    lDur.Content = Duration;
                    Grid.SetColumn(lDur, 1);
                    Grid.SetRow(lDur, RowCount);
                    MainObject.AbsenceView.DataGrid.Children.Add(lDur);

                    //status
                    Label lStatus = new Label();
                    if (IsHeader == true)
                        lStatus.Background = System.Windows.Media.Brushes.LightGray;
                    lStatus.Content = Status;
                    Grid.SetColumn(lStatus, 2);
                    Grid.SetRow(lStatus, RowCount);
                    MainObject.AbsenceView.DataGrid.Children.Add(lStatus);
                }));
        }

        private void AddGridRow(OutlookCalendarItem item)
        {
            AddGridRow(item.Start.ToString(), item.Duration.ToString(), Enum.GetName(item.ResponseType.GetType(), item.ResponseType));
        }

        /// <summary>
        /// Clear UI content so a new monitored email entries can populate it
        /// </summary>
        private void ClearGridContent()
        {
            if (App.Current != null)
                App.Current.Dispatcher.Invoke(DispatcherPriority.Normal, (Action)(() =>
                {
                    MainWindow MainObject = (MainWindow)App.Current.MainWindow;

                    if (MainObject == null)
                        return;

                    MainObject.AbsenceView.DataGrid.Children.Clear();
                    MainObject.AbsenceView.DataGrid.RowDefinitions.Clear();
                    AddGridRow(Globals.MultilangManager.GetTranslation("Date"), Globals.MultilangManager.GetTranslation("Time"), Globals.MultilangManager.GetTranslation("Status"), true);
                }));
        }


        public void UpdateExtensionTooltipAsync(string Email)
        {
            Task mytask = Task.Run(() => { UpdateExtensionTooltip(Email); });
        }

        private void UpdateExtensionTooltip(string Email)
        {
            List<OutlookCalendarItem> ci = Globals.OutlookService.GetAppointmentsInRange(Email, 1);
            bool IsEmailUserAvailable = true;
            foreach (var item in ci)
            {
                //if this item ended before our workday, ignore it
                if (item.Start.Hour * 60 + item.Start.Minute + item.Duration < DateTime.Now.Hour * 60 + DateTime.Now.Minute)
                    continue;
                //if this item did not start yet, ignore
                if (item.Start.Hour * 60 + item.Start.Minute > DateTime.Now.Hour * 60 + DateTime.Now.Minute)
                    continue;
                //Seems like we are busy, update tooltip and bail out
                IsEmailUserAvailable = false;
                Globals.ExtensionManager.OnAbsenceStatusUpdate(Email, IsEmailUserAvailable);
            }
        }

        /// <summary>
        /// Periodically check all possible emails and update their availability in the tooltips
        /// </summary>
        /// <param name="source"></param>
        /// <param name="arg"></param>
        //       int i = 0;
        private void UpdateExtensionTooltips(object source, ElapsedEventArgs arg)
        {
            //nothing to be done for now
            if (Globals.OutlookService.IsConnected() == false)
                return;

            TooltipUpdateTimer.Stop();// in case network buffer gets full, the function might block and threadpool might call us multiple times
/*
            Application.Current.Dispatcher.Invoke((Action)delegate {             
                if( i % 5 == 0)
                    new NotificationTaskBar("Test" + i.ToString()).Show();
                i++;
            });/**/

            try
            {
                // phone number tooltips should show if user is busy or not
                HashSet<string> emails = Globals.ExtensionManager.GetUniqueEmails();
                foreach (string Email in emails)
                    UpdateExtensionTooltip(Email);
            }
            catch(Exception e)
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Could not update absence status of tooltips. Error : " + e.ToString());
            }
            TooltipUpdateTimer.Start();
        }

        /// <summary>
        /// Periodically called to update the absence manage window
        /// </summary>
        /// <param name="forced"></param>
        private void _UpdateUI(bool forced=false)
        {
            if (IsUIVisible == false)
                return;

            //do not spam updates due to user clicking repeatadly
            if (forced == false && DateTime.Now.Subtract(LastUpdated).TotalSeconds < Globals.Config.GetConfigInt("AbsenceInfo", "RefreshTime", 5*60))
                return;
            LastUpdated = DateTime.Now;
            //empty old list
            ClearGridContent();
            //if we have nothing to monitor at this point
            if (MonitoredEmail == null || MonitoredEmail.Length == 0)
                return;
            //mark list as loading
            //query if our status changed in outlook
            int DaysToCheck = Globals.Config.GetConfigInt("AbsenceInfo", "NumCheckedDays", 2);
            if (DaysToCheck == 0)
                DaysToCheck = 2;
            string Start = Globals.Config.GetConfig("AbsenceInfo", "WorkDayStart", "08:00 AM");
            string End = Globals.Config.GetConfig("AbsenceInfo", "WorkDayEnd", "05:00 PM");
            DateTime StartTime =  DateTime.ParseExact(Start, "hh:mm tt", CultureInfo.InvariantCulture);
            DateTime EndTime = DateTime.ParseExact(End, "hh:mm tt", CultureInfo.InvariantCulture);
            List<OutlookCalendarItem> ci = Globals.OutlookService.GetAppointmentsInRange(MonitoredEmail,DaysToCheck);
            foreach(var item in ci)
            {
                //if this item ended before our workday, ignore it
                if (item.Start.Hour * 60 + item.Start.Minute + item.Duration < StartTime.Hour * 60 + StartTime.Minute)
                    continue;
                //if this item starts after our workday, ignore it
                if (item.Start.Hour * 60 + item.Start.Minute > EndTime.Hour * 60 + StartTime.Minute)
                    continue;
                //add it to our list of apointments
                AddGridRow(item);
            }
        }

        public void ShowUI(bool Enabled)
        {
            IsUIVisible = Enabled;
            if (Enabled)
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Async AbsenceStatus manager  : Show UI window that monitors an email status");

                //check new settings ( if anything changed )
                int UpdateIntervalMinutes = Globals.Config.GetConfigInt("AbsenceInfo", "RefreshTime", 5);
                //only enable updates if we have no update interval
                if (UpdateIntervalMinutes > 0)
                    UpdateTimer.Change(0, UpdateIntervalMinutes * 60 * 1000);
                else
                    Task.Factory.StartNew(() => _UpdateUI(true)); // just run it once in a new thread ( since it lags a lot )
            }
            else
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Async AbsenceStatus manager  : Hide UI window that monitors an email status");
                UpdateTimer.Change(System.Threading.Timeout.Infinite, 1);
            }
        }
    }
}
