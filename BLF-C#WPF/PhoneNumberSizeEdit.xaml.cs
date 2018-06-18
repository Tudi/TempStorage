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
    /// Interaction logic for PhoneNumberSizeEdit.xaml
    /// </summary>
    public partial class PhoneNumberSizeEdit : Window
    {
        public PhoneNumberSizeEdit(double Width, double Height)
        {
            InitializeComponent();
            this.CellWidth.Text = Width.ToString();
            this.CellHeight.Text = Height.ToString();
        }

        private void Click_Ok(object sender, RoutedEventArgs e)
        {
            int NewWidth = Int32.Parse(this.CellWidth.Text);
            int NewHeight = Int32.Parse(this.CellHeight.Text);
            if (NewWidth <= 0 || NewHeight >= 500 || NewHeight <= 0 || NewHeight > 500)
            {
                MessageBox.Show("Invalid values");
                return;
            }
            if (App.Current != null && App.Current.MainWindow != null) 
                (App.Current.MainWindow as MainWindow).OnCellSizeChanged(NewWidth, NewHeight);
            this.Close();
        }

        private void Click_Cancel(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void Click_Default(object sender, RoutedEventArgs e)
        {
            this.CellWidth.Text = GridContentArea.GetDefaultCellWidth().ToString();
            this.CellHeight.Text = GridContentArea.GetDefaultCellHeight().ToString();
        }
    }
}
