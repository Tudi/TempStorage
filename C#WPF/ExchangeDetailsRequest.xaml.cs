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
    /// Interaction logic for ExchangeDetailsRequest.xaml
    /// </summary>
    public partial class ExchangeDetailsRequest : Window
    {
        public ExchangeDetailsRequest()
        {
            InitializeComponent();
            this.tb_Email.Text = Globals.ExchangeAPI.GetEmail();
            this.tb_Email.Focus();

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
            if( Globals.ExchangeAPI.TestConnect(this.tb_Email.Text,this.tb_Passw.Password,"") == false )
                MessageBox.Show(Globals.MultilangManager.GetTranslation("Could not connect to Exchange server"), "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            else
                this.Close();
        }

        private void Button_Click_Cancel(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
