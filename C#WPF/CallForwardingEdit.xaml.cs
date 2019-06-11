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
    /// Interaction logic for CallForwardingEdit.xaml
    /// </summary>
    public partial class CallForwardingEdit : Window
    {
        public CallForwardingEdit(string SelectedExtension)
        {
            InitializeComponent();

            if (SelectedExtension.Length != 0)
            {
                Extensions.Items.Add(SelectedExtension);
                Extensions.SelectedIndex = 0;
                Extensions.IsEnabled = false;
            }
            else
            {
                //get all the possible extensions
                HashSet<string> Ext = Globals.ExtensionManager.GetExtensions();
                foreach (string pn in Ext)
                    Extensions.Items.Add(pn);
                if(Ext.Count!=0)
                    Extensions.SelectedIndex = 0;
            }
            ForwardTypes.Items.Add("ImmediateOn"); // the only possible value ?
            ForwardTypes.Items.Add("ImmediateOff"); // the only possible value ?

            OnSelectExtension(); //populate other fields based on the selected extension

            rb_Destination.IsChecked = true;

            //if we push enter we presume we pushed button "ok"
            RoutedCommand firstSettings = new RoutedCommand();
            firstSettings.InputGestures.Add(new KeyGesture(Key.Enter, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(firstSettings, Button_Click_OK));
            RoutedCommand SecondSettings = new RoutedCommand();
            SecondSettings.InputGestures.Add(new KeyGesture(Key.Escape, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(SecondSettings, Button_Click_Cancel));

            Globals.MultilangManager.TranslateUIComponent(this);
            this.Owner = App.Current.MainWindow;
            this.Left = this.Owner.Left + this.Owner.Width / 2 - this.Width / 2;
            this.Top = this.Owner.Top + this.Owner.Height / 2 - this.Height / 2;
        }

        private void OnSelectExtension()
        {
            string SelectedExtension = (string)Extensions.SelectedValue;

            //get this extension and check it's details
            ForwardStatusStore pn = Globals.ForwardManager.ForwardStatusGet(SelectedExtension);
            if (pn != null)
            {
                if (pn.ForwardType != CallForwardingTypes.CallForwardNone)
                {
                    ForwardTypes.SelectedIndex = 0;
                    gb_ForwardTo.IsEnabled = true;
                }
                else
                {
                    ForwardTypes.SelectedIndex = 1;
                    gb_ForwardTo.IsEnabled = false;
                }

                if (pn.ForwardType == CallForwardingTypes.CallForwardDestination)
                {
                    rb_Destination.IsChecked = true;
                    rb_VoiceMail.IsChecked = false;
                }
                else
                {
                    rb_Destination.IsChecked = false;
                    rb_VoiceMail.IsChecked = true;
                }

                if (pn.DestinationForward != 0)
                    DestinationExtension.Text = pn.DestinationForward.ToString();
                else
                    DestinationExtension.Text = "";

                if (pn.VoiceMailForward != 0)
                    Voicemail.Text = pn.VoiceMailForward.ToString();
                else
                    Voicemail.Text = "";
            }
            else
            {
                ForwardTypes.SelectedIndex = 0;
                DestinationExtension.Text = "";
                Voicemail.Text = "";
                rb_Destination.IsChecked = false;
                rb_VoiceMail.IsChecked = false;
                gb_ForwardTo.IsEnabled = false;
                ForwardTypes.SelectedIndex = 0;
            }
        }

        private void Button_Click_OK(object sender, RoutedEventArgs e)
        {
            //get the selected extension
            string SelectedExtension = (string)Extensions.SelectedValue;
            CallForwardingTypes ft = CallForwardingTypes.CallForwardNone;
            if(ForwardTypes.SelectedIndex == 1)
                ft = CallForwardingTypes.CallForwardNone;
            else if (rb_VoiceMail.IsChecked == true)
                ft = CallForwardingTypes.CallForwardVoiceMail;
            else if (rb_Destination.IsChecked == true)
                ft = CallForwardingTypes.CallForwardDestination;
            Globals.ForwardManager.CallForwardingSet(SelectedExtension, ft, PhoneNumberManager.Int32Parse(Voicemail.Text, 0), PhoneNumberManager.Int32Parse(DestinationExtension.Text, 0));
            this.Close();
        }

        private void Button_Click_Cancel(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void Extensions_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            OnSelectExtension();
        }

        private void FwdType_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (ForwardTypes.SelectedIndex == 1)
                gb_ForwardTo.IsEnabled = false;
            else
                gb_ForwardTo.IsEnabled = true;
        }
    }
}
