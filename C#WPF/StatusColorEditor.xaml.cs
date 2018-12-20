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
    //list of possible phone statuses
    public enum PhoneStatusCodes
    {
        PHONE_AUSSER_BETRIEB = 0x0000,//Out of service
        PHONE_FREI = 0x0001,//Idle
        PHONE_RUF = 0x0002,//Ringing
        PHONE_AUFSCHALTEN = 0x0003,//Override possible
        PHONE_BESETZT = 0x0004,//Busy
        PHONE_DOESNOT = 0x0005,//Does not exist
        NumberOfStatusCodes,
        PHONE_UMLEITUNG = 0x0100,//Forward
        PHONE_EXTERNAL = 0x1000,//External call
        PHONE_NOT_FWD = 0xF0FF,//Only the state, without forward state
        PHONE_NOT_EXT = 0x0FFF,//Only the state, without external call state
        // english version
        OutOfService = PHONE_AUSSER_BETRIEB,
        Idle = PHONE_FREI,
        Ringing = PHONE_RUF,
        Busy = PHONE_BESETZT,
        NotExisting = 0x7FFF,
        NumberOfStatusCodes2,
}

/// <summary>
/// Interaction logic for StatusColorEditor.xaml
/// </summary>
public partial class StatusColorEditor : Window
    {
        private static SolidColorBrush[] Colors;  //settings that can be saved / loaded

        public StatusColorEditor()
        {
            InitializeComponent();

            //initialize colors
            Init();

            InitUIComponents();

            //if we push enter we presume we pushed button "ok"
            RoutedCommand firstSettings = new RoutedCommand();
            firstSettings.InputGestures.Add(new KeyGesture(Key.Enter, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(firstSettings, Button_AcceptColorChanges));
            RoutedCommand SecondSettings = new RoutedCommand();
            SecondSettings.InputGestures.Add(new KeyGesture(Key.Escape, ModifierKeys.None));
            CommandBindings.Add(new CommandBinding(SecondSettings, Button_DiscardColorChanges));

            Globals.MultilangManager.TranslateUIComponent(this);
            this.Owner = App.Current.MainWindow;
        }

        private void InitUIComponents()
        {
            //init our own components
            ClrPcker_1.SelectedColor = GetStatusColor(PhoneStatusCodes.Idle).Color;
            ClrPcker_2.SelectedColor = GetStatusColor(PhoneStatusCodes.Ringing).Color;
            ClrPcker_3.SelectedColor = GetStatusColor(PhoneStatusCodes.OutOfService).Color;
            ClrPcker_4.SelectedColor = GetStatusColor(PhoneStatusCodes.Busy).Color;
            ClrPcker_5.SelectedColor = GetStatusColor(PhoneStatusCodes.PHONE_DOESNOT).Color;
        }

        private static SolidColorBrush BrushFromRGB(int CompositeRGB)
        {
            byte B = (byte)((CompositeRGB >> 16) & 255);
            byte G = (byte)((CompositeRGB >> 8) & 255);
            byte R = (byte)((CompositeRGB >> 0) & 255);
            return new SolidColorBrush(Color.FromRgb(R, G, B));
        }

        /// <summary>
        /// Default colors. This should be overwritten once we load settings from config file
        /// </summary>
        private static void Init()
        {
            Colors = new SolidColorBrush[(int)PhoneStatusCodes.NumberOfStatusCodes];

            int CompositeRGB = Globals.Config.GetConfigInt("Options", "Color NotExisting", 0);
            if (CompositeRGB > 0)
                Colors[(int)PhoneStatusCodes.PHONE_DOESNOT] = BrushFromRGB(CompositeRGB);
            else
                Colors[(int)PhoneStatusCodes.PHONE_DOESNOT] = new SolidColorBrush(Color.FromArgb(0xFF, 0xFF, 0xFF, 0xFF)); // white

            CompositeRGB = Globals.Config.GetConfigInt("Options", "Color Frei", 0);
            if (CompositeRGB > 0)
                Colors[(int)PhoneStatusCodes.Idle] = BrushFromRGB(CompositeRGB);
            else
                Colors[(int)PhoneStatusCodes.Idle] = new SolidColorBrush(Color.FromArgb(0xFF, 0x00, 0xFF, 0x00)); // green

            CompositeRGB = Globals.Config.GetConfigInt("Options", "Color Ruf", 0);
            if (CompositeRGB > 0)
                Colors[(int)PhoneStatusCodes.Ringing] = BrushFromRGB(CompositeRGB);
            else
                Colors[(int)PhoneStatusCodes.Ringing] = new SolidColorBrush(Color.FromArgb(0xFF, 0xFF, 0xFF, 0x00)); // yellow

            CompositeRGB = Globals.Config.GetConfigInt("Options", "Color Ruf", 0);
            if (CompositeRGB > 0)
                Colors[(int)PhoneStatusCodes.PHONE_AUFSCHALTEN] = BrushFromRGB(CompositeRGB);
            else
                Colors[(int)PhoneStatusCodes.PHONE_AUFSCHALTEN] = new SolidColorBrush(Color.FromArgb(0xFF, 0xFF, 0xFF, 0x00)); // yellow

            CompositeRGB = Globals.Config.GetConfigInt("Options", "Color Außer Betrieb", 0);
            if (CompositeRGB > 0)
                Colors[(int)PhoneStatusCodes.OutOfService] = BrushFromRGB(CompositeRGB);
            else
                Colors[(int)PhoneStatusCodes.OutOfService] = new SolidColorBrush(Color.FromArgb(0xFF, 0x77, 0x77, 0x77)); // gray

            CompositeRGB = Globals.Config.GetConfigInt("Options", "Color Aufschalten möglich", 0);
            if (CompositeRGB > 0)
                Colors[(int)PhoneStatusCodes.Busy] = BrushFromRGB(CompositeRGB);
            else
                Colors[(int)PhoneStatusCodes.Busy] = new SolidColorBrush(Color.FromArgb(0xFF, 0xFF, 0x00, 0x00)); // red
        }

        /// <summary>
        /// Get the color of a specific status
        /// </summary>
        /// <param name="status"></param>
        /// <returns></returns>
        public static SolidColorBrush GetStatusColor(PhoneStatusCodes status)
        {
            if (Colors == null)
                Init();
            if (status == PhoneStatusCodes.PHONE_EXTERNAL)
                status = PhoneStatusCodes.Busy;
            if (status > PhoneStatusCodes.NumberOfStatusCodes)
                status = PhoneStatusCodes.PHONE_DOESNOT;
            return Colors[(int)status];
        }

        private string CompositeRGBFromColor(Color val)
        {
            int Composite = 0;
            Composite += val.R;
            Composite += (val.G << 8);
            Composite += (val.B << 16);
            return Composite.ToString();
        }

        private void Button_AcceptColorChanges(object sender, RoutedEventArgs e)
        {
            Colors[(int)PhoneStatusCodes.PHONE_DOESNOT] = new SolidColorBrush(ClrPcker_5.SelectedColor.Value); // white
            Colors[(int)PhoneStatusCodes.Idle] = new SolidColorBrush(ClrPcker_1.SelectedColor.Value); // green
            Colors[(int)PhoneStatusCodes.Ringing] = new SolidColorBrush(ClrPcker_2.SelectedColor.Value); // yellow
            Colors[(int)PhoneStatusCodes.OutOfService] = new SolidColorBrush(ClrPcker_3.SelectedColor.Value); // gray
            Colors[(int)PhoneStatusCodes.Busy] = new SolidColorBrush(ClrPcker_4.SelectedColor.Value); // red

            //also save to config
            Globals.Config.SetConfig("Options", "Color NotExisting", CompositeRGBFromColor(ClrPcker_5.SelectedColor.Value));
            Globals.Config.SetConfig("Options", "Color Frei", CompositeRGBFromColor(ClrPcker_1.SelectedColor.Value));
            Globals.Config.SetConfig("Options", "Color Ruf", CompositeRGBFromColor(ClrPcker_2.SelectedColor.Value));
            Globals.Config.SetConfig("Options", "Color Außer Betrieb", CompositeRGBFromColor(ClrPcker_3.SelectedColor.Value));
            Globals.Config.SetConfig("Options", "Color Aufschalten möglich", CompositeRGBFromColor(ClrPcker_4.SelectedColor.Value));

            this.Close();

            //notify everyone that colors need to be updated
            if (App.Current != null && App.Current.MainWindow != null)
                (App.Current.MainWindow as MainWindow).OnStatusColorChanged();
        }

        private void Button_DiscardColorChanges(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void Button_RestoreDefaultColors(object sender, RoutedEventArgs e)
        {
            Init();
            InitUIComponents();
        }
    }
}
