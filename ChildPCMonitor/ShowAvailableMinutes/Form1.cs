using Microsoft.Win32;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace ShowAvailableMinutes
{
    public partial class Form1 : Form
    {
        [DllImport("user32.dll")]
        static extern bool ExitWindowsEx(uint uFlags, uint dwReason);

        private System.Timers.Timer _timer;
        private GetGoogleData myDoc;
        private int _redFlags;

        private readonly RegistryKey _heartBeatRegKey;
        private const string RegistryPath1 = "SOFTWARE\\WOW6432Node\\MinCounter";
        private const string RegistryValueName1 = "UIHeartBeat";
        public Form1()
        {
            InitializeComponent();

            _heartBeatRegKey = Registry.LocalMachine.OpenSubKey(RegistryPath1, true) ?? Registry.LocalMachine.CreateSubKey(RegistryPath1);

            _redFlags = 0;

            myDoc = new GetGoogleData();

            Timer_Elapsed(null, null);

            _timer = new System.Timers.Timer();
            _timer.Elapsed += Timer_Elapsed;
            _timer.Interval = TimeSpan.FromMinutes(5).TotalMilliseconds;
            _timer.Start();
        }

        ~Form1()
        {
            _timer.Stop();
            _timer.Dispose();
            _heartBeatRegKey.Close();
        }

        private void UpdateUsedTime()
        {
            const string RegistryPath1 = "SOFTWARE\\WOW6432Node\\MinCounter";
            const string RegistryValueName1 = "SumOf5";
            using (RegistryKey _minutesPlayedRegKey = Registry.LocalMachine.OpenSubKey(RegistryPath1, false))
            {
                if (_minutesPlayedRegKey != null)
                {
                    var regVal = _minutesPlayedRegKey.GetValue(RegistryValueName1, 0);
                    if (regVal != null)
                    {
                        long modMinutes = (DateTimeOffset.UtcNow.ToUnixTimeSeconds() / 60) % 5;
                        lPlayed.Text = ((int)regVal + modMinutes).ToString();
                    }
                }
            }
        }

        private void Timer_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            // get available minutes and refresh UI
            UpdateUsedTime();
            lMinutesHave.Text =  myDoc.GetMinutesAvailable().ToString();

            // check if we no longer have enough minutes, close the PC
            double haveMinutes = double.Parse(lMinutesHave.Text);
            double playedMinutes = double.Parse(lPlayed.Text);
            double availableMinutes = haveMinutes - playedMinutes;
            lMinutesRemain.Text = availableMinutes.ToString();

            if(availableMinutes <= 0)
            {
                lPlayed.BackColor = Color.Red;
                _redFlags++;

                if (_redFlags > 1)
                {
                    Process.Start("shutdown", "/f /s /t 900");
                }
                else if (_redFlags > 2)
                {
//                    ExitWindowsEx(0x00000008, 0);
                }
            }
            else
            {
                _heartBeatRegKey.SetValue(RegistryValueName1, DateTimeOffset.UtcNow.ToUnixTimeSeconds(), RegistryValueKind.DWord);
                lPlayed.BackColor = Color.Green;
            }
        }
    }
}