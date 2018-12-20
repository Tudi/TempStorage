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

            string strStartTime = Globals.Config.GetConfig("AbsenceInfo", "WorkDayStart");
            string strEndTime = Globals.Config.GetConfig("AbsenceInfo", "WorkDayEnd");
            for (int i=1;i<24;i++)
            {
                string StrToAdd;
                if(i<12)
                {
                    string Hour = i.ToString();
                    if (Hour.Length < 2)
                        Hour = "0" + Hour;
                    StrToAdd = Hour + ":00 AM";
                }
                else
                {
                    string Hour = (i - 11).ToString();
                    if (Hour.Length < 2)
                        Hour = "0" + Hour;
                    StrToAdd = Hour + ":00 PM";
                }
                this.StartTime.Items.Add(StrToAdd);
                this.EndTime.Items.Add(StrToAdd);
                if(StrToAdd == strStartTime)
                    this.StartTime.SelectedIndex = i - 1;
                if (StrToAdd == strEndTime)
                    this.EndTime.SelectedIndex = i - 1;
            }

            //set proper selected menu dropdowns
            this.RefreshTime.SelectedIndex = Globals.Config.GetConfigInt("AbsenceInfo", "RefreshTime", 1) - 1;
            this.CheckedDays.SelectedIndex = Globals.Config.GetConfigInt("AbsenceInfo", "NumCheckedDays", 1) - 1;

            //if we push enter we presume we pushed button "ok"
            RoutedCommand firstSettings = new RoutedCommand();
            firstSettings.InputGestures.Add(new KeyGesture(Key.Enter, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(firstSettings, Button_Click_OK));
            RoutedCommand SecondSettings = new RoutedCommand();
            SecondSettings.InputGestures.Add(new KeyGesture(Key.Escape, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(SecondSettings, Button_Click_Cancel));

            TranslateLocalize();

            this.Owner = App.Current.MainWindow;
        }

        private void Button_Click_OK(object sender, RoutedEventArgs e)
        {
            Button_Click_Apply(sender, e);
            this.Close();
        }

        private void Button_Click_Apply(object sender, RoutedEventArgs e)
        {
            Globals.Config.SetConfig("AbsenceInfo", "RefreshTime", (this.RefreshTime.SelectedIndex + 1).ToString());
            Globals.Config.SetConfig("AbsenceInfo", "NumCheckedDays", this.CheckedDays.SelectionBoxItem.ToString());
            Globals.Config.SetConfig("AbsenceInfo", "WorkDayStart", this.StartTime.SelectionBoxItem.ToString());
            Globals.Config.SetConfig("AbsenceInfo", "WorkDayEnd", this.EndTime.SelectionBoxItem.ToString());
/*
            am.SetHours(this.StartTime.SelectedIndex + 1, this.EndTime.SelectedIndex + 1);
            bool ShowDialog = (bool)this.cb_ShowOutlookDialog.IsChecked;
            am.SetOutlookSettings(ShowDialog, "");  //maybe this should be an email here ?*/
        }

        private void Button_Click_Cancel(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void Button_Click_SelectProfile(object sender, RoutedEventArgs e)
        {
            var wind = new AbsenceManageSelectOutlook();
            wind.Show();
        }

        private void TranslateLocalize()
        {
            Globals.MultilangManager.TranslateUIComponent(this);
        }
    }
}
