using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
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

namespace ReadFortrade1
{
    public class Globals
    {
        public static List<StockDataHistory> DataHistory = new List<StockDataHistory>();
        public static ValueHistory vHistory = new ValueHistory();
        public static TimeoutWatchDog WatchDog =  new TimeoutWatchDog();
        public static DBHandler Persistency = new DBHandler();
    }
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        public MainWindow()
        {
            InitializeComponent();
            Task.Factory.StartNew(() => Globals.WatchDog.StartPageTimoutWatchdog());
//            SendNotification.SendMessage("Test meail to sms");
        }
    }
}
