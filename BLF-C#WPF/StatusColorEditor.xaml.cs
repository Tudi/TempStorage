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
        NotExisting,
        Idle,
        Ringing,
        OutOfService,
        Busy,
        NumberOfStatusCodes
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
        }

        private void InitUIComponents()
        {
            //init our own components
            ClrPcker_1.SelectedColor = GetStatusColor(PhoneStatusCodes.Idle).Color;
            ClrPcker_2.SelectedColor = GetStatusColor(PhoneStatusCodes.Ringing).Color;
            ClrPcker_3.SelectedColor = GetStatusColor(PhoneStatusCodes.OutOfService).Color;
            ClrPcker_4.SelectedColor = GetStatusColor(PhoneStatusCodes.Busy).Color;
            ClrPcker_5.SelectedColor = GetStatusColor(PhoneStatusCodes.NotExisting).Color;
        }

        /// <summary>
        /// Default colors. This should be overwritten once we load settings from config file
        /// </summary>
        private static void Init()
        {
            Colors = new SolidColorBrush[(int)PhoneStatusCodes.NumberOfStatusCodes];
            Colors[(int)PhoneStatusCodes.NotExisting] = new SolidColorBrush(Color.FromArgb(0xFF, 0xFF, 0xFF, 0xFF)); // white
            Colors[(int)PhoneStatusCodes.Idle] = new SolidColorBrush(Color.FromArgb(0xFF, 0x00, 0xFF, 0x00)); // green
            Colors[(int)PhoneStatusCodes.Ringing] = new SolidColorBrush(Color.FromArgb(0xFF, 0xFF, 0xFF, 0x00)); // yellow
            Colors[(int)PhoneStatusCodes.OutOfService] = new SolidColorBrush(Color.FromArgb(0xFF, 0x77, 0x77, 0x77)); // gray
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
            return Colors[(int)status];
        }

        private void Button_AcceptColorChanges(object sender, RoutedEventArgs e)
        {
            Colors[(int)PhoneStatusCodes.NotExisting] = new SolidColorBrush(ClrPcker_5.SelectedColor.Value); // white
            Colors[(int)PhoneStatusCodes.Idle] = new SolidColorBrush(ClrPcker_1.SelectedColor.Value); // green
            Colors[(int)PhoneStatusCodes.Ringing] = new SolidColorBrush(ClrPcker_2.SelectedColor.Value); // yellow
            Colors[(int)PhoneStatusCodes.OutOfService] = new SolidColorBrush(ClrPcker_3.SelectedColor.Value); // gray
            Colors[(int)PhoneStatusCodes.Busy] = new SolidColorBrush(ClrPcker_4.SelectedColor.Value); // red
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
