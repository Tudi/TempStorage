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
