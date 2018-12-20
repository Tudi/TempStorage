using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Forms;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace BLFClient.Backend
{
    public class DSWinConnectionManager
    {
        int ExpectedBytes;
        int RecvedBytes;
        char[] BytesReceived;
        const int MaxPhoneNumberLength = 64;
        string PrevMonitoredExtension = "";
        System.Threading.Timer UpdateTimer;
        PhoneStatusCodes PhoneStatus;

        public DSWinConnectionManager()
        {
            System.Windows.Interop.HwndSource.FromHwnd(new System.Windows.Interop.WindowInteropHelper(System.Windows.Application.Current.MainWindow).Handle)?.AddHook(this.WndProc);
            ExpectedBytes = 0;
            RecvedBytes = 0;
            BytesReceived = new char[MaxPhoneNumberLength];
            PhoneStatus = PhoneStatusCodes.PHONE_EXTERNAL; // invalid status
            UpdateTimer = new System.Threading.Timer(PeriodicUpdate, null, 1000, 500);
            Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "Created DSWin message listener");
        }

        ~DSWinConnectionManager()
        {
            UpdateTimer.Dispose();
        }

        private void PeriodicUpdate(object source)
        {
            if (PrevMonitoredExtension == "")
                return;
            if (Monitor.TryEnter(this))
            {
                //get the monitored extension
                PhoneNumber pn = Globals.ExtensionManager.PhoneNumberGetFirst(PrevMonitoredExtension);
                //check if status changed
                if(pn != null && pn.GetStatus() != PhoneStatus)
                {
                    PhoneStatus = pn.GetStatus();
                    //create a window and show the new status
                    System.Windows.Application.Current.Dispatcher.Invoke((Action)delegate
                    {
                        new NotificationTaskBar(PrevMonitoredExtension,PhoneStatus).Show();
                    });
                }
                Monitor.Exit(this);
            }
        }

        public IntPtr WndProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            const int WM_USER = 0x0400;
            if (msg == WM_USER + 98)
            {
                if(ExpectedBytes == RecvedBytes && wParam.ToInt32() == 0)
                {
                    ExpectedBytes = lParam.ToInt32() * 4;
                    RecvedBytes = 0;
                    for (int i = 0; i < MaxPhoneNumberLength; i++)
                        BytesReceived[i] = (char)0;
                }
                else if(RecvedBytes < MaxPhoneNumberLength)
                {
                    int NumberChars = lParam.ToInt32();
                    BytesReceived[RecvedBytes++] = (char)(NumberChars & 0xFF);
                    BytesReceived[RecvedBytes++] = (char)((NumberChars >> 8 ) & 0xFF);
                    BytesReceived[RecvedBytes++] = (char)((NumberChars >> 16) & 0xFF);
                    BytesReceived[RecvedBytes++] = (char)((NumberChars >> 24) & 0xFF);
                }
                if(RecvedBytes == ExpectedBytes)
                {
                    //remove old monitor
                    Globals.ExtensionManager.RemovePhantomExtension(PrevMonitoredExtension);
                    //convert the extension into a full number
                    string FullNumber = "";
                    for (int i = 0; i < MaxPhoneNumberLength; i++)
                    {
                        if (BytesReceived[i] == 0)
                            break;
                        FullNumber = FullNumber + BytesReceived[i];
                    }
                    //create a dummy ( invisible ) phone number to monitor and update status of it
                    Globals.ExtensionManager.CreatePhantomExtension(FullNumber);
                    //remmber what we should not monitor next time
                    PrevMonitoredExtension = FullNumber;
                    PhoneStatus = PhoneStatusCodes.PHONE_EXTERNAL; // invalid status
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagInfo, "DSWin selection changed to : " + PrevMonitoredExtension);
                }
                return IntPtr.Zero;
            }
            return IntPtr.Zero;
        }
    }
}
