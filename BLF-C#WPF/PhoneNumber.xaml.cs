using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
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
    /// <summary>
    /// Interaction logic for PhoneNumber.xaml
    /// </summary>
    public partial class PhoneNumber : UserControl
    {
        //private members
        PhoneStatusCodes CurrentStatus;
        PhoneStatusCodes[] CurrentStatusRange;

        long Extension;
        string UserName;
        string Prefix;
        string Email;
        string Note;
        int Row, Column;
        bool ShowName;
        bool ShowCanonical;
        bool IsRange;   // if it contains 10 numbers instead 1
        Object HiddenTooltip;

        public PhoneNumber(int X, int Y, PhoneNumberSetupSettings settings)
        {
            InitializeComponent();

            CurrentStatusRange = new PhoneStatusCodes[10];

            //set some value to see how it looks like
            this.StatusText.Content = "";
            this.StatusRange.Visibility = Visibility.Hidden;

            SetStatus(PhoneStatusCodes.NotExisting);    //default status
            SetCoordinate(X, Y);
            //remove border if there is no border
            if (settings.ShowGrid == false)
                OnGridToggle();

            ShowName = settings.ShowName;

            ShowCanonical = settings.ShowCanonical;

            //update size
            if (settings.CellWidth > 0 && settings.CellHeight > 0)
                OnCellSizeChanged(settings.CellWidth, settings.CellHeight);

            this.ContextMenu = Resources["contextMenuNew"] as ContextMenu;

            //hide tooltip until we populate subscriber with data
            HiddenTooltip = this.StatusText.ToolTip;
            this.StatusText.ToolTip = null;

            IsRange = false;
        }

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
                HiddenTooltip = this.StatusText.ToolTip;
                this.StatusText.ToolTip = null;

                this.StatusText.VerticalContentAlignment = VerticalAlignment.Top;
                this.StatusRange.Visibility = Visibility.Visible;
            }
        }

        public void SetEmail(string NewEmail)
        {
            Email = NewEmail;
        }

        public void SetNote(string NewNote)
        {
            Note = NewNote;
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

        public void OnFontSettingChanged(PhoneNumberSetupSettings settings)
        {
            if(settings.FontSize!=0)
            {
                this.StatusText.FontFamily = settings.FontFamily;
                this.StatusText.FontSize = settings.FontSize;
                this.StatusText.FontWeight = settings.FontWeight_;
                this.StatusText.FontStyle = settings.FontStyle_;
            }
        }

        public void OnPrefixChanged(string NewPrefix)
        {
            Prefix = NewPrefix;
            //in case prefix changed, we update cannonical view also
            SetExtension(Extension);
        }

        /// <summary>
        /// Load from some source settings of this cell
        /// </summary>
        public void Load()
        {
            if (Row < 0 || Column < 0)
                return;
        }

        /// <summary>
        /// Change the status of this phone.It will have different color
        /// </summary>
        /// <param name="NewStatus"></param>
        public void SetStatus(PhoneStatusCodes NewStatus)
        {
            if (NewStatus < PhoneStatusCodes.NumberOfStatusCodes)
                this.StatusText.Background = StatusColorEditor.GetStatusColor(NewStatus);
            //unknown status ? Bail out
            else
                return;
            CurrentStatus = NewStatus;
        }

        public void SetRangeStatus(int Index, PhoneStatusCodes NewStatus)
        {
            if (Index < 0 || Index > 9)
                return;
            if (NewStatus < PhoneStatusCodes.NumberOfStatusCodes)
            {
                if(Index==0)
                    this.StatusText0.Background = StatusColorEditor.GetStatusColor(NewStatus);
                else if(Index==1)
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
            this.StatusText.Background = StatusColorEditor.GetStatusColor(CurrentStatus);
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

        private void OnContextMenuChange()
        {
            if (IsRange == false)
            {
                this.ContextMenu = Resources["contextMenuUser"] as ContextMenu;
                this.StatusText.ToolTip = HiddenTooltip;
            }
            else
                this.ContextMenu = Resources["contextMenuRange"] as ContextMenu;
        }
        /// <summary>
        /// Extension is the actual phone number people need to dial
        /// </summary>
        /// <param name="Extension"></param>
        public void SetExtension(long pExtension)
        {
            //no longer an unused cell
            if (Extension != pExtension)
                OnContextMenuChange();

            //some sort of extension
            Extension = pExtension;

            if (ShowName == false)
            {
                if (ShowCanonical == false)
                    this.StatusText.Content = Extension.ToString();
                else
                    this.StatusText.Content = Prefix + Extension.ToString();
            }

            this.TooltipPhoneExtension.Text = "Extension : " + Extension.ToString();
        }

        public void SetName(string pName)
        {
            //no longer an unused cell
            if (UserName != pName)
                OnContextMenuChange();

            //assign a new user name
            UserName = pName;

            //update visually
            if (ShowName == true)
                this.StatusText.Content = "Name " + UserName;
        }

        /// <summary>
        /// Get the actual phone number
        /// </summary>
        /// <returns></returns>
        public long GetExtension()
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

        public void OnGridToggle()
        {
            this.StatusText.BorderThickness = new Thickness(1 - StatusText.BorderThickness.Left);
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
    }
}
