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
        }

        private void Button_Click_OK(object sender, RoutedEventArgs e)
        {
            //need to validate data
            int Port = 0;
            try
            {
                Port = Int32.Parse(this.ServerPort.Text);
            }catch (Exception ex) { };

            //if all good, pass it to main app
            if (App.Current != null && App.Current.MainWindow != null)
                (App.Current.MainWindow as MainWindow).OnBLFServerConfigChange(this.ServerIP.Text, Port);
            this.Close();
        }

        private void Button_Click_Cancel(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
