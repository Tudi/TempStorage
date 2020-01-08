using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Controls;
using System.Windows.Threading;

namespace ReadFortrade1
{
    public class TimeoutWatchDog : Control
    {
        [DllImport("user32.dll")]
        private static extern IntPtr FindWindow(string lpClassName, string lpWindowName);

        [DllImport("user32.dll")]

        private static extern bool SetForegroundWindow(IntPtr hWnd);
        public void StartPageTimoutWatchdog()
        {
            DateTime AntiContinuesRefresh = DateTime.Now;
            while (Globals.AppIsRunning == true)
            {
                TimeSpan TimeSinceValidContentSeen = DateTime.Now.Subtract(Globals.LastFavoriteSectionParseStamp);
                TimeSpan TimeSinceLastRefresh = DateTime.Now.Subtract(AntiContinuesRefresh);
                if (TimeSinceValidContentSeen.Seconds >= Globals.IETimeoutSeconds 
                    && TimeSinceLastRefresh.Seconds >= Globals.IETimeoutSeconds)
                {
//                        ie.Navigate("https://ready.fortrade.com/#?detach#home");
                    TryRefreshWindow();
                    AntiContinuesRefresh = DateTime.Now;
                    Thread.Sleep(Globals.ThreadCycleSleep);
                }
                else
                    Thread.Sleep(Globals.ThreadCycleSleep);

                string Msg = TimeSinceValidContentSeen.Seconds.ToString() + "/" + TimeSinceLastRefresh.Seconds.ToString();
                this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, (Action)(() =>
                {
                    if (App.Current == null)
                        return;

                    MainWindow MainObject = (MainWindow)App.Current.MainWindow;

                    if (MainObject == null)
                        return;

                    (App.Current.MainWindow as MainWindow).TimeoutTextbox.Text = Msg;
                }));
            }
        }
        public void TryRefreshWindow()
        {
            IntPtr zero = IntPtr.Zero;
            for (int i = 0; (i < 60) && (zero == IntPtr.Zero); i++)
            {
                Thread.Sleep(Globals.ThreadCycleSleep);
                zero = FindWindow("IEFrame", null);
            }
            if (zero != IntPtr.Zero)
            {
                SetForegroundWindow(zero);
                System.Windows.Forms.SendKeys.SendWait("^r");
                System.Windows.Forms.SendKeys.Flush();
            }
        }
    }
}
