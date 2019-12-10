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
        public static bool ImportingToDBBlock = false;
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
        }

        private void DeleteSelectedDatabase_Click(object sender, RoutedEventArgs e)
        {
            if (FIG1.IsChecked == true)
                GlobalVariables.DBStorage.ClearInventory();
        }

        private void CreateRaportButton_Click(object sender, RoutedEventArgs e)
        {
            GlobalVariables.Logger.Log("Exporting 'inventory' table - started");
            GlobalVariables.DBStorage.ExportInventoryTable();
            GlobalVariables.Logger.Log("Exporting 'inventory' table - Finished");
        }

        private void Window_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            ConsoleTextbox.Width = this.Width - 25;
            ConsoleTextbox.Height = this.Height - 315;
        }
    }
}
