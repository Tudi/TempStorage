using BLFClient.Backend;
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
    /// Interaction logic for PhoneNumberNew.xaml
    /// </summary>
    public partial class PhoneNumberNew : Window
    {
        long EditedGUID = -1;
        PhoneNumberSetupSettings GeneralSettings;
        FontSettings FontSetting = null;
        ReadOnlyCollection<PersPortDataStore> possibleNames = null;

        public PhoneNumberNew(PhoneNumber EditedNumber)
        {
            InitializeComponent();
            GeneralSettings = new PhoneNumberSetupSettings();
            EditedGUID = EditedNumber.GetGUID();

            if (EditedNumber.GetExtension().Length != 0)
            {
                string Prefix = EditedNumber.GetPrefix();
                if (Prefix.Length > 0)
                    Prefix = Prefix + "-";
                this.SubscriberExtension.Text = Prefix + EditedNumber.GetExtension().ToString();
                this.TB_UserName.Text = EditedNumber.GetUserName();
            }
            else
            {
                string Prefix = "";
                if (App.Current != null && App.Current.MainWindow != null)
                    Prefix = (App.Current.MainWindow as MainWindow).GetPrefix();
                if (Prefix.Length > 0)
                    Prefix = Prefix + "-";
                //check if we have data in persport.txt
                possibleNames = Globals.persPortManager.GetServerExtensions();
                foreach(PersPortDataStore i in possibleNames)
                    SubscriberExtension.Items.Add(Prefix + i.Extension);
                if (possibleNames.Count > 0)
                    SubscriberExtension.SelectedIndex = 0;
            }
            this.SubscriberEmail.Text = EditedNumber.GetEmail();
            this.SubscriberNote.Text = EditedNumber.GetNote();

            //focus on first text field
            this.SubscriberExtension.Focus();

            //if we push enter we presume we pushed button "ok"
            RoutedCommand firstSettings = new RoutedCommand();
            firstSettings.InputGestures.Add(new KeyGesture(Key.Enter, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(firstSettings, Button_Click_Ok));
            RoutedCommand SecondSettings = new RoutedCommand();
            SecondSettings.InputGestures.Add(new KeyGesture(Key.Escape, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(SecondSettings, Button_Click_Cancel));

            Globals.MultilangManager.TranslateUIComponent(this);
            this.Owner = App.Current.MainWindow;
        }

        private void Button_Click_Ok(object sender, RoutedEventArgs e)
        {
            PhoneNumber EditedPhoneNumber = Globals.ExtensionManager.PhoneNumberGet(EditedGUID);
            if (EditedPhoneNumber != null)
            {
                try
                {
                    string sExtension = this.SubscriberExtension.Text;
                    if (sExtension.Contains("-"))
                    {
                        string[] FullNumberParts = sExtension.Split('-');
                        string lPrefix = FullNumberParts[0];
                        EditedPhoneNumber.SetPrefix(lPrefix);
                        sExtension = FullNumberParts[1];
                    }
                    EditedPhoneNumber.SetExtension(sExtension);
                }
                catch (Exception) { };
                // editing should be based on a UID to avoid switching tab in the background
                EditedPhoneNumber.SetName(this.TB_UserName.Text);
                EditedPhoneNumber.SetEmail(this.SubscriberEmail.Text);
                EditedPhoneNumber.SetNote(this.SubscriberNote.Text);
                if (FontSetting != null)
                    EditedPhoneNumber.OnFontSettingChanged(FontSetting);
                // queue up a query for tooltip absence status
                if( SubscriberEmail.Text != null && SubscriberEmail.Text.Length > 0)
                    Globals.AbsenceManage.UpdateExtensionTooltipAsync(this.SubscriberEmail.Text);
            }

            //all done. close it
            this.Close();
        }

        private void Button_Click_Cancel(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {

        }

        private void Button_Click_Font(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.FontDialog fd = new System.Windows.Forms.FontDialog();
            System.Windows.Forms.DialogResult dr = fd.ShowDialog();
            if (dr != System.Windows.Forms.DialogResult.Cancel)
            {
                FontSetting = MainWindow.FontDialogToFontSetting(fd);
                Globals.FontManager.InsertNewSetting(FontSetting);
            }
        }

        private void Button_Click_Resolve(object sender, RoutedEventArgs e)
        {
            if (this.SubscriberEmail.Text.Length == 0)
            {
                MessageBox.Show(Globals.MultilangManager.GetTranslation("Email is empty"), "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }
            if (Globals.OutlookService.ResolveEmail(this.SubscriberEmail.Text) == false)
            {
                MessageBox.Show(Globals.MultilangManager.GetTranslation("Could not confirm email"), "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }
            else
            {
                MessageBox.Show(Globals.MultilangManager.GetTranslation("Outlook said email address exists"), "", MessageBoxButton.OK, MessageBoxImage.Information);
            }
        }

        private void SubscriberExtension_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (SubscriberExtension.SelectedIndex < 0 || SubscriberExtension.SelectedIndex > SubscriberExtension.Items.Count || possibleNames == null)
            {
                this.TB_UserName.Text = "";
                return;
            }
            int ind = 0;
            foreach (PersPortDataStore i in possibleNames)
            {
                if(ind == SubscriberExtension.SelectedIndex)
                {
                    this.TB_UserName.Text = i.Name;
                    break;
                }
                ind++;
            }
        }
    }
}
