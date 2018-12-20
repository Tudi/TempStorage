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
    /// Interaction logic for GridContentArea.xaml
    /// </summary>
    public partial class GridContentArea : UserControl
    {
        double PhoneNumberWidth;
        double PhoneNumberHeight;
        int CellsInWidth = 0;
        int CellsInHeight = 0;
        public GridContentArea()
        {
            InitializeComponent();

            //initialize with default sizes
            PhoneNumberWidth = GetDefaultCellWidth();
            PhoneNumberHeight = GetDefaultCellHeight();
        }

        public static double GetDefaultCellWidth()
        {
            PhoneNumber cn = new PhoneNumber(-1, -1, new PhoneNumberSetupSettings(), -1);
            return cn.TheControl.Width;
        }

        public static double GetDefaultCellHeight()
        {
            PhoneNumber cn = new PhoneNumber(-1, -1, new PhoneNumberSetupSettings(), -1);
            return cn.TheControl.Height;
        }

        public double GetCellWidth()
        {
            return PhoneNumberWidth;
        }

        public double GetCellHeight()
        {
            return PhoneNumberHeight;
        }

        public void SetCellSize(double NewWidth, double NewHeight)
        {
            PhoneNumberWidth = NewWidth;
            PhoneNumberHeight = NewHeight;
        }
        public double GetGridCellsWidth()
        {
            MainWindow MainObject = (MainWindow)App.Current.MainWindow;
            return MainObject.ActualWidth / GetCellWidth();
        }

        public double GetGridCellsHeight()
        {
            MainWindow MainObject = (MainWindow)App.Current.MainWindow;            
            double MenuHeight = MainObject.MenuObject.ActualHeight + 10;
            double StatusBarHeight = MainObject.StatusbarObject.ActualHeight + 10;
            double AbsenceManageHeight = 0;
            if (MainObject.AbsenceView.Visibility != Visibility.Hidden)
                AbsenceManageHeight = MainObject.AbsenceView.ContentHolder.ActualHeight;

            double TabholderWidth = MainObject.TabHolder.ActualWidth;
            //try to guess the number of tab rows
            double TabItemsWidthSum = 0;
            double TabItemHeight = 20;
            foreach (TabItem i in MainObject.TabHolder.Items)
            {
                TabItemsWidthSum += i.ActualWidth;
                if (i.ActualHeight > TabItemHeight)
                    TabItemHeight = i.ActualHeight;
            }
            double TabRowCount = Math.Ceiling(TabItemsWidthSum / (TabholderWidth+0.0001));
            double TabHolderHeight = TabRowCount * TabItemHeight + 10;

            double ret = (MainObject.ActualHeight - MenuHeight - TabHolderHeight - StatusBarHeight - AbsenceManageHeight) / GetCellHeight();
            if (ret < 0)
                return 0;

            return ret;
        }

        double StartDragPosX = -1, StartDragPosY = -1;
        private void OnMouseDownGrid(object sender, MouseButtonEventArgs e)
        {
            if (e.LeftButton != e.ButtonState)
                return;

            StartDragPosX = Mouse.GetPosition(this).X;
            StartDragPosY = Mouse.GetPosition(this).Y;

            Mouse.OverrideCursor = Cursors.Hand;
        }

        private void OnMouseUpGrid(object sender, MouseButtonEventArgs e)
        {
            if (e.LeftButton != e.ButtonState || StartDragPosX < 0)
                return;

            Mouse.OverrideCursor = null;

            double EndDragPosX = Mouse.GetPosition(this).X;
            double EndDragPosY = Mouse.GetPosition(this).Y;

            //get the cell it was below start drag position
            int StartCellX = (int)(StartDragPosX / GetCellWidth());
            int EndCellX = (int)(EndDragPosX / GetCellWidth());
            int StartCellY = (int)(StartDragPosY / GetCellHeight());
            int EndCellY = (int)(EndDragPosY / GetCellHeight());
            //did we drag a cell over another cell ?
            if (StartCellX != EndCellX || StartCellY != EndCellY)
            {
/*                //yes, we dragged it to a neighbour cell
                MainWindow MainObject = (MainWindow)App.Current.MainWindow;
                IndexCard ic = MainObject.GetVisibleIndexCard();
                if (ic != null)
                    ic.SwapPhoneNumbersAtPos(StartCellX, StartCellY, EndCellX, EndCellY);
                    */
            }
            else
            {
                PhoneNumber pn = Globals.ExtensionManager.PhoneNumberGet(StartCellX, StartCellY);
                Globals.ExtensionManager.OnPhoneNumberClick(pn);
                Globals.AbsenceManage.SetMonitoredEmail(pn.GetEmail());
            }
            StartDragPosX = -1;
        }

        public void ClearContent()
        {
            //get rid of old content
            PaintArea.Children.Clear();
            PaintArea.ColumnDefinitions.Clear();
            PaintArea.RowDefinitions.Clear();
            CellsInWidth = 0;
            CellsInHeight = 0;

            CellsInWidth = (int)GetGridCellsWidth();
            CellsInHeight = (int)GetGridCellsHeight();

            //generate new grid content based on the size of the window
            for (int cols = 0; cols < (int)CellsInWidth; cols++)
                PaintArea.ColumnDefinitions.Add(new ColumnDefinition());

            for (int rows = 0; rows < (int)CellsInHeight; rows++)
                PaintArea.RowDefinitions.Add(new RowDefinition());
        }

        public void AddPhoneNumber(PhoneNumber tcn)
        {
            if (tcn.GetX() > CellsInWidth)
            {
                for (int cols = CellsInWidth; cols < (int)tcn.GetX(); cols++)
                    PaintArea.ColumnDefinitions.Add(new ColumnDefinition());
                CellsInWidth = tcn.GetX();
            }
            if (tcn.GetY() > CellsInHeight)
            {
                for (int rows = CellsInHeight; rows < (int)tcn.GetY(); rows++)
                    PaintArea.RowDefinitions.Add(new RowDefinition());
                CellsInHeight = tcn.GetY();
            }
            PaintArea.Children.Add(tcn);
        }

        public void RemovePhoneNumber(PhoneNumber tcn)
        {
            // Todo : check if we are making our grid smaller
            PaintArea.Children.Remove(tcn);
        }
    }
}
