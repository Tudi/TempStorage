using BLFClient.Backend;
using IniParser.Model;
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
    public class DragableTabItem : TabItem
    {
        public DragableTabItem()
        {
            MouseLeftButtonUp += new MouseButtonEventHandler(OnMouseUpTab);
            MouseLeave += new MouseEventHandler(OnMouseLeaveTab);
        }

        private void OnMouseLeaveTab(object Sender, RoutedEventArgs e)
        {
            if(Mouse.LeftButton == MouseButtonState.Pressed)
                ((MainWindow)App.Current.MainWindow).OnTabItemDrag(this, null);
        }

        private void OnMouseUpTab(object Sender, RoutedEventArgs e)
        {
            if ((Sender as TabItem).IsMouseOver == true)
                ((MainWindow)App.Current.MainWindow).OnTabItemDrag(null, this);
        }
    }

    public class PhoneNumberSetupSettings
    {
        public bool ShowGrid;
        public bool ShowName;
        public bool ShowCanonical;
        public double CellWidth, CellHeight;
/*        public System.Windows.Media.FontFamily FontFamily;
        public double FontSize;
        public FontWeight FontWeight_;
        public FontStyle FontStyle_;*/
        public string Prefix;
    }

    /// <summary>
    /// Interaction logic for IndexCard.xaml
    /// </summary>
    public partial class IndexCard : UserControl
    {
        //string shown on the index card
        string IndexCardName = "";
        int ConfigIndex = -1;
        //only show the selected index card
        bool IsCardVisible = false;
        long GUID;

        PhoneNumberSetupSettings GeneralSettings;

        //used only if there is a demo going on
        Timer UpdateTimer;
        int DemoType;   //1 is the random status types and extensions, 2 is default extensions

        //store a list of phone numbers we will need to load/save/restore on focus
        List<PhoneNumber> GridCells;

        TabItem VisualTab;

        /// <summary>
        /// constructor
        /// </summary>
        public IndexCard(TabSetupSettings settings)
        {
            InitializeComponent();

            DemoType = 0;

            GeneralSettings = new PhoneNumberSetupSettings();

            GeneralSettings.ShowGrid = settings.ShowGrid;
            GeneralSettings.ShowName = settings.ShowName;
            GeneralSettings.ShowCanonical = settings.ShowCanonical;
            GeneralSettings.CellWidth = settings.CellWidth;
            GeneralSettings.CellHeight = settings.CellHeight;
            GeneralSettings.Prefix = settings.Prefix;

            DrawArea.SetCellSize(GeneralSettings.CellWidth, GeneralSettings.CellHeight);

            GridCells = new List<PhoneNumber>();

            //create a tabitem that we will show in the "tabholder" tabbar
            VisualTab = new DragableTabItem();
            VisualTab.Content = this;
            VisualTab.ContextMenu = Resources["contextMenu"] as ContextMenu;
            VisualTab.Style = Resources["tabItem"] as Style;

            //not used, just for the sake of sanity
            SetName( Globals.MultilangManager.GetTranslation("Unsaved new") );
            SetVisible(false);

            //show ourself in the tabholder
            MainWindow MainObject = (MainWindow)App.Current.MainWindow;
            MainObject.TabHolder.Items.Add(VisualTab);

            //create a timer that periodically updates the phones
            double DemoTimer = Globals.IniFile.GetConfigNum("Options", "DemoTimer", 0.5);
            UpdateTimer = new Timer(DemoTimer * 60 * 1000);
            UpdateTimer.Enabled = false; // do not trigger the update event until we become visible
            UpdateTimer.Elapsed += new ElapsedEventHandler(PeriodicStatusUpdate);

            Globals.FolderManager.IndexCardAdd(this);

            TranslateLocalize();
        }

        /// <summary>
        /// Save Index card content to file
        /// </summary>
        ~IndexCard()
        {
            OnDelete();
        }

        public void SetGUID(long newGUID)
        {
            GUID = newGUID;
        }

        public long GetGUID()
        {
            return GUID;
        }

        public TabItem GetTabControl()
        {
            return VisualTab;
        }

        public void SetConfigIndex(int Index)
        {
            ConfigIndex = Index;
        }

        public string GetName()
        {
            return IndexCardName;
        }

        public void OnDelete()
        {
            Globals.FolderManager.IndexCardDelete(this);

            //if we were loaded from a config file, remove sections
            if (ConfigIndex>=0)
            {
                Globals.Config.RemoveSection(IndexCardName);
                Globals.Config.RemoveConfig("Folders", "Folder #" + ConfigIndex);
            }

            //stop the demo timer
            if (UpdateTimer != null)
            {
                UpdateTimer.Stop();
                UpdateTimer = null;
            }

            //tell the cells to suicide
            foreach (var cell in GridCells)
                cell.Destroy();
            GridCells.Clear();

            //remove the tab from tabholder
            this.Dispatcher.Invoke(DispatcherPriority.Normal, (Action)(() =>
            {
                if (App.Current == null)
                    return;

                MainWindow MainObject = (MainWindow)App.Current.MainWindow;

                if (MainObject == null)
                    return;

                (App.Current.MainWindow as MainWindow).TabHolder.Items.Remove(VisualTab);
            }));
        }

        /// <summary>
        /// Demo mode. Randomly will change the status of a certain phone
        /// </summary>
        public void StartDemo(int Type)
        {
            DemoType = Type;
            if (Type == 1)
            {
//                UpdateTimer = new Timer(1000) { Enabled = true };
//                UpdateTimer.Elapsed += new ElapsedEventHandler(PeriodicStatusUpdate);
            }
        }

        /// <summary>
        /// Set this card visible. Main window will draw the first visible index card
        /// </summary>
        /// <param name="pIsVisible"></param>
        public void SetVisible(bool pIsVisible)
        {
            //try to load it if it has not yet been loaded. Load data on demand
            if(pIsVisible == true)
                Load();

            bool WasVisible = IsCardVisible;
            IsCardVisible = pIsVisible;
            /*
                        if (IsCardVisible)
                            IndexCardButton.Background = Brushes.Gray;
                        else
                            IndexCardButton.Background = Brushes.LightGray;
            */
            //check if we need to add additional extensions to the draw area
            if(IsCardVisible == true)
                CreateGridBasedOnSize();
            //continue demo on this index card if we became visible again
            if (UpdateTimer != null)
            {
                if (IsCardVisible == true && UpdateTimer.Enabled == false)
                    UpdateTimer.Start();
                //pause the demo until we regain visibility
                if (IsCardVisible == false)
                    UpdateTimer.Stop();
            }
            //if we just became visible, try to query for the status of our extensions
/*            if (IsCardVisible == true && WasVisible != IsVisible)
            {
                foreach (var cell in GridCells)
                    cell.QueryDeviceStatusFromServer();
            }*/
        }

        /// <summary>
        /// Load the content of the index card from a file stream ( config file )
        /// </summary>
        public void Load(bool BackgroundLoad = false)
        {
            lock (this) // we will load index cards in parallel
            {
                //only load once
                if (GridCells.Count != 0)
                    return;
                long StartTime = Environment.TickCount;
                //load Index card related data
                KeyDataCollection IndexCardCollection = Globals.Config.GetConfigSection(GetName());
                //load all possible grid cells ( phone numbers )
                int PhoneNumberLoading = 0;
                do
                {
                    PhoneNumber pn = Globals.ExtensionManager.FactoryNewPhoneNumber(0, 0, GeneralSettings, GetGUID());
                    //try to load from config having this index
                    //                pn.SetConfigIndex(PhoneNumberLoading);
                    //can we load it ?
                    if (pn.Load(GetName(), PhoneNumberLoading) == false)
                        break;
                    //if we loaded it, than add it to our visual grid
                    PhoneNumberAdd(pn.GetX(), pn.GetY(), pn);
                    //try to load more
                    PhoneNumberLoading++;
                } while (true);
                //load ranges
                int RangeNumberLoading = 0;
                do
                {
                    PhoneNumber pn = Globals.ExtensionManager.FactoryNewPhoneNumber(0, 0, GeneralSettings, GetGUID());
                    //try to load from config having this index
                    //                pn.SetConfigIndex(RangeNumberLoading);
                    //can we load it ?
                    if (pn.LoadRange(GetName(), RangeNumberLoading) == false)
                        break;
                    //if we loaded it, than add it to our visual grid
                    PhoneNumberAdd(pn.GetX(), pn.GetY(), pn);
                    //try to load more
                    RangeNumberLoading++;
                } while (true);

                //try to place them inside the grid also
                if (BackgroundLoad == true)
                    CreateGridBasedOnSize();

                long Endtime = Environment.TickCount;
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "For index card " + GetName() + " we loaded " + PhoneNumberLoading.ToString() + " extensions and " + RangeNumberLoading.ToString() + " ranges in " + (Endtime - StartTime) + " ms");
            }
        }

        /// <summary>
        /// Save all index card related data to file
        /// </summary>
        public void Save(int pConfigIndex)
        {
            ConfigIndex = pConfigIndex;
            //make sure we delete old data
            Globals.Config.RemoveSection(IndexCardName);
            //save Index card related data
            Globals.Config.SetConfig("Folders", "Folder #" + ConfigIndex, IndexCardName);
            //iterate through all GridCells ( phone numbers ) and save them 1 by 1
            int SaveIndex = 0;
            int SaveIndexRange = 0;
            foreach (var cell in GridCells)
                if (cell.GetExtension().Length != 0)
                {
                    if (cell.IsSubscriberRange() == false)
                    {
                        cell.Save(GetName(), SaveIndex);
                        SaveIndex++;
                    }
                    else
                    {
                        cell.Save(GetName(), SaveIndexRange);
                        SaveIndexRange++;
                    }
                }
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "For index card " + GetName() + " we saved " + SaveIndex.ToString() + " extensions ");
        }

        /// <summary>
        /// Set the name of the index card. Should update visually also
        /// </summary>
        /// <param name="NewName"></param>
        public void SetName(string NewName)
        {
            //update setting in config file
            if(IndexCardName != "" && ConfigIndex >= 0 && NewName != IndexCardName)
                Globals.Config.RenameSection(IndexCardName, NewName);
            IndexCardName = NewName;
            VisualTab.Header = NewName;
        }

        /// <summary>
        /// Show loaded cells and contents
        /// </summary>
 /*       public void ShowIndexCardContent()
        {
            //nothing to show unless we are the window that should get painted
            if (IsCardVisible == false)
                return;
            //a simple full repaint
            CreateGridBasedOnSize();
        }*/

        /// <summary>
        /// Add a grid cell( Phone number ) to be remembered for this index card.
        /// </summary>
        /// <param name="NewNumber"></param>
        private void AddPhoneNumber(PhoneNumber NewNumber)
        {
            //make sure we do no have this in our list ( sanity check )
            if (GridCells.Find(x => x == NewNumber) != null)
                return;

            GridCells.Add(NewNumber);
        }

        /// <summary>
        /// Get a grid cell from a coordinate. Grid cells are phone numbers for now
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <returns></returns>
        public PhoneNumber PhoneNumberGet(int x, int y)
        {
            return GridCells.Find(iter => iter.GetX() == x && iter.GetY() == y);
        }

        public void PhoneNumberDelete(int x, int y)
        {
            PhoneNumber pn = PhoneNumberGet(x, y);
            if (pn != null)
            {
                GridCells.Remove(pn);
                pn.Destroy();
                DrawArea.RemovePhoneNumber(pn);
            }
        }

        public PhoneNumber PhoneNumberAdd(int x, int y, PhoneNumber NewNumber = null)
        {
            PhoneNumber tcn = PhoneNumberGet(x, y);
            if (tcn == null)
            {
                if (NewNumber != null)
                    tcn = NewNumber;
                else
                    tcn = Globals.ExtensionManager.FactoryNewPhoneNumber(x, y, GeneralSettings, GetGUID());

                if (DemoType == 1 && (x + y) % 2 == 0)
                {
                    tcn.SetExtension((x + y).ToString());
                    if ((x + y) % 3 == 0)
                        tcn.SetIsRange(true);
                }
                else if (DemoType == 2)
                    tcn.SetExtension((x + y).ToString());

                AddPhoneNumber(tcn);
                Grid.SetColumn(tcn, x);
                Grid.SetRow(tcn, y);
            }
            DrawArea.AddPhoneNumber(tcn);
            return tcn;
        }

        public int GetGridColumnCount()
        {
            return DrawArea.GetCellsInWidth();
        }

        /// <summary>
        /// On resize of the window, we want to show as many as possible phone numbers
        /// </summary>
        private void CreateGridBasedOnSize()
        {
            //maybe we should not redraw everything all the time. If tab switching is slow, remake this
            GridContentArea grid = this.DrawArea;
            grid.ClearContent();

            //get the number of rows and columns this grid should have
            int CellsFitInWidth = grid.GetCellsInWidth();
            int CellsFitInHeight = grid.GetCellsInHeight();

            //generate new grid content based on the size of the window
            for (int cols = 0; cols < (int)CellsFitInWidth; cols++)
                for (int rows = 0; rows < (int)CellsFitInHeight; rows++)
                    PhoneNumberAdd(cols, rows);
        }

        private void DoDemoUpdate()
        {
            //find this random cell and color it
            this.Dispatcher.Invoke(DispatcherPriority.Normal, (Action)(() =>
            {
                MainWindow MainObject = (MainWindow)App.Current.MainWindow;

                if (MainObject == null)
                    return;

                GridContentArea grid = this.DrawArea;

                int Rows = grid.GetCellsInHeight();
                int Cols = grid.GetCellsInWidth();

                //get a random cell
                Random rnd = new Random();
                int RandRow = rnd.Next(0, Rows);
                int RandCol = rnd.Next(0, Cols);

                PhoneNumber RandomElement = RandomElement = grid.PaintArea.Children.Cast<PhoneNumber>().FirstOrDefault(e => Grid.GetRow(e) == RandRow && Grid.GetColumn(e) == RandCol);
                if (RandomElement != null)
                {
                    //try to set random look between simple extension and range
                  /*  if (rnd.Next(0, 10) < 5)
                        RandomElement.SetIsRange(true);
                    else
                        RandomElement.SetIsRange(false);*/

                    //set a random status color
                    int RandStatus = rnd.Next(0, (int)PhoneStatusCodes.NumberOfStatusCodes);
                    if (RandomElement.IsSubscriberRange() == false)
                        RandomElement.SetStatus((PhoneStatusCodes)RandStatus);
                    else
                        RandomElement.SetStatus((PhoneStatusCodes)RandStatus, rnd.Next(0, 9).ToString());

                    //set a random extension
                    RandomElement.SetExtension(rnd.Next(0, 9999).ToString());
                    RandomElement.SetName("Name" + RandomElement.GetExtension().ToString());

                    //can we see name properly ?
                    if (rnd.Next(0, 10) < 5)
                        RandomElement.OnToggleShowName();

                    //note can be seen in the tooltip
                    RandomElement.SetNote("Topcomment" + RandomElement.GetExtension().ToString());

                    //absence status can be seen in the tooltip
                    if (rnd.Next(0, 10) < 5)
                        RandomElement.OnAbsenceStatusUpdate(false);
                    else
                        RandomElement.OnAbsenceStatusUpdate(true);

                    //can we see prefix ?
                    RandomElement.SetPrefix(rnd.Next(0, 9999).ToString());
                    if (rnd.Next(0, 10) < 5)
                        RandomElement.OnToggleShowCanonical();

                    //try to set random forwarding status
                    ForwardStatusStore fss = new ForwardStatusStore(RandomElement.GetExtension());
                    if (rnd.Next(0, 10) < 5)
                        fss.ForwardType = CallForwardingTypes.CallForwardDestination;
                    else
                        fss.ForwardType = CallForwardingTypes.CallForwardNone;
                    RandomElement.OnForwardingChange(fss);
                }
            }));
        }

        /// <summary>
        /// Used for demo mode. Randomly update a phone status
        /// </summary>
        /// <param name="source"></param>
        /// <param name="arg"></param>
        public void PeriodicStatusUpdate(object source, ElapsedEventArgs arg)
        {
            //wait until load finishes. On slow systems too many events build up and will deadlock this update thread
            if (Globals.IndexCardsLoaded == false)
                return;

            //stop timer to avoid event overflows
            UpdateTimer.Stop();

            if (DemoType == 1)
            {
                DoDemoUpdate();
            }
            
            //resume demo timer
            UpdateTimer.Start();
        }

        private bool SetHasValue(HashSet<long> pset, long val)
        {
            foreach (long t in pset)
                if (t == val)
                    return true;
            return false;
        }

        public void SwapPhoneNumbersAtPos(int x1, int y1, int x2, int y2)
        {
            PhoneNumber p1 = PhoneNumberGet(x1, y1);
            PhoneNumber p2 = PhoneNumberGet(x2, y2);
            //sanity check, no point of swapping same cells
            if (p1 == p2)
                return;
            //sanity check, dragging cell out of draw area
            if (p1 == null || p2 == null)
                return;
            Grid.SetColumn(p2, x1);
            Grid.SetRow(p2, y1);
            Grid.SetColumn(p1, x2);
            Grid.SetRow(p1, y2);
            p1.SetCoordinate(x2, y2);
            p2.SetCoordinate(x1, y1);
        }

        private void Click_Context_RenameIndexCard(object sender, RoutedEventArgs e)
        {
            if (CanReceiveFocus() == false)
                return;
            var cw = new IndexCardRename(this);
            cw.ShowInTaskbar = false;
            cw.Owner = Application.Current.MainWindow;
            cw.Show();
        }

        private void Click_Context_DeleteIndexCard(object sender, RoutedEventArgs e)
        {
            if (CanReceiveFocus() == false)
                return;
            if (App.Current != null && App.Current.MainWindow != null)
                (App.Current.MainWindow as MainWindow).DeleteIndexCard(this);
        }

        public void OnStatusColorChanged()
        {
            foreach (var cell in GridCells)
                cell.OnStatusColorChanged();
        }

        public void SetShowGrid(bool show)
        {
           GeneralSettings.ShowGrid = show;

            foreach (var cell in GridCells)
                cell.SetShowGrid(show);
        }

        public void OnToggleShowName()
        {
            if (GeneralSettings.ShowName)
                GeneralSettings.ShowName = false;
            else
                GeneralSettings.ShowName = true;

            foreach (var cell in GridCells)
                cell.OnToggleShowName();
        }

        public void OnToggleShowCanonical()
        {
            if (GeneralSettings.ShowName)
                GeneralSettings.ShowCanonical = false;
            else
                GeneralSettings.ShowCanonical = true;

            foreach (var cell in GridCells)
                cell.OnToggleShowCanonical();
        }

        public void OnCellSizeChanged(double NewWidth, double NewHeight)
        {
            //new cells should be created with updated settings
            GeneralSettings.CellWidth = NewWidth;
            GeneralSettings.CellHeight = NewHeight;

            //draw area should space cells based on new size
            this.DrawArea.SetCellSize(NewWidth, NewHeight);

            //each cell should cahnge it's current size
            foreach (var cell in GridCells)
                cell.OnCellSizeChanged(NewWidth, NewHeight);
        }

        public void OnFontSettingChanged(FontSettings fs)
        {
            //each cell should cahnge it's current size
            foreach (var cell in GridCells)
                cell.OnFontSettingChanged(fs);
        }

        public void OnPrefixChanged(string NewPrefix)
        {
            //new cells should be created with updated settings
            GeneralSettings.Prefix = NewPrefix;

            //each cell should cahnge it's current size
            foreach (var cell in GridCells)
                cell.OnPrefixChanged(NewPrefix);
        }
/*
        public void OnServerConnectionChanged(bool Connected)
        {
            //update device state from the server if a new connection has been established
            if (Connected == true)
                foreach (var cell in GridCells)
                    cell.QueryDeviceStatusFromServer();
        }*/

        private void TranslateLocalize()
        {
            Globals.MultilangManager.TranslateUIComponent(this);
            Globals.MultilangManager.TranslateUIComponent(VisualTab.ContextMenu);
        }

        public void ConvertToAddIndexCardTab()
        {
            SetName("+");
            ContextMenu = null;
        }

        public bool SkipSave()
        {
            if (GetName() == "+")
                return true;
            return false;
        }

        public bool CanReceiveFocus()
        {
            if (GetName() == "+")
                return false;
            return true;
        }
    }
}
