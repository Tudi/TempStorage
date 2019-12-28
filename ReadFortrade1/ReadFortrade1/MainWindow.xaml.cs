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
        public static TimeoutWatchDog WatchDog = null;
        public static long doubleScaler = 100000;
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
            Globals.WatchDog = new TimeoutWatchDog();
            Globals.vHistory.LoadFromPersistency();
            Task.Factory.StartNew(() => Globals.WatchDog.StartPageTimoutWatchdog());
//            SendNotification.SendMessage("Test meail to sms");
        }
    }
}
