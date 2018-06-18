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
    /// Interaction logic for AdminChangePassw.xaml
    /// </summary>
    public partial class AdminChangePassw : Window
    {
        public AdminChangePassw()
        {
            InitializeComponent();
        }

        private void Button_Click_OK(object sender, RoutedEventArgs e)
        {
            if(this.NewPassw.Text != this.NewPasswConfirm.Text)
            {
                MessageBox.Show("New and confirm password does not match");
                return;
            }
            //only close window if passw got changed
            if (App.Current != null && App.Current.MainWindow != null)
                if ((App.Current.MainWindow as MainWindow).OnAdminChangePassw(this.OldPassw.Text, this.NewPassw.Text))
                {
                    this.Close();
                }
        }

        private void Button_Click_Cancel(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
