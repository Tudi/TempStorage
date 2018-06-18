using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Timers;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace BLFClient
{
    public class TabSetupSettings
    {
        public bool ShowGrid;
        public bool ShowName;
        public bool ShowCanonical;
        public double CellWidth, CellHeight;
        public System.Windows.Media.FontFamily FontFamily;
        public double FontSize;
        public FontWeight FontWeight_;
        public FontStyle FontStyle_;
        public string Prefix;   // only used when creating a new user ( phone number )
    }

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        List<IndexCard> IndexCards;
        int SelectedIndexCard;
        bool TopmostWindow;
        TabSetupSettings GeneralSettings;
        string AdminPassw;
        bool HasAdminPriviledges;
        int BLFServerPort;
        string BLFServerIP;

        public MainWindow()
        {
            InitializeComponent();

            GeneralSettings = new TabSetupSettings();

            //load settings 
            GeneralSettings.ShowGrid = true;
            GeneralSettings.ShowName = false;
            TopmostWindow = false;
            GeneralSettings.ShowCanonical = false;
            GeneralSettings.CellWidth = GridContentArea.GetDefaultCellWidth();
            GeneralSettings.CellHeight = GridContentArea.GetDefaultCellHeight();
            AdminPassw = "HIPATH";
            HasAdminPriviledges = false;
            BLFServerPort = 5059;
            BLFServerIP = "127.0.0.1";

            //load saved tabs
            LoadIndexCards();

            //use loaded settings to init data
            //generate grid layout based on the size of the window
            RedrawIndexCards();

            //set status code background colors
            InitStatusBar();

            // after all settings have been loaded
            // in case we resize the window, we will redraw the grid

            //last thing, add hotkeys
            AddHotKeys();
        }

        /// <summary>
        /// Destroy the main window while we save recent changes
        /// </summary>
        ~MainWindow()
        {
            foreach (var Item in IndexCards)
                Item.Save();
            IndexCards.Clear();
            IndexCards = null;
        }

        public string GetPrefix()
        {
            return GeneralSettings.Prefix;
        }

        private void AddHotKeys()
        {
            RoutedCommand firstSettings = new RoutedCommand();
            firstSettings.InputGestures.Add(new KeyGesture(Key.F7, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(firstSettings, OnF7Pressed));

            RoutedCommand b = new RoutedCommand();
            b.InputGestures.Add(new KeyGesture(Key.F1, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(b, OnF1Pressed));

        }

        private void OnF7Pressed(object sender, ExecutedRoutedEventArgs e)
        {
            Click_Menu_ShowNameToggle(null, null);
        }

        private void OnF1Pressed(object sender, ExecutedRoutedEventArgs e)
        {
            Click_Menu_OpenHelp(null, null);
        }

        /// <summary>
        /// Load configs and index cards
        /// </summary>
        public void LoadIndexCards()
        {
            if (IndexCards == null)
            {
                //create a list
                IndexCards = new List<IndexCard>();

                //create a default index card that we will later save / populate
                AppendIndexCard();
                IndexCards[0].SetVisible(true);
                IndexCards[0].StartDemo(1);

                AppendIndexCard();
                IndexCards[1].StartDemo(2);

                AppendIndexCard();
            }
            SelectedIndexCard = 0;
        }

        /// <summary>
        /// Create a default index card and append it to the list of index cards
        /// </summary>
        public void AppendIndexCard()
        {

            IndexCard nc = new IndexCard(GeneralSettings);
            nc.SetVisible(false);
            nc.SetName("Unsaved " + IndexCards.Count.ToString());
            IndexCards.Add(nc);
        }

        /// <summary>
        /// Delete an index card. Auto select a new one and draw it
        /// </summary>
        /// <param name="todel"></param>
        public void DeleteIndexCard(IndexCard todel)
        {
            bool NeedsVisualRefresh = (todel == IndexCards[SelectedIndexCard]);
            //delete it
            todel.OnDelete();
            IndexCards.Remove(todel);
            // if this is the selected index card. try to select a new one
            if (NeedsVisualRefresh)
            {
                //select a new index card and show it
                if (IndexCards.Count == 0)
                    SelectedIndexCard = -1;
                else
                {
                    SelectedIndexCard = 0;
                    IndexCards[SelectedIndexCard].SetVisible(true);
                    RedrawIndexCards();
                }
            }
        }

        /// <summary>
        /// Hide all index cards from being visible. No more redraw should happen on resize
        /// </summary>
        public void HideActiveIndexCard()
        {
            if (SelectedIndexCard < 0 || SelectedIndexCard >= IndexCards.Count)
                return;
            IndexCards[SelectedIndexCard].SetVisible(false);
        }

        public IndexCard GetVisibleIndexCard()
        {
            if (SelectedIndexCard < 0 || SelectedIndexCard >= IndexCards.Count)
                return null;
            return IndexCards[SelectedIndexCard];
        }

        /// <summary>
        /// Redraw the active index card. Invisible cards will be ingored
        /// </summary>
        public void RedrawIndexCards()
        {
            IndexCard ic = GetVisibleIndexCard();
            if (ic != null)
                ic.ShowIndexCardContent();
        }

        /// <summary>
        /// Set status bar colors based on the settings loaded
        /// </summary>
        public void InitStatusBar()
        {
            this.StatusIdle.Background = StatusColorEditor.GetStatusColor(PhoneStatusCodes.Idle);
            this.StatusRinging.Background = StatusColorEditor.GetStatusColor(PhoneStatusCodes.Ringing);
            this.StatusOutOfService.Background = StatusColorEditor.GetStatusColor(PhoneStatusCodes.OutOfService);
            this.StatusBusy.Background = StatusColorEditor.GetStatusColor(PhoneStatusCodes.Busy);
            this.StatusNotExisting.Background = StatusColorEditor.GetStatusColor(PhoneStatusCodes.NotExisting);
        }

        /// <summary>
        /// On window size change, we should adjust the number of shown PhoneNumbers on the screen
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void WindowSizeChanged(object sender, SizeChangedEventArgs e)
        {
            RedrawIndexCards();
        }

        private void TabControl_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (this.TabHolder.SelectedIndex < 0 || this.TabHolder.SelectedIndex >= IndexCards.Count)
                return;
            HideActiveIndexCard();
            SelectedIndexCard = this.TabHolder.SelectedIndex;
            IndexCards[SelectedIndexCard].SetVisible(true);
            RedrawIndexCards();
        }

        private void Click_Menu_NewIndexCard(object sender, RoutedEventArgs e)
        {
            AppendIndexCard();
        }

        private void Click_Menu_DeleteIndexCard(object sender, RoutedEventArgs e)
        {
            DeleteIndexCard(IndexCards[SelectedIndexCard]);
        }

        private void Click_Menu_RenameIndexCard(object sender, RoutedEventArgs e)
        {
            var cw = new IndexCardRename(IndexCards[SelectedIndexCard]);
            cw.ShowInTaskbar = false;
            cw.Owner = System.Windows.Application.Current.MainWindow;
            cw.Show();
        }

        private void Click_Menu_Color(object sender, RoutedEventArgs e)
        {
            var cw = new StatusColorEditor();
            cw.ShowInTaskbar = false;
            cw.Owner = System.Windows.Application.Current.MainWindow;
            cw.Show();
        }

        public void OnStatusColorChanged()
        {
            //status bar color
            InitStatusBar();
            //iterate through all tabs of indexxards and let them also know to update cells
            for (int i = 0; i < IndexCards.Count; i++)
                IndexCards[i].OnStatusColorChanged();
        }

        private void Click_Menu_GridToggle(object sender, RoutedEventArgs e)
        {
            if (GeneralSettings.ShowGrid)
                GeneralSettings.ShowGrid = false;
            else
                GeneralSettings.ShowGrid = true;

            for (int i = 0; i < IndexCards.Count; i++)
                IndexCards[i].OnGridToggle();
        }

        private void Click_Menu_ShowNameToggle(object sender, RoutedEventArgs e)
        {
            if (GeneralSettings.ShowName)
                GeneralSettings.ShowName = false;
            else
                GeneralSettings.ShowName = true;

            for (int i = 0; i < IndexCards.Count; i++)
                IndexCards[i].OnToggleShowName();
        }

        private void Click_Menu_LegendToggle(object sender, RoutedEventArgs e)
        {
            if (this.StatusIdle.Visibility == Visibility.Hidden)
            {
                this.StatusIdle.Visibility = Visibility.Visible;
                this.StatusRinging.Visibility = Visibility.Visible;
                this.StatusOutOfService.Visibility = Visibility.Visible;
                this.StatusBusy.Visibility = Visibility.Visible;
                this.StatusNotExisting.Visibility = Visibility.Visible;
            }
            else
            {
                this.StatusIdle.Visibility = Visibility.Hidden;
                this.StatusRinging.Visibility = Visibility.Hidden;
                this.StatusOutOfService.Visibility = Visibility.Hidden;
                this.StatusBusy.Visibility = Visibility.Hidden;
                this.StatusNotExisting.Visibility = Visibility.Hidden;
            }
        }

        private void Click_Menu_TopmostToggle(object sender, RoutedEventArgs e)
        {
            if (TopmostWindow == false)
                TopmostWindow = true;
            else
                TopmostWindow = false;
            this.Topmost = TopmostWindow;
        }

        private void Window_Deactivated(object sender, EventArgs e)
        {
            this.Topmost = TopmostWindow;
        }

        private void Click_Menu_CanonicalToggle(object sender, RoutedEventArgs e)
        {
            if (GeneralSettings.ShowCanonical)
                GeneralSettings.ShowCanonical = false;
            else
                GeneralSettings.ShowCanonical = true;

            for (int i = 0; i < IndexCards.Count; i++)
                IndexCards[i].OnToggleShowCanonical();
        }

        private void Click_Menu_ViewAbsenceManagementToggle(object sender, RoutedEventArgs e)
        {
            if (this.AbsenceView.Visibility == Visibility.Hidden)
                this.AbsenceView.Visibility = Visibility.Visible;
            else
                this.AbsenceView.Visibility = Visibility.Hidden;
            RedrawIndexCards();
        }

        private void Click_Menu_OpenAbsenceManagement(object sender, RoutedEventArgs e)
        {
            var cw = new AbsenceManageEdit();
            cw.ShowInTaskbar = false;
            cw.Owner = System.Windows.Application.Current.MainWindow;
            cw.Show();
        }

        private void Click_Menu_OpenCellSizeEdit(object sender, RoutedEventArgs e)
        {
            var cw = new PhoneNumberSizeEdit(GeneralSettings.CellWidth, GeneralSettings.CellHeight);
            cw.ShowInTaskbar = false;
            cw.Owner = System.Windows.Application.Current.MainWindow;
            cw.Show();
        }

        public void OnCellSizeChanged(double NewWidth, double NewHeight)
        {
            //new tabs will be opened with this setting
            GeneralSettings.CellWidth = NewWidth;
            GeneralSettings.CellHeight = NewHeight;

            //update all current index cards with new global settings
            for (int i = 0; i < IndexCards.Count; i++)
                IndexCards[i].OnCellSizeChanged(NewWidth, NewHeight);

            //make sure index cards show according to actual settings
            RedrawIndexCards();
        }

        private void Click_Menu_OpenFontEdit(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.FontDialog fd = new System.Windows.Forms.FontDialog();
            System.Windows.Forms.DialogResult dr = fd.ShowDialog();
            if (dr != System.Windows.Forms.DialogResult.Cancel)
            {
                GeneralSettings.FontFamily = new System.Windows.Media.FontFamily(fd.Font.Name);
                GeneralSettings.FontSize = fd.Font.Size * 96.0 / 72.0;
                GeneralSettings.FontWeight_ = fd.Font.Bold ? FontWeights.Bold : FontWeights.Regular;
                GeneralSettings.FontStyle_ = fd.Font.Italic ? FontStyles.Italic : FontStyles.Normal;

                for (int i = 0; i < IndexCards.Count; i++)
                    IndexCards[i].OnFontSettingChanged(GeneralSettings);
            }
        }

        private void Click_Menu_OpenPrefixEdit(object sender, RoutedEventArgs e)
        {
            var cw = new PhoneNumberPrefixEdit(GeneralSettings.Prefix);
            cw.ShowInTaskbar = false;
            cw.Owner = System.Windows.Application.Current.MainWindow;
            cw.Show();
        }

        public void OnPrefixChanged(string NewPrefix)
        {
            //new tabs will be opened with this setting
            GeneralSettings.Prefix = NewPrefix;

            //update all current index cards with new global settings
            //            for (int i = 0; i < IndexCards.Count; i++)
            //                IndexCards[i].OnPrefixChanged(NewPrefix);
        }

        public PhoneNumber PhoneNumberGet(int x, int y)
        {
            return GetVisibleIndexCard().PhoneNumberGet(x, y);
        }

        public void PhoneNumberDeleteAddNew(int x, int y)
        {
            GetVisibleIndexCard().PhoneNumberDelete(x, y);
            GetVisibleIndexCard().PhoneNumberAdd(x, y);
        }

        private void Click_Menu_OpenAdminLogin(object sender, RoutedEventArgs e)
        {
            //this is a toggle menu. Only admin can save to "*.blf" files
            if (HasAdminPriviledges == true)
            {
                this.Tools_User_Admin.IsChecked = false;
                this.FILE_SAVE.IsEnabled = false;
                this.FILE_SAVE_AS.IsEnabled = false;
                HasAdminPriviledges = false;
                return;
            }
            var cw = new AdminLogin();
            cw.ShowInTaskbar = false;
            cw.Owner = System.Windows.Application.Current.MainWindow;
            cw.Show();
        }

        public void OnAdminLogin(string Passw)
        {
            if(AdminPassw == Passw)
            {
                HasAdminPriviledges = true;
                this.FILE_SAVE.IsEnabled = true;
                this.FILE_SAVE_AS.IsEnabled = true;
                this.Tools_User_Admin.IsChecked = true;
            }
            else
                this.Tools_User_Admin.IsChecked = false;
        }

        private void Click_Menu_OpenChangePassw(object sender, RoutedEventArgs e)
        {
            var cw = new AdminChangePassw();
            cw.ShowInTaskbar = false;
            cw.Owner = System.Windows.Application.Current.MainWindow;
            cw.Show();
        }

        public bool OnAdminChangePassw(string oldPassw,string Passw)
        {
            if(oldPassw != AdminPassw)
            {
                MessageBox.Show("Old password is incorrect");
                return false;
            }
            AdminPassw = Passw;
            return true;
        }

        private void Click_Menu_OpenBLFServerConfig(object sender, RoutedEventArgs e)
        {
            var cw = new BLFServerConfig(BLFServerIP,BLFServerPort);
            cw.ShowInTaskbar = false;
            cw.Owner = System.Windows.Application.Current.MainWindow;
            cw.Show();
        }

        public void OnBLFServerConfigChange(string IP, int Port)
        {
            BLFServerIP = IP;
            BLFServerPort = Port;
        }

        private void Click_Menu_OpenHelp(object sender, RoutedEventArgs e)
        {
            //System.Windows.Forms.Help.ShowHelp(null, @".\\Help\\BLF_AdminEng.chm");
            System.Diagnostics.Process.Start(@".\\Help\\BLF_AdminEng.chm");
        }

        private void Click_Menu_OpenAbout(object sender, RoutedEventArgs e)
        {
            var cw = new About();
            cw.ShowInTaskbar = false;
            cw.Owner = System.Windows.Application.Current.MainWindow;
            cw.Show();
        }
    }
}
