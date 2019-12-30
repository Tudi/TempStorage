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
        public static DBHandler Persistency = null;
        public static ValueHistory vHistory = null;
        public static TimeoutWatchDog TimeoutMonitor = null;
        public static NotificationWatchdog PriceChangeMonitor = null;
        public static long doubleScaler = 100000;
        public static double IgnorePriceChangePCT = 0.0001; // if Old/New is smaller than this ratio, ignore recording it to the DB
    }
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            Globals.Persistency = new DBHandler();
            Globals.vHistory = new ValueHistory();
            Globals.TimeoutMonitor = new TimeoutWatchDog();
            Globals.vHistory.LoadFromPersistency();
            Globals.PriceChangeMonitor = new NotificationWatchdog();
            Task.Factory.StartNew(() => Globals.TimeoutMonitor.StartPageTimoutWatchdog());
            Task.Factory.StartNew(() => Globals.PriceChangeMonitor.StartPriceChangeWatchdog());
            //            SendNotification.SendMessage("Test meail to sms");
        }
    }
}
