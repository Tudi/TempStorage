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
    public class PhoneNumberSetupSettings
    {
        public bool ShowGrid;
        public bool ShowName;
        public bool ShowCanonical;
        public double CellWidth, CellHeight;
        public System.Windows.Media.FontFamily FontFamily;
        public double FontSize;
        public FontWeight FontWeight_;
        public FontStyle FontStyle_;
        public string Prefix;
    }

    /// <summary>
    /// Interaction logic for IndexCard.xaml
    /// </summary>
    public partial class IndexCard : UserControl
    {
        //string shown on the index card
        string IndexCardName;
        //only show the selected index card
        bool IsCardVisible;

        PhoneNumberSetupSettings GeneralSettings;

        //used only if there is a demo going on
        Timer DemoTimer;
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
            GeneralSettings.FontSize = 0; //default until manually set

            GridCells = new List<PhoneNumber>();

            //create a tabitem that we will show in the "tabholder" tabbar
            VisualTab = new TabItem();
            VisualTab.Content = this;
            VisualTab.ContextMenu = Resources["contextMenu"] as ContextMenu;
            VisualTab.Style = Resources["tabItem"] as Style;

            //not used, just for the sake of sanity
            SetName("Unsaved new");
            SetVisible(false);

            //show ourself in the tabholder
            MainWindow MainObject = (MainWindow)App.Current.MainWindow;
            MainObject.TabHolder.Items.Add(VisualTab);
        }

        /// <summary>
        /// Save Index card content to file
        /// </summary>
        ~IndexCard()
        {
            OnDelete();
        }

        public string GetName()
        {
            return IndexCardName;
        }

        public void OnDelete()
        {
            //stop the demo timer
            if (DemoTimer != null)
            {
                DemoTimer.Stop();
                DemoTimer = null;
            }

            //tell the cells to suicide
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
                DemoTimer = new Timer(1000) { Enabled = true };
                DemoTimer.Elapsed += new ElapsedEventHandler(PeriodicStatusUpdate);
            }
        }

        /// <summary>
        /// Set this card visible. Main window will draw the first visible index card
        /// </summary>
        /// <param name="pIsVisible"></param>
        public void SetVisible(bool pIsVisible)
        {
            IsCardVisible = pIsVisible;
            /*
                        if (IsCardVisible)
                            IndexCardButton.Background = Brushes.Gray;
                        else
                            IndexCardButton.Background = Brushes.LightGray;
            */
            //continue demo on this index card if we became visible again
            if (DemoTimer != null)
            {
                if (IsCardVisible == true)
                    DemoTimer.Start();
                //pause the demo until we regain visibility
                if (IsCardVisible == false)
                    DemoTimer.Stop();
            }
        }

        /// <summary>
        /// Load the content of the index card from a file stream
        /// </summary>
        public void Load()
        {
            //load Index card related data
            //load all possible grid cells ( phone numbers )
        }

        /// <summary>
        /// Save all index card related data to file
        /// </summary>
        public void Save()
        {
            //save Index card related data
            //iterate through all GridCells ( phone numbers ) and save them 1 by 1
        }

        /// <summary>
        /// Set the name of the index card. Should update visually also
        /// </summary>
        /// <param name="NewName"></param>
        public void SetName(string NewName)
        {
            IndexCardName = NewName;
            VisualTab.Header = NewName;
        }

        /// <summary>
        /// Show loaded cells and contents
        /// </summary>
        public void ShowIndexCardContent()
        {
            //nothing to show unless we are the window that should get painted
            if (IsCardVisible == false)
                return;
            //a simple full repaint
            CreateGridBasedOnSize();
        }

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
            GridCells.Remove(PhoneNumberGet(x, y));
        }

        public void PhoneNumberAdd(int x, int y)
        {
            PhoneNumber tcn = PhoneNumberGet(x, y);
            if (tcn == null)
            {
                tcn = new PhoneNumber(x, y, GeneralSettings);

                if (DemoType == 1 && (x + y) % 2 == 0)
                {
                    tcn.SetExtension(x + y);
                    if ((x + y) % 3 == 0)
                        tcn.SetIsRange(true);
                }
                else if (DemoType == 2)
                    tcn.SetExtension(x + y);

                tcn.OnFontSettingChanged(GeneralSettings);

                AddPhoneNumber(tcn);
                Grid.SetColumn(tcn, x);
                Grid.SetRow(tcn, y);
            }
            DrawArea.AddPhoneNumber(tcn);
        }

        /// <summary>
        /// On resize of the window, we want to show as many as possible phone numbers
        /// </summary>
        private void CreateGridBasedOnSize()
        {
            MainWindow MainObject = (MainWindow)App.Current.MainWindow;
            GridContentArea grid = this.DrawArea;

            grid.ClearContent();

            //get the number of rows and columns this grid should have
            double CellsFitInWidth = grid.GetGridCellsWidth();
            double CellsFitInHeight = grid.GetGridCellsHeight();

            //generate new grid content based on the size of the window
            for (int cols = 0; cols < (int)CellsFitInWidth; cols++)
            {
                for (int rows = 0; rows < (int)CellsFitInHeight; rows++)
                {
                    PhoneNumberAdd(cols, rows);
                }
            }
        }

        /// <summary>
        /// Used for demo mode. Randomly update a phone status
        /// </summary>
        /// <param name="source"></param>
        /// <param name="arg"></param>
        public void PeriodicStatusUpdate(object source, ElapsedEventArgs arg)
        {
            //stop timer to avoid event overflows
            DemoTimer.Stop();

            //find this random cell and color it
            this.Dispatcher.Invoke(DispatcherPriority.Normal, (Action)(() =>
            {
                MainWindow MainObject = (MainWindow)App.Current.MainWindow;

                if (MainObject == null)
                    return;

                GridContentArea grid = this.DrawArea;

                int Rows = grid.PaintArea.RowDefinitions.Count;
                int Cols = grid.PaintArea.ColumnDefinitions.Count;

                //get a random cell
                Random rnd = new Random();
                int RandRow = rnd.Next(0, Rows);
                int RandCol = rnd.Next(0, Cols);

                PhoneNumber RandomElement = RandomElement = grid.PaintArea.Children.Cast<PhoneNumber>().FirstOrDefault(e => Grid.GetRow(e) == RandRow && Grid.GetColumn(e) == RandCol);
                if (RandomElement != null)
                {
                    int RandStatus = rnd.Next(0, (int)PhoneStatusCodes.NumberOfStatusCodes);
                    if(RandomElement.IsSubscriberRange() == false)
                        RandomElement.SetStatus((PhoneStatusCodes)RandStatus);
                    else
                        RandomElement.SetRangeStatus(rnd.Next(0, 9),(PhoneStatusCodes)RandStatus);
                    RandomElement.SetExtension(rnd.Next(0, 9999));
                }
            }));

            //resume demo timer
            DemoTimer.Start();
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
            Grid.SetColumn(p1, x2);
            Grid.SetRow(p1, y2);
            Grid.SetColumn(p2, x1);
            Grid.SetRow(p2, y1);
            p1.SetCoordinate(x2, y2);
            p2.SetCoordinate(x1, y1);
        }

        private void Click_Context_RenameIndexCard(object sender, RoutedEventArgs e)
        {
            var cw = new IndexCardRename(this);
            cw.ShowInTaskbar = false;
            cw.Owner = Application.Current.MainWindow;
            cw.Show();
        }

        private void Click_Context_DeleteIndexCard(object sender, RoutedEventArgs e)
        {
            if (App.Current != null && App.Current.MainWindow != null)
                (App.Current.MainWindow as MainWindow).DeleteIndexCard(this);
        }

        public void OnStatusColorChanged()
        {
            foreach (var cell in GridCells)
                cell.OnStatusColorChanged();
        }

        public void OnGridToggle()
        {
            if (GeneralSettings.ShowGrid)
                GeneralSettings.ShowGrid = false;
            else
                GeneralSettings.ShowGrid = true;

            foreach (var cell in GridCells)
                cell.OnGridToggle();
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

        public void OnFontSettingChanged(TabSetupSettings settings)
        {
            //new cells should be created with updated settings
            GeneralSettings.FontFamily = settings.FontFamily;
            GeneralSettings.FontSize = settings.FontSize;
            GeneralSettings.FontWeight_ = settings.FontWeight_;
            GeneralSettings.FontStyle_ = settings.FontStyle_;

            //each cell should cahnge it's current size
            foreach (var cell in GridCells)
                cell.OnFontSettingChanged(GeneralSettings);
        }

        public void OnPrefixChanged(string NewPrefix)
        {
            //new cells should be created with updated settings
            GeneralSettings.Prefix = NewPrefix;

            //each cell should cahnge it's current size
            foreach (var cell in GridCells)
                cell.OnPrefixChanged(NewPrefix);
        }
    }
}
