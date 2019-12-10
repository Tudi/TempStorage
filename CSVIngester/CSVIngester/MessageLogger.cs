using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;

namespace CSVIngester
{
    public class MessageLogger : Control
    {
        static bool LogToPopup = false;
        static bool LogToConsole = true;
//        static bool LogToFile = false;
        public void Log(string Msg)
        {
            if(LogToPopup)
                MessageBox.Show(Msg);
            if(LogToConsole)
            {
                string TimeMsg = "[" + DateTime.Now.Hour + ":" + DateTime.Now.Minute + "]";
                Msg = TimeMsg + Msg + "\n";

                this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, (Action)(() =>
                {
                    if (App.Current == null)
                        return;

                    MainWindow MainObject = (MainWindow)App.Current.MainWindow;

                    if (MainObject == null)
                        return;

                    (App.Current.MainWindow as MainWindow).ConsoleTextbox.Text += Msg;
                    (App.Current.MainWindow as MainWindow).ConsoleTextbox.ScrollToEnd();
                }));
            }
        }
    }
}
