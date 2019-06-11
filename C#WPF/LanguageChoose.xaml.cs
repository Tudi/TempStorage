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
    /// Interaction logic for LanguageChoose.xaml
    /// </summary>
    public partial class LanguageChoose : Window
    {
        public LanguageChoose()
        {
            InitializeComponent();

            //if we push enter we presume we pushed button "ok"
            RoutedCommand SecondSettings = new RoutedCommand();
            SecondSettings.InputGestures.Add(new KeyGesture(Key.Escape, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(SecondSettings, CloseWindow));

            Globals.MultilangManager.TranslateUIComponent(this);
            this.Owner = App.Current.MainWindow;
            this.Left = this.Owner.Left + this.Owner.Width / 2 - this.Width / 2;
            this.Top = this.Owner.Top + this.Owner.Height / 2 - this.Height / 2;
        }

        private void CloseWindow(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void Button_Click_En(object sender, RoutedEventArgs e)
        {
            Globals.IniFile.SetConfig("Options", "Language", "ENG");
            Globals.IniFile.SaveIni();
            this.Close();
        }

        private void Button_Click_Ger(object sender, RoutedEventArgs e)
        {
            Globals.IniFile.SetConfig("Options", "Language", "GER");
            Globals.IniFile.SaveIni();
            this.Close();
        }

        private void Button_Click_It(object sender, RoutedEventArgs e)
        {
            Globals.IniFile.SetConfig("Options", "Language", "ITA");
            Globals.IniFile.SaveIni();
            this.Close();
        }
    }
}
