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
    /// Interaction logic for NotificationTaskBar.xaml
    /// </summary>
    public partial class NotificationTaskBar : Window
    {
        public NotificationTaskBar(string content, PhoneStatusCodes status)
        {
            InitializeComponent();
            this.ContentBorder.Background = StatusColorEditor.GetStatusBrush(status);
            this.PopupContent.Text = content;
        }

        public new void Show()
        {
            this.Topmost = true;
            base.Show();

            if (System.Windows.Application.Current.MainWindow != this)
            {
                this.Owner = System.Windows.Application.Current.MainWindow;
                this.Owner.Focus();
            }
            this.Closed += this.NotificationWindowClosed;
            var workingArea = System.Windows.Forms.Screen.PrimaryScreen.WorkingArea;

            this.Left = workingArea.Right - this.ActualWidth - 385;
            double top = workingArea.Bottom - this.ActualHeight - 205;

            foreach (Window window in System.Windows.Application.Current.Windows)
            {
                string windowName = window.GetType().Name;

                if (windowName.Equals("NotificationWindow") && window != this)
                {
                    window.Topmost = true;
                    top = window.Top - window.ActualHeight;
                }
            }

            this.Top = top;
        }

        private void ImageMouseUp(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            this.Close();
        }

        private void DoubleAnimationCompleted(object sender, EventArgs e)
        {
            if (!this.IsMouseOver)
            {
                this.Close();
            }
        }

        private void NotificationWindowClosed(object sender, EventArgs e)
        {
            foreach (Window window in System.Windows.Application.Current.Windows)
            {
                string windowName = window.GetType().Name;

                if (windowName.Equals("NotificationWindow") && window != this)
                {
                    // Adjust any windows that were above this one to drop down
                    if (window.Top < this.Top)
                    {
                        window.Top = window.Top + this.ActualHeight;
                    }
                }
            }
        }
    }
}

