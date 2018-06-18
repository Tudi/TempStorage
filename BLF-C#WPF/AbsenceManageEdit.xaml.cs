using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
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
    /// Interaction logic for AbsenceManageEdit.xaml
    /// </summary>
    public partial class AbsenceManageEdit : Window
    {
        public AbsenceManageEdit()
        {
            InitializeComponent();
            for(int i=1;i<24;i++)
            {
                if(i<12)
                {
                    string Hour = i.ToString();
                    if (Hour.Length < 2)
                        Hour = "0" + Hour;
                    this.StartTime.Items.Add(Hour + ":00 AM");
                    this.EndTime.Items.Add(Hour + ":00 AM");
                }
                else
                {
                    string Hour = (i - 11).ToString();
                    if (Hour.Length < 2)
                        Hour = "0" + Hour;
                    this.StartTime.Items.Add(Hour + ":00 PM");
                    this.EndTime.Items.Add(Hour + ":00 PM");
                }
            }
        }

        private void Button_Click_OK(object sender, RoutedEventArgs e)
        {
            AbsenceManage am = new AbsenceManage();
            am.SetHours(this.StartTime.SelectedIndex + 1, this.EndTime.SelectedIndex + 1);
            am.SetCheckedDays(this.CheckedDays.SelectedIndex + 1);
            am.SetRefreshTime(this.RefreshTime.SelectedIndex + 1);
            bool ShowDialog = (bool)this.cb_ShowOutlookDialog.IsChecked;
            am.SetOutlookSettings(ShowDialog, "");  //maybe this should be an email here ?

            this.Close();
        }

        private void Button_Click_Apply(object sender, RoutedEventArgs e)
        {

        }

        private void Button_Click_Cancel(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
