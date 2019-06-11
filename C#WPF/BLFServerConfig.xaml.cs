using BLFClient.Backend;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections.ObjectModel;
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
using System.Windows.Shapes;

namespace BLFClient
{
    public class ServerConnectionRow
    {
        public string Connected { get; set; }
        public int Enabled { get; set; }
        public string IP { get; set; }
        public int Port { get; set; }
        public string Name { get; set; }
        public int Delete_ { get; set; }
    }

    /// <summary>
    /// Interaction logic for BLFServerConfig.xaml
    /// </summary>
    public partial class BLFServerConfig : Window
    {
        ObservableCollection<ServerConnectionRow> ServerConnectionRows;
        public BLFServerConfig()
        {
            InitializeComponent();

            if (Globals.ConnectionManager == null)
                return;

            ConcurrentBag<ServerConnectionStatus> Connections = Globals.ConnectionManager.GetConnections();

            ServerConnectionRows = new ObservableCollection<ServerConnectionRow>();
            int AddedConnections = 0;
            foreach (ServerConnectionStatus sc in Connections)
            {
                if (sc.PendingRemove == true)
                    continue;
                ServerConnectionRows.Add(new ServerConnectionRow() { Connected = (sc.nclient != null).ToString(), Enabled = sc.Enabled, IP = sc.IP, Port = sc.Port, Name = sc.ServerName, Delete_ = 0 });
                AddedConnections++;
            }
            for(; AddedConnections<10; AddedConnections++)
                ServerConnectionRows.Add(new ServerConnectionRow() { Enabled = 0, IP = "", Port = 5050, Name = "", Delete_ = 0 });
            this.ServerList.ItemsSource = ServerConnectionRows;

            //if we push enter we presume we pushed button "ok"
            RoutedCommand firstSettings = new RoutedCommand();
            firstSettings.InputGestures.Add(new KeyGesture(Key.Enter, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(firstSettings, Button_Click_OK));
            RoutedCommand SecondSettings = new RoutedCommand();
            SecondSettings.InputGestures.Add(new KeyGesture(Key.Escape, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(SecondSettings, Button_Click_Cancel));

            Globals.MultilangManager.TranslateUIComponent(this);
            this.Owner = App.Current.MainWindow;
            this.Left = this.Owner.Left + this.Owner.Width / 2 - this.Width / 2;
            this.Top = this.Owner.Top + this.Owner.Height / 2 - this.Height / 2;
        }

        private void Button_Click_OK(object sender, RoutedEventArgs e)
        {
            Globals.ConnectionManager.UpdateConnectionsDetails(ServerConnectionRows);

            this.Close();
        }

        private void Button_Click_Cancel(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
