using System;
using System.Collections.Generic;
using System.IO;
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
using System.Windows.Threading;

namespace CSVIngester
{
    public class GlobalVariables
    {
        public static DBHandler DBStorage = null;
        public static string ImportingToDBBlock = "";
        public static MessageLogger Logger = null;
        public static int NULLValue = -9999999; // has to fit into a float without truncations
        public static Window MyMainWindow = null;
    }
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            GlobalVariables.Logger = new MessageLogger();
            GlobalVariables.DBStorage = new DBHandler();
            GlobalVariables.Logger.Log("Application started..");
            GlobalVariables.MyMainWindow = this;
            ExportStartDate.SelectedDate = new DateTime(2001, 1, 1);
            ExportEndDate.SelectedDate = DateTime.Now;
            AExportStartDate.SelectedDate = new DateTime(2001, 1, 1);
            AExportEndDate.SelectedDate = DateTime.Now;
            DBBusyMonitorThreadStart();
        }

        ~MainWindow()
        {
            GlobalVariables.MyMainWindow = null;
        }
        private void DBBusyMonitorThreadStart()
        {
            var thread = new Thread(new ThreadStart(DBBusyMonitorThread));
//            thread.SetApartmentState(ApartmentState.STA);
            thread.Start();
        }

        private void DBBusyMonitorThread()
        {
            int PrevValue = 1;
//            Cursor PrevCursor = null;
            while (GlobalVariables.MyMainWindow != null && Application.Current != null && Application.Current.Dispatcher != null)
            {
                if (GlobalVariables.ImportingToDBBlock != "" && PrevValue != 0)
                {
                    //Mouse.OverrideCursor = Cursors.Wait;
                    //GlobalVariables.MyMainWindow.Cursor = Cursors.Wait;
//                    PrevCursor = GlobalVariables.MyMainWindow.Cursor;
//                    Application.Current.Dispatcher.Invoke(new Action(() => this.Cursor = Cursors.Wait));
                    Application.Current.Dispatcher.Invoke(new Action(() => Mouse.OverrideCursor = Cursors.Wait));
                    Application.Current.Dispatcher.Invoke(new Action(() => ConsoleTextbox.Background = Brushes.LightBlue));
                    PrevValue = 0;
                }
                else if (GlobalVariables.ImportingToDBBlock == "" && PrevValue != 1)
                {
                    //Mouse.OverrideCursor = Cursors.None;
                    //GlobalVariables.MyMainWindow.Cursor = null;
//                    Application.Current.Dispatcher.Invoke(new Action(() => this.Cursor = PrevCursor));
                    Application.Current.Dispatcher.Invoke(new Action(() => Mouse.OverrideCursor = null));
                    Application.Current.Dispatcher.Invoke(new Action(() => ConsoleTextbox.Background = Brushes.White));
                    PrevValue = 1;
                }
                else
                    Thread.Sleep(1);
            }
        }

        private void PopupFileSelect_Click(object sender,RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.DefaultExt = ".csv";
            dlg.Filter = "CSV Files (*.csv)|*.csv";
            Nullable<bool> result = dlg.ShowDialog();
            if (result == true)
            {
                string filename = dlg.FileName;
                FileImportLocation.Text = filename;
            }
        }

        private void FileImportProcess_Click(object sender, RoutedEventArgs e)
        {
            if (File.Exists(FileImportLocation.Text) == false)
            {
                GlobalVariables.Logger.Log("Could not open file to import");
                return;
            }

            GlobalVariables.Logger.Log("File import started");

            string ThreadParam = FileImportLocation.Text;
            if (FIG1.IsChecked == true)
                Task.Factory.StartNew(() => ReadCSVFile.ReadInvenotryCSVFile(ThreadParam));
            else if (FIG2.IsChecked == true)
                Task.Factory.StartNew(() => ReadCSVFile.ReadVATCSVFile(ThreadParam));
            else if (FIG3.IsChecked == true)
                Task.Factory.StartNew(() => ReadCSVFile.ReadAmazonOrdersCSVFile(ThreadParam,"Amazon_Orders", "AMAZON-ORDERS", true));
            else if (FIG4.IsChecked == true)
                Task.Factory.StartNew(() => ReadCSVFile.ReadAmazonOrdersCSVFile(ThreadParam, "Amazon_Refunds", "AMAZON-REFUNDS", false));
            else if (FIG5.IsChecked == true)
                Task.Factory.StartNew(() => ReadCSVFile.ReadPaypalSalesCSVFile(ThreadParam));
        }

        private void DeleteSelectedDatabase_Click(object sender, RoutedEventArgs e)
        {
            string TableName = "";
            if (DDG1.IsChecked == true)
                TableName = "Inventory";
            else if (DDG2.IsChecked == true)
                TableName = "Amazon-Orders";
            else if (DDG3.IsChecked == true)
                TableName = "Amazon-Refunds";
            else if (DDG4.IsChecked == true)
                TableName = "Paypal-Sales";
            else if (DDG5.IsChecked == true)
                TableName = "Paypal-Refunds";

            MessageBoxResult messageBoxResult = System.Windows.MessageBox.Show("Are you sure you wish to empty "+ TableName +" Database ? ", "Delete Confirmation", System.Windows.MessageBoxButton.YesNo);
            if (messageBoxResult == MessageBoxResult.Yes)
            {
                if (DDG1.IsChecked == true)
                    GlobalVariables.DBStorage.ClearInventory();
                else if (DDG2.IsChecked == true)
                    GlobalVariables.DBStorage.ClearAmazonOrders();
                else if (DDG3.IsChecked == true)
                    GlobalVariables.DBStorage.ClearAmazonRefunds();
                else if (DDG4.IsChecked == true)
                    GlobalVariables.DBStorage.ClearPaypalSales();
                else if (DDG5.IsChecked == true)
                    GlobalVariables.DBStorage.ClearPaypalRefunds();
            }
        }

        private void CreateRaportButton_Click(object sender, RoutedEventArgs e)
        {
            if (GlobalVariables.ImportingToDBBlock.Length != 0)
            {
                GlobalVariables.Logger.Log("Another thread is already importing in table " + GlobalVariables.ImportingToDBBlock + ". Please wait until it finishes");
                return;
            }

            if (RTG1.IsChecked == true)
            {
                GlobalVariables.Logger.Log("Exporting 'inventory' table - started");
                GlobalVariables.DBStorage.ExportInventoryTable();
                GlobalVariables.Logger.Log("Exporting 'inventory' table - Finished");
            }
            else if (RTG2.IsChecked == true)
            {
                GlobalVariables.Logger.Log("Exporting 'AMAZON-ORDERS' table - started");
                GlobalVariables.DBStorage.ExportAmazonOrdersTable("Amazon_Orders", "AMAZON-ORDERS", ExportStartDate.SelectedDate.Value, ExportEndDate.SelectedDate.Value);
                GlobalVariables.Logger.Log("Exporting 'AMAZON-ORDERS' table - Finished");
            }
            else if (RTG3.IsChecked == true)
            {
                GlobalVariables.Logger.Log("Exporting 'AMAZON-REFUNDS' table - started");
                GlobalVariables.DBStorage.ExportAmazonOrdersTable("Amazon_Refunds", "AMAZON-REFUNDS", ExportStartDate.SelectedDate.Value, ExportEndDate.SelectedDate.Value);
                GlobalVariables.Logger.Log("Exporting 'AMAZON-REFUNDS' table - Finished");
            }
            else if (RTG4.IsChecked == true)
            {
                GlobalVariables.Logger.Log("Exporting 'PAYPAL-SALES' table - started");
                GlobalVariables.DBStorage.ExportPaypalSales("PAYPAL_SALES", "PAYPAL-SALES", ExportStartDate.SelectedDate.Value, ExportEndDate.SelectedDate.Value);
                GlobalVariables.Logger.Log("Exporting 'PAYPAL-SALES' table - Finished");
            }
            else if (RTG5.IsChecked == true)
            {
                GlobalVariables.Logger.Log("Exporting 'PAYPAL-REFUNDS' table - started");
                GlobalVariables.DBStorage.ExportPaypalSales("PAYPAL_REFUNDS", "PAYPAL-REFUNDS", ExportStartDate.SelectedDate.Value, ExportEndDate.SelectedDate.Value);
                GlobalVariables.Logger.Log("Exporting 'PAYPAL-REFUNDS' table - Finished");
            }
        }

        private void Window_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            ConsoleTextbox.Width = this.Width - 25;
            ConsoleTextbox.Height = this.Height - 315;
        }

        private void UpdateVatButton_Click(object sender, RoutedEventArgs e)
        {
            Task.Factory.StartNew(() => GlobalVariables.DBStorage.UpdateVAT());
        }

        private void CreateAccountingRaportButton_Click(object sender, RoutedEventArgs e)
        {
            if (GlobalVariables.ImportingToDBBlock.Length != 0)
            {
                GlobalVariables.Logger.Log("Another thread is already importing in table " + GlobalVariables.ImportingToDBBlock + ". Please wait until it finishes");
                return;
            }

            if (ARTG1.IsChecked == true)
            {
                GlobalVariables.Logger.Log("Exporting 'SALES' report - started");
                GlobalVariables.DBStorage.ExportAccountingSalesReport("PAYPAL_SALES", "SALES", AExportStartDate.SelectedDate.Value, AExportEndDate.SelectedDate.Value);
                GlobalVariables.Logger.Log("Exporting 'SALES' report - Finished");
            }
            else if (ARTG2.IsChecked == true)
            {
                GlobalVariables.Logger.Log("Exporting 'SALES RETURNS' report - started");
                GlobalVariables.DBStorage.ExportAccountingSalesReport("PAYPAL_REFUNDS", "SALES RETURNS", AExportStartDate.SelectedDate.Value, AExportEndDate.SelectedDate.Value);
                GlobalVariables.Logger.Log("Exporting 'SALES RETURNS' report - Finished");
            }
            else if (ARTG3.IsChecked == true)
            {
                GlobalVariables.Logger.Log("Exporting 'PURCHASES' report - started");
                GlobalVariables.DBStorage.ExportAmazonAccountingSalesReport("Amazon_Orders", "PURCHASES", AExportStartDate.SelectedDate.Value, AExportEndDate.SelectedDate.Value);
                GlobalVariables.Logger.Log("Exporting 'PURCHASES' report - Finished");
            }
            else if (ARTG4.IsChecked == true)
            {
                GlobalVariables.Logger.Log("Exporting 'PURCHASES RETURNS' report - started");
                GlobalVariables.DBStorage.ExportAmazonAccountingSalesReport("Amazon_Refunds", "PURCHASES RETURNS", AExportStartDate.SelectedDate.Value, AExportEndDate.SelectedDate.Value);
                GlobalVariables.Logger.Log("Exporting 'PURCHASES RETURNS' report - Finished");
            }
        }

        private void DeleteNonShippedButton_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.DefaultExt = ".csv";
            dlg.Filter = "CSV Files (*.csv)|*.csv";
            Nullable<bool> result = dlg.ShowDialog();
            if (result == true)
            {
                string filename = dlg.FileName;
                Task.Factory.StartNew(() => ReadCSVFile.ReadAmazonDeleteCSVFile(filename));
            }
        }
    }
}
