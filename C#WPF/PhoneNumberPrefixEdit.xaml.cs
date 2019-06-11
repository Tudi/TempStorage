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
    /// Interaction logic for PhoneNumberPrefixEdit.xaml
    /// </summary>
    public partial class PhoneNumberPrefixEdit : Window
    {
        public PhoneNumberPrefixEdit(string OldPrefix)
        {
            InitializeComponent();
            this.SubscriberPrefix.Text = OldPrefix;
            this.SubscriberPrefix.Focus();

            //if we push enter we presume we pushed button "ok"
            RoutedCommand firstSettings = new RoutedCommand();
            firstSettings.InputGestures.Add(new KeyGesture(Key.Enter, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(firstSettings, Click_Ok));
            RoutedCommand SecondSettings = new RoutedCommand();
            SecondSettings.InputGestures.Add(new KeyGesture(Key.Escape, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(SecondSettings, Click_Cancel));

            Globals.MultilangManager.TranslateUIComponent(this);
            this.Owner = App.Current.MainWindow;
            this.Left = this.Owner.Left + this.Owner.Width / 2 - this.Width / 2;
            this.Top = this.Owner.Top + this.Owner.Height / 2 - this.Height / 2;
        }

        private void Click_Ok(object sender, RoutedEventArgs e)
        {
            if (App.Current != null && App.Current.MainWindow != null)
                (App.Current.MainWindow as MainWindow).OnPrefixChanged(this.SubscriberPrefix.Text);
            this.Close();
        }

        private void Click_Cancel(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
