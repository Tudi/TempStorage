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
    /// Interaction logic for IndexCardRename.xaml
    /// </summary>
    public partial class IndexCardRename : Window
    {
        IndexCard RenameWho;
        public IndexCardRename(IndexCard sender)
        {
            InitializeComponent();
            RenameWho = sender;
            this.IndexCardNewName.Text = sender.GetName();
        }

        private void Button_AcceptRename(object sender, RoutedEventArgs e)
        {
            RenameWho.SetName(this.IndexCardNewName.Text);
            this.Close();
        }

        private void Button_DiscardRename(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
