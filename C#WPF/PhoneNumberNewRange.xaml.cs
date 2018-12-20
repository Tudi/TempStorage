using BLFClient.Backend;
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
    /// Interaction logic for PhoneNumberNewRange.xaml
    /// </summary>
    public partial class PhoneNumberNewRange : Window
    {
        long EditedGUID;
        PhoneNumberSetupSettings GeneralSettings;
        FontSettings FontSetting = null;

        public PhoneNumberNewRange(PhoneNumber EditedNumber)
        {
            InitializeComponent();
            GeneralSettings = new PhoneNumberSetupSettings();
            EditedGUID = EditedNumber.GetGUID();
            //            if (App.Current != null && App.Current.MainWindow != null)
            //                this.SubscriberExtension.Text = (App.Current.MainWindow as MainWindow).GetPrefix();
            if (EditedNumber.GetExtension().Length != 0)
            {
                string Prefix = EditedNumber.GetPrefix();
                if (Prefix.Length > 0)
                    Prefix = Prefix + "-";
                this.SubscriberExtension.Text = Prefix + EditedNumber.GetExtension().ToString();
            }
            else
            {
                System.Collections.ObjectModel.ReadOnlyCollection<PersPortDataStore> possibleNames = Globals.persPortManager.GetServerExtensions();
                HashSet<string> Ranges = new HashSet<string>();
                foreach (PersPortDataStore i in possibleNames)
                    Ranges.Add(i.Extension.Substring(0, i.Extension.Length-1));
                foreach (var i in Ranges)
                    SubscriberExtension.Items.Add(i);
                if (possibleNames.Count > 0)
                    SubscriberExtension.SelectedIndex = 0;
            }

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
                EditedPhoneNumber.SetIsRange(true);
                EditedPhoneNumber.SetExtension(this.SubscriberExtension.Text);
                if(FontSetting!=null)
                    EditedPhoneNumber.OnFontSettingChanged(FontSetting);
            }

            //all done. close it
            this.Close();
        }

        private void Button_Click_Cancel(object sender, RoutedEventArgs e)
        {
            this.Close();
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
    }
}
