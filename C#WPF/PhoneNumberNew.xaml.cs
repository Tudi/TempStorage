using BLFClient.Backend;
using System;
using System.Collections.Concurrent;
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
        FontSettings FontSetting = null;
        ReadOnlyCollection<PersPortDataStore> possibleNames = null;
        string SelectedServer = null;
        bool ShowCannonical = false;
        string Prefix = "";
        string ExtensionsFilterString = null;

        public PhoneNumberNew(PhoneNumber EditedNumber)
        {
            InitializeComponent();
            EditedGUID = EditedNumber.GetGUID();

            if (EditedNumber.GetExtension().Length != 0)
            {
                string Prefix = EditedNumber.GetPrefixIfShown(true);
                this.SubscriberExtension.Text = Prefix + EditedNumber.GetExtension().ToString();
                this.TB_UserName.Text = EditedNumber.GetUserName();
                this.SubscriberEmail.Text = EditedNumber.GetEmail();
                this.SubscriberNote.Text = EditedNumber.GetNote();
                this.Servers.Items.Add(EditedNumber.GetServerIPAndPort());
                this.Servers.SelectedIndex = 0;
            }
            else
            {                         
                if (App.Current != null && App.Current.MainWindow != null && (App.Current.MainWindow as MainWindow).ShowCannonical() == true)
                {
                    ShowCannonical = true;
                    Prefix = (App.Current.MainWindow as MainWindow).GetPrefix();
                    if (Prefix.Length > 0)
                        Prefix = Prefix + "-";
                }
                //check if we have data in persport.txt
                possibleNames = Globals.persPortManager.GetServerExtensions();
                GenerateFilteredExtensionDropdown();
            }

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
            this.Left = this.Owner.Left + this.Owner.Width / 2 - this.Width / 2;
            this.Top = this.Owner.Top + this.Owner.Height / 2 - this.Height / 2;
        }

        private void GenerateFilteredExtensionDropdown()
        {
            //nothing to filter or to show
            if (possibleNames == null)
                return;

            //select a server to show
            if (SelectedServer == null)
            {
                Servers.Items.Clear();
                ConcurrentBag<ServerConnectionStatus> Connections = Globals.ConnectionManager.GetConnections();
                foreach (ServerConnectionStatus sc in Connections)
                {
                    if (sc.PendingRemove == true)
                        continue;

                    Servers.Items.Add(sc.GetServerIPAndPort());
                }
                if (Servers.Items.Count != 0)
                {
                    Servers.SelectedIndex = 0;
                    SelectedServer = Servers.Items.GetItemAt(Servers.SelectedIndex).ToString();
                }
            }

            SubscriberExtension.Items.Clear();
            foreach (PersPortDataStore i in possibleNames)
            {
                //ony list extensions from 1 server at a time
                if (SelectedServer != null && i.ServerIPAndPort != SelectedServer)
                    continue;

                string ToAddString = "";
                if (ShowCannonical == false)
                    ToAddString = i.Extension;
                else if (ShowCannonical == true && i.GetPrefix().Length > 0)
                    ToAddString = i.GetPrefix() + "-" + i.Extension;
                else //if (ShowCannonical == true)
                    ToAddString = Prefix + i.Extension;

                if (ExtensionsFilterString != null && ExtensionsFilterString.Length > 0 && ExtensionsFilterString.IndexOf(ToAddString) < 0 )
                    continue;

                SubscriberExtension.Items.Add(ToAddString);
            }
            if (possibleNames.Count > 0)
                SubscriberExtension.SelectedIndex = 0;
            else
                SubscriberExtension.SelectedIndex = -1;
            SubscriberExtension_SelectionChanged(null, null);
        }

        private void Button_Click_Ok(object sender, RoutedEventArgs e)
        {
            PhoneNumber EditedPhoneNumber = Globals.ExtensionManager.PhoneNumberGet(EditedGUID);
            if (EditedPhoneNumber != null)
            {
                // editing should be based on a UID to avoid switching tab in the background
                EditedPhoneNumber.SetName(this.TB_UserName.Text);
                EditedPhoneNumber.SetEmail(this.SubscriberEmail.Text);
                EditedPhoneNumber.SetNote(this.SubscriberNote.Text);
                if(Servers.SelectedIndex>=0)
                    EditedPhoneNumber.SetServerIPAndPort(Servers.Items.GetItemAt(Servers.SelectedIndex).ToString());
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
                    else
                        EditedPhoneNumber.SetPrefix(Globals.persPortManager.GetServerExtensionPrefix(sExtension));
                    EditedPhoneNumber.SetExtension(sExtension);
                }
                catch (Exception) { };
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

        private void Servers_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            SelectedServer = Servers.SelectedItem as string;
            GenerateFilteredExtensionDropdown();
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

        private void SubscriberExtension_TextChanged(object sender, TextChangedEventArgs e)
        {
//            ExtensionsFilterString = SubscriberExtension.Text;
//            GenerateFilteredExtensionDropdown();
        }
    }
}
