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
using System.Windows.Shapes;

namespace BLFClient
{
    /// <summary>
    /// Interaction logic for BLFServerConfig.xaml
    /// </summary>
    public partial class BLFServerConfig : Window
    {
        public BLFServerConfig(string OldIp, int OldPort)
        {
            InitializeComponent();
            this.ServerIP.Text = OldIp;
            this.ServerPort.Text = OldPort.ToString();

            //if we push enter we presume we pushed button "ok"
            RoutedCommand firstSettings = new RoutedCommand();
            firstSettings.InputGestures.Add(new KeyGesture(Key.Enter, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(firstSettings, Button_Click_OK));
            RoutedCommand SecondSettings = new RoutedCommand();
            SecondSettings.InputGestures.Add(new KeyGesture(Key.Escape, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(SecondSettings, Button_Click_Cancel));

            Globals.MultilangManager.TranslateUIComponent(this);
            this.Owner = App.Current.MainWindow;
        }

        private void Button_Click_OK(object sender, RoutedEventArgs e)
        {
            //need to validate data
            int Port = 0;
            try
            {
                Port = Int32.Parse(this.ServerPort.Text);
            }catch { };

            //should do some basic checks on this
            string IP = this.ServerIP.Text;
            Globals.Config.SetConfig("Options", "BlfServerIp", IP);
            Globals.Config.SetConfig("Options", "BlfServerPort", Port.ToString());
            Globals.ConnectionManager.SetConnectionDetails(IP, Port);
            Globals.ConnectionManager.Disconnect(); // supervisor thread should reconnect. Will not block main htread
            //if it has not  yet been set
            if (Globals.IniFile.GetConfig("Options", "ServerAddress", "").Length == 0)
            {
                Globals.IniFile.SetConfig("Options", "ServerAddress", IP);
                Globals.IniFile.SetConfig("Options", "ServerListeningPort", Port.ToString());
                Globals.IniFile.SaveIni();
            }

            this.Close();
        }

        private void Button_Click_Cancel(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
