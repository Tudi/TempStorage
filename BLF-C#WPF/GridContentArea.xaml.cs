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
        public GridContentArea()
        {
            InitializeComponent();

            //initialize with default sizes
            PhoneNumberWidth = GetDefaultCellWidth();
            PhoneNumberHeight = GetDefaultCellHeight();
        }

        public static double GetDefaultCellWidth()
        {
            PhoneNumber cn = new PhoneNumber(-1, -1, new PhoneNumberSetupSettings());
            return cn.TheControl.Width;
        }

        public static double GetDefaultCellHeight()
        {
            PhoneNumber cn = new PhoneNumber(-1, -1, new PhoneNumberSetupSettings());
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
            return MainObject.Width / GetCellWidth();
        }

        public double GetGridCellsHeight()
        {
            MainWindow MainObject = (MainWindow)App.Current.MainWindow;
            double MenuHeight = 30;
            double TabHolderHeight = 30;
            double StatusBarHeight = 40;
            double AbsenceManageHeight = 0;
            if (MainObject.AbsenceView.Visibility != Visibility.Hidden)
                AbsenceManageHeight = MainObject.AbsenceView.ContentHolder.Height;
            return (MainObject.Height - MenuHeight - TabHolderHeight - StatusBarHeight - AbsenceManageHeight) / GetCellHeight();
        }

        double StartDragPosX, StartDragPosY;
        private void OnMouseDownGrid(object sender, MouseButtonEventArgs e)
        {
            StartDragPosX = Mouse.GetPosition(this).X;
            StartDragPosY = Mouse.GetPosition(this).Y;
        }

        private void OnMouseUpGrid(object sender, MouseButtonEventArgs e)
        {
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
                //yes, we dragged it to a neighbour cell
                MainWindow MainObject = (MainWindow)App.Current.MainWindow;
                IndexCard ic = MainObject.GetVisibleIndexCard();
                if (ic != null)
                    ic.SwapPhoneNumbersAtPos(StartCellX, StartCellY, EndCellX, EndCellY);
            }
        }

        public void ClearContent()
        {
            //get rid of old content
            PaintArea.Children.Clear();
            PaintArea.ColumnDefinitions.Clear();
            PaintArea.RowDefinitions.Clear();

            double CellsFitInWidth = GetGridCellsWidth();
            double CellsFitInHeight = GetGridCellsHeight();

            //generate new grid content based on the size of the window
            for (int cols = 0; cols < (int)CellsFitInWidth; cols++)
                PaintArea.ColumnDefinitions.Add(new ColumnDefinition());

            for (int rows = 0; rows < (int)CellsFitInHeight; rows++)
                PaintArea.RowDefinitions.Add(new RowDefinition());
        }

        public void AddPhoneNumber(PhoneNumber tcn)
        {
            PaintArea.Children.Add(tcn);
        }
    }
}
