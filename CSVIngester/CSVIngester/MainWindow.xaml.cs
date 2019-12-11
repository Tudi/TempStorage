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
            ExportStartDate.SelectedDate = new DateTime(2001, 1, 1);
            ExportEndDate.SelectedDate = DateTime.Now;
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
//FileImportLocation.Text="D:\\MyStuff\\TempStorage\\CSVIngester\\FilesFromContrator\\test files\\inventory.csv";
//FileImportLocation.Text= "D:\\MyStuff\\TempStorage\\CSVIngester\\FilesFromContrator\\test files\\EBAY VAT.csv";
//FileImportLocation.Text= "D:\\MyStuff\\TempStorage\\CSVIngester\\FilesFromContrator\\test files\\ASIN VAT.csv";

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
                Task.Factory.StartNew(() => ReadCSVFile.ReadAmazonOrdersCSVFile(ThreadParam,"Amazon_Orders", "Amazon-Orders", true));
            else if (FIG4.IsChecked == true)
                Task.Factory.StartNew(() => ReadCSVFile.ReadAmazonOrdersCSVFile(ThreadParam, "Amazon_Refunds", "Amazon-Refunds", false));
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
            if (RTG1.IsChecked == true)
            {
                GlobalVariables.Logger.Log("Exporting 'inventory' table - started");
                GlobalVariables.DBStorage.ExportInventoryTable();
                GlobalVariables.Logger.Log("Exporting 'inventory' table - Finished");
            }
            else if (RTG2.IsChecked == true)
            {
                GlobalVariables.Logger.Log("Exporting 'AMAZON-ORDERS' table - started");
                GlobalVariables.DBStorage.ExportAmazonOrdersTable("Amazon_Orders", "Amazon-Orders", ExportStartDate.SelectedDate.Value, ExportEndDate.SelectedDate.Value);
                GlobalVariables.Logger.Log("Exporting 'AMAZON-ORDERS' table - Finished");
            }
            else if (RTG3.IsChecked == true)
            {
                GlobalVariables.Logger.Log("Exporting 'AMAZON-REFUNDS' table - started");
                GlobalVariables.DBStorage.ExportAmazonOrdersTable("Amazon_Refunds", "Amazon-Refunds", ExportStartDate.SelectedDate.Value, ExportEndDate.SelectedDate.Value);
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
    }
}
