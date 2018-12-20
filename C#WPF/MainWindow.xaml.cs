using BLFClient.Backend;
using IniParser.Model;
using System;
using System.Collections.Generic;
using System.IO;
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
        public bool ShowLegend;
        public double CellWidth, CellHeight;
        public string Prefix;   // only used when creating a new user ( phone number )
    }

    public static class Globals
    {
        public static LogManager Logger;
        public static ConfigManager Config;
        public static ConfigManager IniFile;
        public static FontStyleManager FontManager;
        public static OutlookAPI OutlookService;
        public static AbsenceManager AbsenceManage;
        public static PhoneNumberManager ExtensionManager;
        public static IndexCardManager FolderManager;
        public static ExchangeWebServices ExchangeAPI;
        public static NetworkConnectionManager ConnectionManager;
        public static CallForwardManager ForwardManager;
        public static ServerAntiFloodManager AntiFloodManager;
        public static LanguageTranslator MultilangManager;
        public static PersPortManager persPortManager;
        public static ACWinConnectionManager AcWinManager = null;
        public static DSWinConnectionManager DSWinManager = null;
        public static bool IsAppRunning = false;  // set it false when shutting down to allow parallel running managers to shut down correctly
        public static bool IndexCardsLoaded = false; // if each index card, and each extension data is loaded, only than set this value to true
        public static ApplicationGlobalVariables AppVars;
        public static bool WindowLoaded = false;
        public static string GetFullAppPath(string FileName)
        {
            string ret = Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData);
            Directory.CreateDirectory(ret + "\\BLF");
            Directory.CreateDirectory(ret + "\\BLF\\Settings");
            Directory.CreateDirectory(ret + "\\BLF\\Logs");
            Directory.CreateDirectory(ret + "\\BLF\\Database");
            ret = ret + "\\BLF\\" + FileName;
            return ret;
        }
    }

    public class ApplicationGlobalVariables
    {
        //        public bool m_bMakeCall = false;
        //        public bool m_bConsultationCall = false;
        //        public string m_strCallerExtension = "";
        //        public string m_strDeviceID = "";
        //        public string m_strCallID = "";
        //        public string m_strDeviceIDTemp = "";
        //        public string m_strCallIDTemp = "";
        public NoConnection NoConnectionWindow = null;
        public string SkinFileName = "RoundedStyles.xaml";
    }

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        List<IndexCard> IndexCards;
        bool TopmostWindow;
        TabSetupSettings GeneralSettings;
        string AdminPassw = "HIPATH";
        bool HasAdminPriviledges = false;
        bool ConnectedToServer = false; // do we need to update UI status when we get a connection heartbeat event
        bool LoadingData = false;   //set this status while loading data to avoid triggering UI events
        int DraggedTabIndex = -1;   // in case we want to reorder tab order
        PhoneNumber DraggedPhoneNumber = null;
        // these are application variables

        public MainWindow()
        {
            InitializeComponent();
            Globals.IsAppRunning = true;

            //as soon as we have the log file name, initialize the logger so we can give feedback 
            Globals.AppVars = new ApplicationGlobalVariables();
            Globals.Logger = new LogManager();
            Globals.Config = new ConfigManager();
            Globals.IniFile = new ConfigManager();
            Globals.FontManager = new FontStyleManager();
            Globals.MultilangManager = new LanguageTranslator();
            Globals.AntiFloodManager = new ServerAntiFloodManager();
            Globals.OutlookService = new OutlookAPI();
            Globals.AbsenceManage = new AbsenceManager();
            Globals.ExtensionManager = new PhoneNumberManager();
            Globals.FolderManager = new IndexCardManager();
            Globals.ExchangeAPI = new ExchangeWebServices();
            Globals.ConnectionManager = new NetworkConnectionManager();
            Globals.ForwardManager = new CallForwardManager();
            Globals.persPortManager = new PersPortManager();
            Globals.ExtensionManager.Load();
            Globals.ForwardManager.Load();
            Globals.OutlookService.InitInBackground();
            Globals.AbsenceManage.Load();           //start monitoring extension emails. Requires a working outlook connection
            Globals.persPortManager.Load();
            TabHolder.ContextMenu = Resources["contextMenuNewIndexCard"] as ContextMenu;

            Globals.MultilangManager.LoadDBFromFile("Resources/Translations.txt");

            //            Globals.Config.LoadIni("BLF_Profine16_S.blf");
            if (Globals.IniFile.LoadIni(Globals.GetFullAppPath("Settings\\BLF.ini")) == false)
                Globals.IniFile.LoadIni("BLF.ini");
            string LastConfigFile = Globals.IniFile.GetConfig("Files", "LastConfigFile");
            if (LastConfigFile == null || LastConfigFile.Length == 0 || File.Exists(LastConfigFile) == false)
                LastConfigFile = "Init.blf";

            RedrawFromConfig(LastConfigFile);

            //last thing, add hotkeys
            AddHotKeys();

            //apply a template if there is any
            Loaded += Backend.StyleManager.RuntimeSkinning_Loaded;
        }

        /// <summary>
        /// Destroy the main window while we save recent changes
        /// </summary>
        ~MainWindow()
        {
            //save window position or other settings that maybe got ignored 
            Window_Closed(null, null);
        }

        private void RedrawFromConfig(string NewConfigFileName)
        {
            LoadingData = true; // disable UI events for now

            //get rid of old data
            Globals.IndexCardsLoaded = false;
            CloseIndexCards();

            //load up the new config data
            if (NewConfigFileName != null)
            {
                Globals.Config.LoadIni(NewConfigFileName);
                UpdateWindowTitle();
            }
            else
                Globals.Config.CloseOpenedConfig();

            Globals.FontManager.LoadFontSettingsFromConfig();

            GeneralSettings = new TabSetupSettings();

            // load settings
            // GenerateDefaultConfigValues(false);

            //reload settings from config file
            LoadSettings();

            //load saved tabs
            LoadIndexCards();

            //use loaded settings to init data
            //generate grid layout based on the size of the window
            RedrawIndexCards();

            //set status code background colors
            InitStatusBar();

            LoadingData = false; // Enable

            //translate static of this window to localized version
            TranslateLocalize();
        }

        private void CloseIndexCards()
        {
            if (IndexCards != null)
            {
                //clone current list
                IndexCard[] t = new IndexCard[IndexCards.Count];
                IndexCards.CopyTo(t);
                IndexCards.Clear();
                foreach (var ic in t)
                    DeleteIndexCard(ic);
                IndexCards = null;
            }
        }

        private void Click_Menu_New(object sender, EventArgs e)
        {
            RedrawFromConfig(null);
        }

        private void Click_Menu_Save(object sender, EventArgs e)
        {
            if (Globals.Config.GetIniName() == null)
                Click_Menu_SaveAs(sender, e);
            else
                SaveConfigValues();
        }

        private void Click_Menu_SaveAs(object sender, EventArgs e)
        {
            Microsoft.Win32.SaveFileDialog dlg = new Microsoft.Win32.SaveFileDialog();
            dlg.FileName = Globals.Config.GetIniName(); // Default file name
            dlg.DefaultExt = ".blf"; // Default file extension
            dlg.Filter = "BLF Files (*.blf)|*.blf"; // Filter files by extension

            // Show save file dialog box
            Nullable<bool> result = dlg.ShowDialog();

            // Process save file dialog box results
            if (result == true)
            {
                Globals.Config.SetDefaultInitFileName(dlg.FileName);
                Globals.IniFile.SetConfig("Files", "LastConfigFile", dlg.FileName);
                SaveConfigValues();
            }
            UpdateWindowTitle();
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            //no need to call it multiple times. Seen some issues with shutdown
            if (Globals.IsAppRunning == false)
                return;

            Globals.IsAppRunning = false;

            //we no longer send network messages to servers. Nor we parse incomming messages
            if (Globals.ConnectionManager != null)
            {
                Globals.ConnectionManager.Shutdown();
                Globals.ConnectionManager = null;
            }

            if (IndexCards != null)
                SaveConfigValues();

            if (IndexCards != null)
            {
                IndexCards.Clear();
                IndexCards = null;
            }
        }

        private void Click_Menu_CallForwarding(object sender, EventArgs e)
        {
            CallForwardingEdit t = new CallForwardingEdit("");
            t.Show();
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

            RoutedCommand c = new RoutedCommand();
            c.InputGestures.Add(new KeyGesture(Key.F2, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(c, Click_Menu_HideMenu));
        }

        private void OnF7Pressed(object sender, ExecutedRoutedEventArgs e)
        {
            Click_Menu_ShowNameToggle(null, null);
        }

        private void OnF1Pressed(object sender, ExecutedRoutedEventArgs e)
        {
            Click_Menu_OpenHelp(null, null);
        }

        private void IndexCardMakeVisible(IndexCard ic)
        {
            for (int i = 0; i < IndexCards.Count; i++)
            {
                if (IndexCards[i] != ic)
                    continue;
                TabHolder.SelectedIndex = i;
                IndexCards[TabHolder.SelectedIndex].SetVisible(true);
                break;
            }
            //make sure others are hidded. No need to spam updates for idle extensions
            for (int i = 0; i < IndexCards.Count; i++)
            {
                if (i == TabHolder.SelectedIndex)
                    continue;
                IndexCards[i].SetVisible(false);
            }
        }

        /// <summary>
        /// If we delete an index card, jump to the next index card and make it visible
        /// </summary>
        private void SetVisibleIndexCardAuto()
        {
            for (int i = 0; i < IndexCards.Count; i++)
            {
                if (IndexCards[i].CanReceiveFocus() == false)
                    continue;
                IndexCardMakeVisible(IndexCards[i]);
                break;
            }
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
            }

            KeyDataCollection IndexCardCollection = Globals.Config.GetConfigSection("Folders");

            //this is demo mode
            if (Globals.IniFile.GetConfig("Options", "Demo", "NO") == "YES")
            {
                //create a default index card that we will later save / populate
                AppendIndexCard();
                IndexCards[0].StartDemo(1);

                AppendIndexCard();
                IndexCards[1].StartDemo(2);

                AppendIndexCard();

                SetVisibleIndexCardAuto();
                Globals.IndexCardsLoaded = true;
            }
            else if (IndexCardCollection != null)
            {
                //iterate through all the index card names
                foreach (KeyData key in IndexCardCollection)
                {
                    IndexCard nc = new IndexCard(GeneralSettings);
                    nc.SetName(key.Value);
                    // as soon as index card has a name we can load it from config file
                    //                    nc.Load();    //load content on demand instead of use
                    nc.SetConfigIndex(IndexCards.Count);
                    IndexCards.Add(nc);
                }

                if (IndexCards.Count > 0)
                {
                    //load the main Index card in this thread to block other load operations, and than load the rest in the background
                    SetVisibleIndexCardAuto();
                    // try to load the index cards in the background
                    // Disable this line ( comment ) if you wish to have load on demand
                    Task.Run(() => { BackgroundLoadIndexCards(); });
                }
            }
            else
            {
                AppendIndexCard();
                SetVisibleIndexCardAuto();
                Globals.IndexCardsLoaded = true;
            }
        }

        private void BackgroundLoadIndexCards()
        {
            Application.Current.Dispatcher.Invoke((Action)delegate
            {
                //quick close while application is starting up
                if (IndexCards == null)
                    return;
                for (int i = IndexCards.Count; i > 0; i--)
                {
                    //this can happen if you close the window while it is still loading
                    if (IndexCards == null)
                        return;
                    IndexCards[i - 1].Load();
                }
                IndexCard nc = AppendIndexCard();
                nc.ConvertToAddIndexCardTab();
                Globals.IndexCardsLoaded = true;
            });
        }

        /// <summary>
        /// Create a default index card and append it to the list of index cards
        /// </summary>
        public IndexCard AppendIndexCard()
        {

            IndexCard nc = new IndexCard(GeneralSettings);
            IndexCards.Add(nc);
            if (IndexCards.Count == 1)
                SetVisibleIndexCardAuto();
            else
                nc.SetVisible(false);
            nc.SetName(Globals.MultilangManager.GetTranslation("Unsaved ") + IndexCards.Count.ToString());
            return nc;
        }

        /// <summary>
        /// Delete an index card. Auto select a new one and draw it
        /// </summary>
        /// <param name="todel"></param>
        public void DeleteIndexCard(IndexCard todel)
        {
            bool NeedsVisualRefresh;
            if (TabHolder.SelectedIndex < IndexCards.Count && TabHolder.SelectedIndex >= 0 && todel == IndexCards[TabHolder.SelectedIndex])
                NeedsVisualRefresh = true;
            else
                NeedsVisualRefresh = false;
            //delete it
            todel.OnDelete();
            IndexCards.Remove(todel);
            // if this is the selected index card. try to select a new one
            if (NeedsVisualRefresh)
            {
               SetVisibleIndexCardAuto();
            }
        }

        public IndexCard GetVisibleIndexCard()
        {
            if (TabHolder.SelectedIndex < 0 || IndexCards == null || TabHolder.SelectedIndex >= IndexCards.Count)
                return null;
            return IndexCards[TabHolder.SelectedIndex];
        }

        /// <summary>
        /// Redraw the active index card. Invisible cards will be ingored
        /// </summary>
        public void RedrawIndexCards()
        {
            IndexCard ic = GetVisibleIndexCard();
            if (ic == null)
                return;
            //grids are loaded on demand. if there is already a grid, nothing to be done here
            ic.SetVisible(true);
            //if no tab is selected, no content will be shown
            if (TabHolder.SelectedIndex == -1)
                TabHolder.SelectedIndex = 0;
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
            this.StatusNotExisting.Background = StatusColorEditor.GetStatusColor(PhoneStatusCodes.PHONE_DOESNOT);
        }

        /// <summary>
        /// On window size change, we should adjust the number of shown PhoneNumbers on the screen
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void WindowSizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (App.Current.MainWindow.WindowState == WindowState.Maximized)
            {
                App.Current.MainWindow.Top = 0;
                App.Current.MainWindow.Left = 0;
            }
            Globals.Config.SetConfig("Windowposition", "Left", ((int)App.Current.MainWindow.Left).ToString());
            Globals.Config.SetConfig("Windowposition", "Right", ((int)(App.Current.MainWindow.Left + App.Current.MainWindow.ActualWidth)).ToString());
            Globals.Config.SetConfig("Windowposition", "Top", ((int)App.Current.MainWindow.Top).ToString());
            Globals.Config.SetConfig("Windowposition", "Bottom", ((int)(App.Current.MainWindow.Top + App.Current.MainWindow.ActualHeight)).ToString());
            RedrawIndexCards();
        }

        private void Window_LocationChanged(object sender, EventArgs e)
        {
            Globals.Config.SetConfig("Windowposition", "Left", ((int)App.Current.MainWindow.Left).ToString());
            Globals.Config.SetConfig("Windowposition", "Right", ((int)(App.Current.MainWindow.Left + App.Current.MainWindow.ActualWidth)).ToString());
            Globals.Config.SetConfig("Windowposition", "Top", ((int)App.Current.MainWindow.Top).ToString());
            Globals.Config.SetConfig("Windowposition", "Bottom", ((int)(App.Current.MainWindow.Top + App.Current.MainWindow.ActualHeight)).ToString());
        }

        private void TabControl_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (TabHolder.SelectedIndex < 0 || TabHolder.SelectedIndex >= IndexCards.Count || LoadingData == true)
                return;
            //if we click last index card, we will add a new index card to the list
            if(IndexCards[TabHolder.SelectedIndex].CanReceiveFocus() == false)
            {
                Click_Menu_NewIndexCard(null,null);
                return;
            }
            IndexCardMakeVisible(IndexCards[TabHolder.SelectedIndex]);
        }

        private void Click_Menu_FileOpen(object sender, RoutedEventArgs e)
        {
            // Create OpenFileDialog 
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            // Set filter for file extension and default file extension 
            dlg.DefaultExt = ".blf";
            dlg.Filter = "BLF Files (*.blf)|*.blf";
            // Display OpenFileDialog by calling ShowDialog method 
            Nullable<bool> result = dlg.ShowDialog();
            // Get the selected file name and display in a TextBox 
            if (result == true)
            {
                // Open document 
                if (File.Exists(dlg.FileName) == true)
                {
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Opened config file " + dlg.FileName + " to load UI content");
                    Globals.IniFile.SetConfig("Files", "LastConfigFile", dlg.FileName);
                    //get rid of current visual layout
                    RedrawFromConfig(dlg.FileName);
                }
                else
                {
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Tried to open an unexisting file. Is this possible ?");
                }
            }
        }

        private void Click_Menu_Exit(object sender, RoutedEventArgs e)
        {
            MessageBoxResult result = MessageBox.Show(Globals.MultilangManager.GetTranslation("Are you sure you want to quit this program?"), "Exit", MessageBoxButton.YesNo, MessageBoxImage.Question);
            if (result == MessageBoxResult.Yes)
            {
                Application.Current.Shutdown();
            }
        }

        private void Click_Menu_NewIndexCard(object sender, RoutedEventArgs e)
        {
            //create a new "+" index card
            IndexCard nic = AppendIndexCard();
            //copy new name to previous tab
            if(IndexCards.Count >= 2)
                IndexCards[IndexCards.Count - 2].SetName(nic.GetName());
            //newly appended tab will become the "+" tab
            nic.ConvertToAddIndexCardTab();
            //make the last-1 tab to be visible
            IndexCardMakeVisible(IndexCards[IndexCards.Count - 2]);
            //open the rename index card window
            Click_Menu_RenameIndexCard(null, null);
        }

        private void Click_Menu_DeleteIndexCard(object sender, RoutedEventArgs e)
        {
            if (IndexCards[TabHolder.SelectedIndex].CanReceiveFocus() == false)
                return;
            MessageBoxResult result = MessageBox.Show(Globals.MultilangManager.GetTranslation("Do you want to delete the index card?"), "Exit", MessageBoxButton.YesNo, MessageBoxImage.Question);
            if (result == MessageBoxResult.Yes)
            {
                DeleteIndexCard(IndexCards[TabHolder.SelectedIndex]);
            }
        }

        private void Click_Menu_RenameIndexCard(object sender, RoutedEventArgs e)
        {
            if (IndexCards[TabHolder.SelectedIndex].CanReceiveFocus() == false)
                return;
            var cw = new IndexCardRename(IndexCards[TabHolder.SelectedIndex]);
            cw.ShowInTaskbar = false;
            cw.ShowDialog();
        }

        private void Click_Menu_Color(object sender, RoutedEventArgs e)
        {
            var cw = new StatusColorEditor();
            cw.ShowInTaskbar = false;
            cw.Owner = System.Windows.Application.Current.MainWindow;
            cw.ShowDialog();
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
                IndexCards[i].SetShowGrid(GeneralSettings.ShowGrid);
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
            {
                Globals.Config.SetConfig("Options", "AbsenceManagement", "YES");
                this.AbsenceView.Visibility = Visibility.Visible;
                Globals.AbsenceManage.ShowUI(true);
            }
            else
            {
                Globals.Config.SetConfig("Options", "AbsenceManagement", "NO");
                this.AbsenceView.Visibility = Visibility.Hidden;
                Globals.AbsenceManage.ShowUI(false);
            }
            RedrawIndexCards();
        }

        private void Click_Menu_OpenAbsenceManagement(object sender, RoutedEventArgs e)
        {
            var cw = new AbsenceManageEdit();
            cw.ShowInTaskbar = false;
            cw.Owner = System.Windows.Application.Current.MainWindow;
            cw.ShowDialog();
        }

        private void Click_Menu_OpenCellSizeEdit(object sender, RoutedEventArgs e)
        {
            var cw = new PhoneNumberSizeEdit(GeneralSettings.CellWidth, GeneralSettings.CellHeight);
            cw.ShowInTaskbar = false;
            cw.Owner = System.Windows.Application.Current.MainWindow;
            cw.ShowDialog();
        }

        public void OnCellSizeChanged(double NewWidth, double NewHeight)
        {
            //new tabs will be opened with this setting
            GeneralSettings.CellWidth = NewWidth;
            GeneralSettings.CellHeight = NewHeight;
            Globals.Config.SetConfig("Options", "CellSizeHor", NewWidth.ToString());
            Globals.Config.SetConfig("Options", "CellSizeVer", NewHeight.ToString());

            //update all current index cards with new global settings
            for (int i = 0; i < IndexCards.Count; i++)
                IndexCards[i].OnCellSizeChanged(NewWidth, NewHeight);

            //make sure index cards show according to actual settings
            RedrawIndexCards();
        }

        public static FontSettings FontDialogToFontSetting(System.Windows.Forms.FontDialog fd)
        {
            FontSettings fs = new FontSettings();

            fs.FaceName = fd.Font.Name;
            fs.FontSize = fd.Font.Size * 96.0 / 72.0;
            if (fd.Font.Italic == true)
                fs.Italic = 1;
            else
                fs.Italic = 0;
            if (fd.Font.Bold == true)
                fs.FontWeight = 1;
            else
                fs.FontWeight = 0;
            if (fd.Font.Strikeout == true)
                fs.StrikeOut = 1;
            else
                fs.StrikeOut = 0;
            if (fd.Font.Underline == true)
                fs.Underline = 1;
            else
                fs.Underline = 0;
            fs.Charset = fd.Font.GdiCharSet;
            return fs;
        }

        private void Click_Menu_OpenFontEdit(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.FontDialog fd = new System.Windows.Forms.FontDialog();
            System.Windows.Forms.DialogResult dr = fd.ShowDialog();
            if (dr != System.Windows.Forms.DialogResult.Cancel)
            {
                Globals.FontManager.ClearAllFonts();
                FontSettings fs = FontDialogToFontSetting(fd);
                Globals.FontManager.InsertNewSetting(fs);
                for (int i = 0; i < IndexCards.Count; i++)
                    IndexCards[i].OnFontSettingChanged(fs);
            }
        }

        private void Click_Menu_OpenPrefixEdit(object sender, RoutedEventArgs e)
        {
            var cw = new PhoneNumberPrefixEdit(GeneralSettings.Prefix);
            cw.ShowInTaskbar = false;
            cw.Owner = System.Windows.Application.Current.MainWindow;
            cw.ShowDialog();
        }

        public void OnPrefixChanged(string NewPrefix)
        {
            //new tabs will be opened with this setting
            GeneralSettings.Prefix = NewPrefix;
            Globals.Config.SetConfig("Options", "Prefix", NewPrefix);
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
            cw.ShowDialog();
        }

        public void OnAdminLogin(string Passw)
        {
            if (AdminPassw == Passw)
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
            cw.ShowDialog();
        }

        public bool OnAdminChangePassw(string oldPassw, string Passw)
        {
            if (oldPassw != AdminPassw)
            {
                MessageBox.Show(Globals.MultilangManager.GetTranslation("Old password is incorrect"));
                return false;
            }
            AdminPassw = Passw;
            return true;
        }

        private void Click_Menu_OpenBLFServerConfig(object sender, RoutedEventArgs e)
        {
            string BLFServerIP = Globals.Config.GetConfig("Options", "BlfServerIp", "");
            int BLFServerPort = Globals.Config.GetConfigInt("Options", "BlfServerPort", 5050);
            var cw = new BLFServerConfig(BLFServerIP, BLFServerPort);
            cw.ShowInTaskbar = false;
            cw.Owner = System.Windows.Application.Current.MainWindow;
            cw.ShowDialog();
        }

        private void Click_Menu_OpenHelp(object sender, RoutedEventArgs e)
        {
            //System.Windows.Forms.Help.ShowHelp(null, @".\\Help\\BLF_AdminEng.chm");
            if (File.Exists(@".\\Help\\BLF_AdminEng.chm") == true)
                System.Diagnostics.Process.Start(@".\\Help\\BLF_AdminEng.chm");
            else
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Missing help file BLF_AdminEng.chm");
                MessageBoxResult result = MessageBox.Show(Globals.MultilangManager.GetTranslation("Error : Help file missing"), "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void Click_Menu_HideMenu(object sender, RoutedEventArgs e)
        {
            //            Globals.Config.SetConfig("Options", "Menu","NO"); // once hidden, there is no more way to save this setting. So there is no point setting it ?
            if (this.MenuObject.Height != 1)
                this.MenuObject.Height = 1;
            else
                this.MenuObject.Height = 20;
        }

        private void Click_Menu_ShowMenu(object sender, RoutedEventArgs e)
        {
            //            Globals.Config.SetConfig("Options", "Menu","YES"); // once hidden, there is no more way to save this setting. So there is no point setting it ?
            this.MenuObject.Height = 20;
        }

        private void Click_Menu_OpenAbout(object sender, RoutedEventArgs e)
        {
            var cw = new About();
            cw.ShowInTaskbar = false;
            cw.Owner = System.Windows.Application.Current.MainWindow;
            cw.ShowDialog();
        }

        public void OnServerConnectionChanged(bool Connected)
        {
            if (ConnectedToServer == Connected)
                return;
            ConnectedToServer = Connected;
            if (Connected == true)
                this.StatusConnectionStatusText.Content = Globals.MultilangManager.GetTranslation("Connected");
            else
                this.StatusConnectionStatusText.Content = Globals.MultilangManager.GetTranslation("Connecting");
            // we should initiate a new "phone" device state query and attach a new monitor for each extension.
            //            if (GetVisibleIndexCard() != null)
            //                GetVisibleIndexCard().OnServerConnectionChanged(Connected);
            //Toggle call forwarding support
            this.Menu_CallForwarding.IsEnabled = Connected;
            //update the context menu of each extension to allow call forwarding clicking
            Globals.ExtensionManager.OnConnectionChanged(Connected);
            //update window title
            UpdateWindowTitle();
        }

        private void LoadSettings()
        {
            //load window position settings 
            int t;
            t = Globals.Config.GetConfigInt("Windowposition", "Left", -1);
            if (t != -1)
                App.Current.MainWindow.Left = t;
            t = Globals.Config.GetConfigInt("Windowposition", "Top", -1);
            if (t != -1)
                App.Current.MainWindow.Top = t;
            t = Globals.Config.GetConfigInt("Windowposition", "Right", -1);
            if (t != -1 && t > App.Current.MainWindow.Left)
                App.Current.MainWindow.Width = t - App.Current.MainWindow.Left;
            t = Globals.Config.GetConfigInt("Windowposition", "Bottom", -1);
            if (t != -1 && t > App.Current.MainWindow.Top)
                App.Current.MainWindow.Height = t - App.Current.MainWindow.Top;

            if (Globals.Config.GetConfig("Options", "Grid", "With Grid") == "With Grid")
            {
                this.cbMenuViewGridLines.IsChecked = true;
                GeneralSettings.ShowGrid = true;
            }
            else
            {
                this.cbMenuViewGridLines.IsChecked = false;
                GeneralSettings.ShowGrid = false;
            }

            if (Globals.Config.GetConfig("Options", "Names", "NO") == "YES")
            {
                this.cbMenuViewNames.IsChecked = true;
                GeneralSettings.ShowName = true;
            }
            else
            {
                this.cbMenuViewNames.IsChecked = false;
                GeneralSettings.ShowName = false;
            }

            if (Globals.Config.GetConfig("Options", "AlwaysOnTop", "NO") == "YES")
            {
                this.cbMenuViewOnTop.IsChecked = true;
                TopmostWindow = true;
            }
            else
            {
                this.cbMenuViewOnTop.IsChecked = false;
                TopmostWindow = false;
            }

            if (Globals.Config.GetConfig("Options", "CanonicalNumber", "NO") == "YES")
            {
                this.cbMenuViewCanonical.IsChecked = true;
                GeneralSettings.ShowCanonical = true;
            }
            else
            {
                this.cbMenuViewCanonical.IsChecked = false;
                GeneralSettings.ShowCanonical = false;
            }

            if (Globals.Config.GetConfig("Options", "Legend", "YES") == "YES")
            {
                this.cbMenuViewLegend.IsChecked = true;
                GeneralSettings.ShowLegend = true;
            }
            else
            {
                this.cbMenuViewLegend.IsChecked = false;
                GeneralSettings.ShowLegend = false;
            }

            //we should monitor optiset status even if it is not present in the UI. This way we can know when to make consultation call and when to call directly
            if (Globals.Config.GetConfig("Options", "Optiset", "NO") == "YES" || Globals.Config.GetConfig("Options", "Optiset", null) == null)
                Globals.ExtensionManager.CreateOptisetPhantomExtension();

            if (Globals.Config.GetConfig("Options", "ACWin", "NO") == "YES")
                Globals.AcWinManager = new ACWinConnectionManager();
            else
                Globals.AcWinManager = null;

            GeneralSettings.CellWidth = Globals.Config.GetConfigInt("Options", "CellSizeHor", (int)GridContentArea.GetDefaultCellWidth());
            GeneralSettings.CellHeight = Globals.Config.GetConfigInt("Options", "CellSizeVer", (int)GridContentArea.GetDefaultCellHeight());
            //            AdminPassw = "HIPATH";
            //            HasAdminPriviledges = false;
            //first load the root values
            string BLFServerIP = Globals.IniFile.GetConfig("Options", "ServerAddress", "");
            int BLFServerPort = Globals.IniFile.GetConfigInt("Options", "ServerListeningPort", 5050);
            //try to override root values with local values
            BLFServerIP = Globals.Config.GetConfig("Options", "BlfServerIp", BLFServerIP);
            BLFServerPort = Globals.Config.GetConfigInt("Options", "BlfServerPort", BLFServerPort);
            //if there is no special setting for this specific config file, try to fall back to original value
            Globals.ConnectionManager.SetConnectionDetails(BLFServerIP, BLFServerPort);

            GeneralSettings.Prefix = Globals.Config.GetConfig("Options", "Prefix", "");

            //should we show absence manage window by default ? Should come after loading index cards to know the size of index cards ?
            if (Globals.Config.GetConfig("Options", "AbsenceManagement", "NO").CompareTo("YES") == 0)
            {
                this.cbMenuViewAbsence.IsChecked = true;
                Click_Menu_ViewAbsenceManagementToggle(null, null);
            }

            if (Globals.IniFile.GetConfig("Options", "Language") == null)
            {
                new LanguageChoose().ShowDialog();
            }

            Globals.Logger.SetFileLogLevelsFromConfig(Globals.IniFile.GetConfig("Options", "LogFlags", ""));
        }

        private void SaveConfigValues()
        {
            //only admin can save changes
            if (HasAdminPriviledges == false)
                return;

            long StartTime = Environment.TickCount;

            //save ini changes. Should this happen without Admin mode ?
            Globals.IniFile.SaveIni();

            //save index cards
            Globals.FolderManager.SaveIndexCards();

            //save fonts. Not reuqired until we support delete. Better be safe than sorry
            Globals.FontManager.SaveAllFonts();

            //save Call forwarding status. Actually this can be stored server side also
            Globals.ForwardManager.Save();

            Globals.Config.SaveIni();

            long Endtime = Environment.TickCount;
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Saved config file in " + (Endtime - StartTime) + " ms");
        }

        private void TranslateLocalize()
        {
            Globals.MultilangManager.TranslateUIComponent(this);
            Globals.MultilangManager.TranslateUIComponent(TabHolder.ContextMenu);
            /*            Globals.MultilangManager.TranslateUIComponent(Resources["contextMenuNew"] as ContextMenu);
                        Globals.MultilangManager.TranslateUIComponent(Resources["contextMenuUser"] as ContextMenu);
                        Globals.MultilangManager.TranslateUIComponent(Resources["contextMenuUserNotConnected"] as ContextMenu);
                        Globals.MultilangManager.TranslateUIComponent(Resources["contextMenuRange"] as ContextMenu);
                        Globals.MultilangManager.TranslateUIComponent(Resources["contextMenuRangeNotConnected"] as ContextMenu);
                        Globals.MultilangManager.TranslateUIComponent(Resources["contextMenu"] as ContextMenu);*/
        }

        private int GetItemIndex(ItemCollection col, object it)
        {
            int ret = -1;
            foreach (object t in col)
            {
                ret++;
                if (t == it)
                    return ret;
            }
            return -1;

        }

        public void OnTabItemDrag(TabItem GrabbedItem, TabItem ReleasedItem)
        {
            //we grabbed a tab, mark it as dragged
            if (GrabbedItem != null && DraggedTabIndex == -1)
            {
                DraggedTabIndex = GetItemIndex(TabHolder.Items, GrabbedItem);
                return;
            }
            //we released a mouse over a tab, if we were dragging another tab than swap places
            if (DraggedTabIndex >= 0 && ReleasedItem != null)
            {
                int ReleasedIndex = GetItemIndex(TabHolder.Items, ReleasedItem);
                //we are dragging a non existing tab ? abort
                if (ReleasedIndex == DraggedTabIndex || DraggedTabIndex > TabHolder.Items.Count || ReleasedIndex < 0)
                {
                    DraggedTabIndex = -1;
                    return;
                }
                int SelectedIndex = TabHolder.SelectedIndex;
                object ReleasedObject = TabHolder.Items[ReleasedIndex];
                object DraggedObject = TabHolder.Items[DraggedTabIndex];
                //create a list that is ordered differently
                List<object> newList = new List<object>();
                for (int i = 0; i< TabHolder.Items.Count; i++)
                {
                    object t = TabHolder.Items[i];
                    if (t == ReleasedObject)
                    {
                        newList.Add(ReleasedObject);
                        newList.Add(DraggedObject);
                    }
                    else if (t != DraggedObject)
                        newList.Add(t);
                }
                TabHolder.Items.Clear();
                foreach (object t in newList)
                    TabHolder.Items.Add(t);

                //do the same with index cards
                List<IndexCard> newListIndexCards = new List<IndexCard>();
                IndexCard DraggedIndexCard = IndexCards[DraggedTabIndex];
                for(int i=0;i< IndexCards.Count;i++)
                {
                    if (i == ReleasedIndex)
                    {
                        newListIndexCards.Add(IndexCards[i]);
                        newListIndexCards.Add(IndexCards[DraggedTabIndex]);
                    }
                    else if(i != DraggedTabIndex)
                        newListIndexCards.Add(IndexCards[i]);
                }
                IndexCards = newListIndexCards;
                //set visible to the new index
                IndexCardMakeVisible(DraggedIndexCard);
            }
            //we no longer are grabbing a tab
            if (ReleasedItem != null)
                DraggedTabIndex = -1;
        }

        public void OnPhoneNumberDrag(PhoneNumber GrabbedItem, PhoneNumber ReleasedItem)
        {
            //did we stop dragging ?
            if (GrabbedItem == null && ReleasedItem == null ) 
            {
                DraggedPhoneNumber = null;
                return;
            }
            //we grabbed a tab, mark it as dragged
            if (GrabbedItem != null && DraggedPhoneNumber == null)
            {
                DraggedPhoneNumber = GrabbedItem;
                return;
            }
            //simple click on the same item
            if (ReleasedItem == DraggedPhoneNumber)
                return;
            //do not swap locations with already taken cells
            if (ReleasedItem != null && ReleasedItem.GetExtension() != "")
                return;
            //we released a mouse over a tab, if we were dragging another tab than swap places
            if (DraggedPhoneNumber != null && ReleasedItem != null)
            {
                IndexCard ic = GetVisibleIndexCard();
                if (ic != null)
                    ic.SwapPhoneNumbersAtPos(DraggedPhoneNumber.GetX(), DraggedPhoneNumber.GetY(), ReleasedItem.GetX(), ReleasedItem.GetY());
            }
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            Globals.WindowLoaded = true;

            //            if (Globals.Config.GetConfig("Options", "DSWin", "YES") == "YES")
            Globals.DSWinManager = new DSWinConnectionManager();
            //            else
            //                Globals.DSWinManager = null;

        }

        private void UpdateWindowTitle()
        {
            string NewTitle = "BLF-WIN";

            if (Globals.ConnectionManager.IsConnected() == true)
                NewTitle += " - connected";
            else
                NewTitle += " - not connected";

            if (Globals.Config.GetIniName() != "")
            {
                if(Globals.Config.GetIniName().IndexOf('\\') > 0 )
                    NewTitle += " - " + Globals.Config.GetIniName().Substring((Globals.Config.GetIniName().LastIndexOf('\\'))+1);
                else
                    NewTitle += " - " + Globals.Config.GetIniName();
            }

            this.Title = NewTitle;
        }
    }
}
