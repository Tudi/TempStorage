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
    /// Interaction logic for OptisetNew.xaml
    /// </summary>
    public partial class OptisetNew : Window
    {
        public OptisetNew()
        {
            InitializeComponent();
            if (Globals.Config.GetConfig("Options", "Optiset", "NO") == "YES")
                this.UseOptisetExtension.IsChecked = true;

            this.OptisetExtension.Text = Globals.Config.GetConfig("Options", "OptisetExtension", "");

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
            if(this.UseOptisetExtension.IsChecked == true)
                Globals.Config.SetConfig("Options", "Optiset", "YES");
            else
                Globals.Config.SetConfig("Options", "Optiset", "NO");

            Globals.Config.SetConfig("Options", "OptisetExtension", this.OptisetExtension.Text);

            Globals.Config.SaveIni();

            Globals.ExtensionManager.CreateOptisetPhantomExtension();

            this.Close();
        }

        private void Button_Click_Cancel(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
