using BLFClient.Backend;
using IniParser.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace BLFClient
{
    //internal types, might not reflect forward type on the server
    public enum CallForwardingTypes
    {
        CallForwardNone = 0,
        CallForwardDestination = 1,
        CallForwardVoiceMail = 2,
    };

    /// <summary>
    /// Interaction logic for PhoneNumber.xaml
    /// </summary>
    public partial class PhoneNumber : UserControl
    {
        //private members
        PhoneStatusCodes CurrentStatus;
        PhoneStatusCodes[] CurrentStatusRange;

        string Extension = "";
        string UserName;
        string Prefix;
        string Email;
        string Note;
        int Row, Column;
        bool ShowName;
        bool ShowCanonical;
        bool IsRange;   // if it contains 10 numbers instead 1
        Object HiddenTooltip;
        //        int ConfigIndex = -1;    // it's like a SUID ( section unique ID )
        int FontIndex;
        long GUID, GUIDOwner;  // will be assigned to us by the manager

        public PhoneNumber(int X, int Y, PhoneNumberSetupSettings settings, long OwnerGUID)
        {
            InitializeComponent();

            CurrentStatusRange = new PhoneStatusCodes[10];
            HiddenTooltip = this.StatusText.ToolTip;

            //set some value to see how it looks like
            this.StatusText.Content = "";
            this.StatusRange.Visibility = Visibility.Collapsed;
            this.img_Forward.Visibility = Visibility.Collapsed;
            this.img_External.Visibility = Visibility.Collapsed;

            //            SetStatus(PhoneStatusCodes.PHONE_DOESNOT);    //default status
            CurrentStatus = PhoneStatusCodes.NumberOfStatusCodes2; //invalid state
            SetCoordinate(X, Y);
            //remove border if there is no border
            SetShowGrid(settings.ShowGrid);

            ShowName = settings.ShowName;

            ShowCanonical = settings.ShowCanonical;

            //update size
            if (settings.CellWidth > 0 && settings.CellHeight > 0)
                OnCellSizeChanged(settings.CellWidth, settings.CellHeight);

            //hide tooltip until we populate subscriber with data
            this.StatusText.ToolTip = null;

            IsRange = false;

            GUIDOwner = OwnerGUID;
            Globals.ExtensionManager.PhoneNumberAdd(this);
            Prefix = settings.Prefix;

            //data has not yet been loaded, this will init only a dummy look
            UpdateContextMenu();

            // prepare the UI for the case when the forwarding query reply will take a lot of time
            OnForwardingChange(null);
            //try to update forwarding status. This will either do a query with callback or set the UI updates directly
            Globals.ForwardManager.OnExtensionCreate(this);

            //implementation of drag and drop
            MouseLeave += new MouseEventHandler(OnMouseLeave);
            MouseEnter += new MouseEventHandler(OnMouseEnter);
            MouseUp += new MouseButtonEventHandler(OnMouseUp);
        }

        ~PhoneNumber()
        {
        }

        /// <summary>
        /// Remove ourself from reference lists
        /// </summary>
        public void Destroy()
        {
            //remove from owner
            //remove from manager
            if (GUID != 0)
            {
                Globals.ExtensionManager.PhoneNumberDelete(this);
                GUID = 0;
            }
            //destroy values
        }

        public void SetGUID(long newGUID)
        {
            GUID = newGUID;
        }

        public long GetGUID()
        {
            return GUID;
        }
        /*
                public void SetConfigIndex(int NewIndex)
                {
                    ConfigIndex = NewIndex;
                }*/

        public bool IsSubscriberRange()
        {
            return IsRange;
        }

        public void OnDelete()
        {
        }

        public void SetPrefix(string NewPrefix)
        {
            Prefix = NewPrefix;
        }

        public string GetPrefix()
        {
            return Prefix;
        }

        public string GetEmail()
        {
            return Email;
        }

        public string GetNote()
        {
            return Note;
        }

        public string GetUserName()
        {
            return UserName;
        }

        public void SetIsRange(bool pIsRange)
        {
            IsRange = pIsRange;
            //disable tooltip
            if (IsRange == true)
            {
                //hide the label with simple extension number and tooltip
                OnForwardingChange(null);
                //make it override the label
                this.StatusText.VerticalContentAlignment = VerticalAlignment.Top;
                this.StatusRange.Visibility = Visibility.Visible;
            }
            else
            {
                this.StatusText.VerticalContentAlignment = VerticalAlignment.Center;
                this.StatusRange.Visibility = Visibility.Collapsed;
            }

            UpdateTooltipContent();
            UpdateContextMenu();

            //for each sub extension try to fetch the status
            Globals.ForwardManager.OnExtensionCreate(this);
        }

        public void SetEmail(string NewEmail)
        {
            Email = NewEmail;
            UpdateTooltipContent();
        }

        public void SetNote(string NewNote)
        {
            Note = NewNote;
            UpdateTooltipContent();
        }

        public void OnToggleShowName()
        {
            if (ShowName == false)
            {
                ShowName = true;
                SetName(UserName);
            }
            else
            {
                ShowName = false;
                SetExtension(Extension);
            }
        }

        public void OnToggleShowCanonical()
        {
            if (ShowCanonical == false)
                ShowCanonical = true;
            else
                ShowCanonical = false;
            SetExtension(Extension);
        }

        public void OnFontSettingChanged(FontSettings fs)
        {
            if (fs == null)
                return;
            FontIndex = fs.Index;
            this.StatusText.FontFamily = new FontFamily(fs.FaceName);
            if (fs.FontSize != 0)
                this.StatusText.FontSize = fs.FontSize;
            if (fs.FontWeight == 1)
                this.StatusText.FontWeight = FontWeights.Bold;
            else
                this.StatusText.FontWeight = FontWeights.Normal;
            if (fs.Italic == 1)
                this.StatusText.FontStyle = FontStyles.Italic;
            else
                this.StatusText.FontStyle = FontStyles.Normal;
        }

        public void OnPrefixChanged(string NewPrefix)
        {
            Prefix = NewPrefix;
            //in case prefix changed, we update cannonical view also
            SetExtension(Extension);
        }

        public static string GetExtensionFromFullNumberStr(string FullNumber, bool LogError = true)
        {
            int Splitpos = FullNumber.IndexOf('-');
            if (Splitpos > 0)
                return FullNumber.Substring(Splitpos + 1);
            return FullNumber;
        }

        public static string GetExtensionFromFullNumber(string FullNumber, bool LogError = true)
        {
            string ret = "";
            try
            {
                ret = GetExtensionFromFullNumberStr(FullNumber);
            }
            catch (Exception e)
            {
                if (LogError)
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Expecting prefix-extension number format, but got :  [" + FullNumber + " Exception : " + e.ToString());
            }
            return ret;
        }

        public static string GetPrefixFromFullNumber(string FullNumber)
        {
            if (FullNumber.Contains("-"))
            {
                string[] FullNumberParts = FullNumber.Split('-');
                return FullNumberParts[0];
            }
            return "";
        }

        /// <summary>
        /// Load from some source settings of this cell
        /// </summary>
        public bool Load(string IndexCardName, int ConfigIndex)
        {
            KeyDataCollection IndexCardCollection = Globals.Config.GetConfigSection(IndexCardName);
            if (IndexCardCollection == null)
                return false; // could not find this index card saved as config
            int DataSectionsFound = 0;
            int x = 0, y = 0;
            if (IndexCardCollection["Phone #" + ConfigIndex] != null)
                DataSectionsFound++;
            if (IndexCardCollection["Name #" + ConfigIndex] != null)
                DataSectionsFound++;
            if (IndexCardCollection["Number #" + ConfigIndex] != null)
                DataSectionsFound++;
            if (IndexCardCollection["Email #" + ConfigIndex] != null)
                DataSectionsFound++;
            if (IndexCardCollection["Comment #" + ConfigIndex] != null)
                DataSectionsFound++;
            if (IndexCardCollection["Position x #" + ConfigIndex] != null)
            {
                DataSectionsFound++;
                x = Globals.Config.GetConfigInt(IndexCardName, "Position x #" + ConfigIndex, 0);
            }
            if (IndexCardCollection["Position y #" + ConfigIndex] != null)
            {
                DataSectionsFound++;
                y = Globals.Config.GetConfigInt(IndexCardName, "Position y #" + ConfigIndex, 0);
            }
            if (IndexCardCollection["Font #" + ConfigIndex] != null)
            {
                FontIndex = Globals.Config.GetConfigInt(IndexCardName, "Font #" + ConfigIndex, 0);
                DataSectionsFound++;
            }
            //if we found enough data to initialize a phone number..do it
            if (DataSectionsFound == 8)
            {
                SetCoordinate(x, y);
                SetEmail(IndexCardCollection["Email #" + ConfigIndex]);
                SetNote(IndexCardCollection["Comment #" + ConfigIndex]);
                SetName(IndexCardCollection["Name #" + ConfigIndex]);
                SetPrefix(GetPrefixFromFullNumber(IndexCardCollection["Number #" + ConfigIndex]));
                SetExtension(GetExtensionFromFullNumber(IndexCardCollection["Number #" + ConfigIndex]));
                if (GetExtension().Length == 0)
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Expecting prefix-extension number format, but got :  [" + IndexCardName + "][" + "Number #" + ConfigIndex + "]=" + IndexCardCollection["Number #" + ConfigIndex]);
                OnFontSettingChanged(Globals.FontManager.GetFontSettingByIndex(FontIndex));
            }
            else
                return false;
            //we found all data to load this phone/"extension"
            return true;
        }

        /// <summary>
        /// Load from some source settings of this cell
        /// </summary>
        public bool LoadRange(string IndexCardName, int ConfigIndex)
        {
            KeyDataCollection IndexCardCollection = Globals.Config.GetConfigSection(IndexCardName);
            if (IndexCardCollection == null)
                return false; // could not find this index card saved as config
            int DataSectionsFound = 0;
            int x = 0, y = 0;
            if (IndexCardCollection["Group #" + ConfigIndex] != null)
                DataSectionsFound++;
            if (IndexCardCollection["GroupNumber #" + ConfigIndex] != null)
                DataSectionsFound++;
            if (IndexCardCollection["GroupPosition x #" + ConfigIndex] != null)
            {
                DataSectionsFound++;
                x = Globals.Config.GetConfigInt(IndexCardName, "GroupPosition x #" + ConfigIndex, 0);
            }
            if (IndexCardCollection["GroupPosition y #" + ConfigIndex] != null)
            {
                DataSectionsFound++;
                y = Globals.Config.GetConfigInt(IndexCardName, "GroupPosition y #" + ConfigIndex, 0);
            }
            if (IndexCardCollection["GroupFont #" + ConfigIndex] != null)
            {
                FontIndex = Globals.Config.GetConfigInt(IndexCardName, "GroupFont #" + ConfigIndex, 0);
                DataSectionsFound++;
            }
            //if we found enough data to initialize a phone number..do it
            if (DataSectionsFound == 5)
            {
                SetCoordinate(x, y);
                SetIsRange(true);
                string lExtension;
                if (IndexCardCollection["GroupNumber #" + ConfigIndex].Contains("-"))
                {
                    string[] FullNumberParts = IndexCardCollection["GroupNumber #" + ConfigIndex].Split('-');
                    string lPrefix = FullNumberParts[0];
                    SetPrefix(lPrefix);
                    lExtension = FullNumberParts[1];
                }
                else
                    lExtension = IndexCardCollection["GroupNumber #" + ConfigIndex];
                try
                {
                    SetExtension(lExtension);
                }
                catch (Exception e)
                {
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Expecting prefix-extension number format, but got :  [" + IndexCardName + "][" + "Number #" + ConfigIndex + "]=" + IndexCardCollection["Number #" + ConfigIndex] + " Exception : " + e.ToString());
                }
                OnFontSettingChanged(Globals.FontManager.GetFontSettingByIndex(FontIndex));
            }
            else
                return false;
            return true;
        }

        public void Save(string IndexCardName, int pConfigIndex)
        {
            //            ConfigIndex = pConfigIndex;
            if (IsRange == true)
                SaveRange(IndexCardName, pConfigIndex);
            else
                SaveExtension(IndexCardName, pConfigIndex);
        }

        private void SaveRange(string IndexCardName, int pConfigIndex)
        {
            //when we get saved, the whole index card gets saved, there is no need to delete old data
            Globals.Config.SetConfig(IndexCardName, "Group #" + pConfigIndex, "//////////////");
            if (Prefix != null && Prefix.Length > 0)
            {
                string CompositeNumber = Prefix + "-" + Extension.ToString();
                Globals.Config.SetConfig(IndexCardName, "GroupNumber #" + pConfigIndex, CompositeNumber);
            }
            else
                Globals.Config.SetConfig(IndexCardName, "GroupNumber #" + pConfigIndex, Extension.ToString());
            Globals.Config.SetConfig(IndexCardName, "GroupPosition x #" + pConfigIndex, GetX().ToString());
            Globals.Config.SetConfig(IndexCardName, "GroupPosition y #" + pConfigIndex, GetY().ToString());
            Globals.Config.SetConfig(IndexCardName, "GroupFont #" + pConfigIndex, FontIndex.ToString());
        }

        private void SaveExtension(string IndexCardName, int pConfigIndex)
        {
            Globals.Config.SetConfig(IndexCardName, "Phone #" + pConfigIndex, "//////////////");
            Globals.Config.SetConfig(IndexCardName, "Name #" + pConfigIndex, UserName);
            if (Prefix != null && Prefix.Length > 0)
            {
                string CompositeNumber = Prefix + "-" + Extension.ToString();
                Globals.Config.SetConfig(IndexCardName, "Number #" + pConfigIndex, CompositeNumber);
            }
            else
                Globals.Config.SetConfig(IndexCardName, "Number #" + pConfigIndex, Extension.ToString());
            Globals.Config.SetConfig(IndexCardName, "Email #" + pConfigIndex, Email);
            Globals.Config.SetConfig(IndexCardName, "Comment #" + pConfigIndex, Note);
            Globals.Config.SetConfig(IndexCardName, "Position x #" + pConfigIndex, GetX().ToString());
            Globals.Config.SetConfig(IndexCardName, "Position y #" + pConfigIndex, GetY().ToString());
            Globals.Config.SetConfig(IndexCardName, "Font #" + pConfigIndex, FontIndex.ToString());
        }
        /*
                public void DeleteSave(string IndexCardName)
                {
                    //we never got saved or loaded, there is no need to delete
                    if (ConfigIndex < 0)
                        return;
                    Globals.Config.RemoveConfig(IndexCardName, "Phone #" + ConfigIndex);
                    Globals.Config.RemoveConfig(IndexCardName, "Name #" + ConfigIndex);
                    Globals.Config.RemoveConfig(IndexCardName, "Number #" + ConfigIndex);
                    Globals.Config.RemoveConfig(IndexCardName, "Email #" + ConfigIndex);
                    Globals.Config.RemoveConfig(IndexCardName, "Comment #" + ConfigIndex);
                    Globals.Config.RemoveConfig(IndexCardName, "Position x #" + ConfigIndex);
                    Globals.Config.RemoveConfig(IndexCardName, "Position y #" + ConfigIndex);
                    Globals.Config.RemoveConfig(IndexCardName, "Font #" + ConfigIndex);
                }*/

        /// <summary>
        /// Change the status of this phone.It will have different color
        /// </summary>
        /// <param name="NewStatus"></param>
        public void SetStatus(PhoneStatusCodes NewStatus, string pExtension = "")
        {
            //unknown status ? Bail out
            if (NewStatus >= PhoneStatusCodes.NumberOfStatusCodes2)
                return;
            if (GetExtension() == "" && NewStatus == PhoneStatusCodes.PHONE_DOESNOT)
                return;
            //ranges and non ranges are handled differently
            if (IsRange == false)
            {
                CurrentStatus = NewStatus;
                this.CellBorder.Background = StatusColorEditor.GetStatusColor(NewStatus);
                UpdateTooltipContent();
                if (NewStatus == PhoneStatusCodes.PHONE_EXTERNAL)
                    img_External.Visibility = Visibility.Visible;
                else
                    img_External.Visibility = Visibility.Collapsed;
            }
            else
            {
                if (pExtension != "")
                {
                    int LastDigit = pExtension[pExtension.Length - 1] - '0';
                    SetRangeStatus(LastDigit, NewStatus);
                }
                else
                {
                    //refresh colors
                    for(int i=0;i<9;i++)
                        SetRangeStatus(i, CurrentStatusRange[i]);
                }
            }
        }

        public PhoneStatusCodes GetStatus()
        {
            return CurrentStatus;
        }

        private void SetRangeStatus(int Index, PhoneStatusCodes NewStatus)
        {
            if (Index < 0 || Index > 9)
                return;
            if (NewStatus < PhoneStatusCodes.NumberOfStatusCodes)
            {
                if (Index == 0)
                    this.StatusText0.Background = StatusColorEditor.GetStatusColor(NewStatus);
                else if (Index == 1)
                    this.StatusText1.Background = StatusColorEditor.GetStatusColor(NewStatus);
                else if (Index == 2)
                    this.StatusText2.Background = StatusColorEditor.GetStatusColor(NewStatus);
                else if (Index == 3)
                    this.StatusText3.Background = StatusColorEditor.GetStatusColor(NewStatus);
                else if (Index == 4)
                    this.StatusText4.Background = StatusColorEditor.GetStatusColor(NewStatus);
                else if (Index == 5)
                    this.StatusText5.Background = StatusColorEditor.GetStatusColor(NewStatus);
                else if (Index == 6)
                    this.StatusText6.Background = StatusColorEditor.GetStatusColor(NewStatus);
                else if (Index == 7)
                    this.StatusText7.Background = StatusColorEditor.GetStatusColor(NewStatus);
                else if (Index == 8)
                    this.StatusText8.Background = StatusColorEditor.GetStatusColor(NewStatus);
                else if (Index == 9)
                    this.StatusText9.Background = StatusColorEditor.GetStatusColor(NewStatus);
            }
            //unknown status ? Bail out
            else
                return;
            CurrentStatusRange[Index] = NewStatus;
        }

        /// <summary>
        /// This is a global event passed from main window to all children
        /// </summary>
        public void OnStatusColorChanged()
        {
            SetStatus(CurrentStatus);
        }

        /// <summary>
        /// In case we swap places of 2 different grid cells, we will only spawn their location
        /// </summary>
        /// <param name="pRow"></param>
        /// <param name="pCol"></param>
        public void SetCoordinate(int x, int y)
        {
            Column = x;
            Row = y;
        }

        private void UpdateContextMenu()
        {
            //reset it just in case
            ContextMenu PrevMenu = this.ContextMenu;
            this.ContextMenu = Resources["contextMenuNew"] as ContextMenu;
            if (GetExtension().Length > 0)
            {
                if (IsRange == false)
                {
                    if (Globals.ConnectionManager.IsConnected() == true)
                        this.ContextMenu = Resources["contextMenuUser"] as ContextMenu;
                    else
                        this.ContextMenu = Resources["contextMenuUserNotConnected"] as ContextMenu;
                }
                else
                {
                    if (Globals.ConnectionManager.IsConnected() == true)
                        this.ContextMenu = Resources["contextMenuRange"] as ContextMenu;
                    else
                        this.ContextMenu = Resources["contextMenuRangeNotConnected"] as ContextMenu;
                }
            }
            if (PrevMenu != this.ContextMenu)
                Globals.MultilangManager.TranslateUIComponent(this.ContextMenu);
        }

        private void UpdateTooltipContent()
        {
            if (IsRange == false && Extension.Length > 0)
            {
                this.StatusText.ToolTip = HiddenTooltip;
                this.TooltipName.Text = UserName;
                this.TooltipNote.Text = Note;
                if (CurrentStatus == PhoneStatusCodes.Busy)
                    this.TooltipStatus.Text = "Busy";
                else if (CurrentStatus == PhoneStatusCodes.PHONE_EXTERNAL)
                    this.TooltipStatus.Text = "Busy(external)";
                else if (CurrentStatus == PhoneStatusCodes.Idle)
                    this.TooltipStatus.Text = "Idle";
                else if (CurrentStatus == PhoneStatusCodes.Ringing)
                    this.TooltipStatus.Text = "Ringing";
                else if (CurrentStatus == PhoneStatusCodes.PHONE_DOESNOT)
                    this.TooltipStatus.Text = "Not existing";
                else if (CurrentStatus == PhoneStatusCodes.OutOfService)
                    this.TooltipStatus.Text = "Out of service";
                /*               if (Email != null && Email.Length > 0)
                                    Task.Run(() => { BackgroundUpdateAvailability(); });
                                else
                                    TooltipAvail.Text = ""; */
                if (img_Forward.Visibility == Visibility.Visible)
                {
                    ForwardStatusStore fs = Globals.ForwardManager.ForwardStatusGet(GetExtension());
                    ToolTipForward.Visibility = Visibility.Visible;
                    t_TooltipFrw.Text = "Forwarding to " + fs.DestinationForward.ToString();
                }
                else
                {
                    ToolTipForward.Visibility = Visibility.Collapsed;
                }
            }
            else
                this.StatusText.ToolTip = null;
        }
        /*
                private void BackgroundUpdateAvailability()
                {
                    TooltipAvail.Text = "Avail";
                    List<OutlookCalendarItem> ci = Globals.OutlookService.GetAppointmentsInRange(Email, 1);
                    foreach (var item in ci)
                    {
                        //if this item ended before our workday, ignore it
                        if (item.Start.Hour * 60 + item.Start.Minute + item.Duration < DateTime.Now.Hour * 60 + DateTime.Now.Minute)
                            continue;
                        //if this item did not start yet, ignore
                        if (item.Start.Hour * 60 + item.Start.Minute > DateTime.Now.Hour * 60 + DateTime.Now.Minute)
                            continue;
                        //Seems like we are busy, update tooltip and bail out
                        TooltipAvail.Text = "Busy";
                        return;
                    }
                }*/

        public void OnAbsenceStatusUpdate(bool Available)
        {
            if (Available == true)
                TooltipAvail.Text = "Avail";
            else
                TooltipAvail.Text = "Busy";
        }

        /// <summary>
        /// Extension is the actual phone number people need to dial
        /// </summary>
        /// <param name="Extension"></param>
        public void SetExtension(string pExtension)
        {
            if (pExtension == null)
                pExtension = "";
            //some sort of extension
            Extension = pExtension;

            if (ShowName == false || UserName == null || UserName.Length == 0)
            {
                if (ShowCanonical == false)
                    this.StatusText.Content = Extension;
                else
                {
                    if(Prefix.Length > 0 )
                        this.StatusText.Content = Prefix + "-" + Extension;
                    else
                        this.StatusText.Content = Extension;
                }
            }

            //query the status for this new extension
            if (IsRange == false)
            {
                SetStatus(Globals.ExtensionManager.GetCachedStatus(GetExtension())); // get extension status if available right now. Else we will get updated later
                Globals.ForwardManager.OnExtensionCreate(this); // check forward status for this extension
            }
            else
            {
                //always show extension ( no prefix, no name .. )
                this.StatusText.Content = Extension;
                for (long i = 0; i < 10; i++)
                {
                    string FullExtension = GetExtension() + i.ToString();
                    SetStatus(Globals.ExtensionManager.GetCachedStatus(FullExtension), i.ToString()); // get extension status if available right now. Else we will get updated later
                }
            }
            UpdateContextMenu();
            UpdateTooltipContent();
        }

        public void SetName(string pName)
        {
            //assign a new user name
            UserName = pName;

            UpdateTooltipContent();

            //update visually
            if (ShowName == true && IsRange == false)
            {
                if (UserName != null && UserName.Length > 0)
                    this.StatusText.Content = UserName;
                else if (this.StatusText.Content.ToString().Length == 0)
                    SetExtension(GetExtension());
            }
        }

        /// <summary>
        /// Get the actual phone number
        /// </summary>
        /// <returns></returns>
        public string GetExtension()
        {
            return Extension;
        }

        /// <summary>
        /// Get Column where this phone number is located in the grid
        /// </summary>
        /// <returns></returns>
        public int GetX()
        {
            return Column;
        }

        /// <summary>
        /// Get Row where this phone number is located in the grid
        /// </summary>
        /// <returns></returns>
        public int GetY()
        {
            return Row;
        }

        public void SetShowGrid(bool show)
        {
            if (show == true)
            {
                this.CellBorder.BorderBrush = Brushes.Black;
                //                this.CellBorder.BorderThickness = new Thickness(1);
            }
            else
            {
                this.CellBorder.BorderBrush = Brushes.Transparent;
                //               this.CellBorder.BorderBrush = StatusText.Background;
                //                this.CellBorder.BorderThickness = new Thickness(0);
            }
        }

        public void OnCellSizeChanged(double NewWidth, double NewHeight)
        {
            this.TheControl.Width = NewWidth;
            this.TheControl.Height = NewHeight;
        }

        private void Click_Context_DeletePhone(object sender, RoutedEventArgs e)
        {
            if (App.Current != null && App.Current.MainWindow != null)
                (App.Current.MainWindow as MainWindow).PhoneNumberDeleteAddNew(GetX(), GetY());
        }

        private void Click_Context_CallForward(object sender, RoutedEventArgs e)
        {
            CallForwardingEdit t = new CallForwardingEdit(GetExtension());
            t.Show();
        }

        private void Click_Context_NewPhone(object sender, RoutedEventArgs e)
        {
            var cw = new PhoneNumberNew(this);
            cw.ShowInTaskbar = false;
            cw.Owner = System.Windows.Application.Current.MainWindow;
            cw.Show();
        }

        private void Click_Context_NewPhoneRange(object sender, RoutedEventArgs e)
        {
            var cw = new PhoneNumberNewRange(this);
            cw.ShowInTaskbar = false;
            cw.Owner = System.Windows.Application.Current.MainWindow;
            cw.Show();
        }

        public void OnConnectionChanged(bool Connected)
        {
            // this cell is not yet initialized. Leave designer choices intact until it gets used
            if (GetExtension() == "" && GetStatus() == PhoneStatusCodes.NumberOfStatusCodes2)
                return;
            SetStatus(PhoneStatusCodes.PHONE_DOESNOT);
            UpdateContextMenu();
            UpdateTooltipContent(); // in case we want to show the connection status in proper color
        }

        public void OnForwardingChange(ForwardStatusStore fs)
        {
            //update the extension to show an arrow
            if (fs == null || fs.ForwardType == CallForwardingTypes.CallForwardNone)
                img_Forward.Visibility = Visibility.Collapsed;
            else
                img_Forward.Visibility = Visibility.Visible;

            //update tooltip
            UpdateTooltipContent();
        }

        private void MouseLeftDownHandler(object sender, MouseButtonEventArgs e)
        {
            if (e.ClickCount == 2)
            {
                if (GetExtension().Length == 0)
                    return;
                if (GetStatus() != PhoneStatusCodes.Idle)
                    return;
                //send call to ACWin
                if (Globals.AcWinManager != null)
                {
                    Globals.AcWinManager.MakeCall(GetPrefix(), GetExtension());
//                    if (Globals.AcWinManager.MakeCall(GetPrefix(), GetExtension()) == false)
//                        Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Could not create call through ACWin");
                }
                else if (Globals.Config.GetConfig("Options", "Optiset", "NO") == "YES")
                {
                    //get the extension only from a full phone number
                    string OptisetExtenstion = PhoneNumber.GetExtensionFromFullNumberStr(Globals.Config.GetConfig("Options", "OptisetExtension", ""));
                    if (OptisetExtenstion == null || OptisetExtenstion.Length == 0)
                        return;

                    //optiset can't call itself
                    string Extensionstr = GetExtension().ToString();
                    if (OptisetExtenstion == Extensionstr)
                        return;

                    long CurrentCallId = Globals.ExtensionManager.GetCallId(GetExtension());
                    if (CurrentCallId != 0)
                    {
                        Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Extension " + GetExtension().ToString() + " is already in a call. Can't call him");
                        return;
                    }

                    // if this extension is not in a call, than make a direct call
                    long OptisetCallId = Globals.ExtensionManager.GetOptisetCallId();
                    if (OptisetCallId == 0) // if this target extension does not have an ongoing call yet
                    {
                        Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "CallingDevice=" + OptisetExtenstion + ", CalledDirectoryNumber=" + Extensionstr);
                        //MakeCall(m_strOptisetExtension, m_strSelectedExtension, make_call_doNotPrompt);
                        NetworkClientBuildPacket.MakeCall(OptisetExtenstion, Extensionstr, MakeCallPrompTypes.make_call_doNotPrompt);
                        return;
                    }
                    if (OptisetCallId != 0)
                    {
                        //Client.ConsultationCall(sDevice.GetBuffer(sDevice.GetLength()), strConsultedDevice.GetBuffer(strConsultedDevice.GetLength()), strExistingCallID.GetBuffer(strExistingCallID.GetLength()), "", 0);
                        NetworkClientBuildPacket.ConsultationCall(OptisetExtenstion, Extensionstr, OptisetCallId.ToString(), "", 0);
                    }
                }
            }
        }

        private void MouseRightDownHandler(object sender, MouseButtonEventArgs e)
        {
            if (e.ClickCount == 2)
            {
                long OptisetCallId = Globals.ExtensionManager.GetOptisetCallId();
            }
        }

        private void OnMouseLeave(object Sender, RoutedEventArgs e)
        {
            if (Mouse.LeftButton == MouseButtonState.Pressed)
                ((MainWindow)App.Current.MainWindow).OnPhoneNumberDrag(this, null);
        }

        private void OnMouseEnter(object Sender, RoutedEventArgs e)
        {
            if (Mouse.LeftButton == MouseButtonState.Pressed)
                ((MainWindow)App.Current.MainWindow).OnPhoneNumberDrag(null, this);
        }

        private void OnMouseUp(object Sender, MouseButtonEventArgs e)
        {
            if (Mouse.LeftButton == MouseButtonState.Released)
                ((MainWindow)App.Current.MainWindow).OnPhoneNumberDrag(null, null);
        }
    }
}
