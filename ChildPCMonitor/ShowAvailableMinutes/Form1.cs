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

        private readonly RegistryKey _registryKey1;
        private const string RegistryPath1 = "SOFTWARE\\WOW6432Node\\MinCounter";
        private const string RegistryValueName1 = "UIHeartBeat";
        public Form1()
        {
            InitializeComponent();

            _registryKey1 = Registry.LocalMachine.OpenSubKey(RegistryPath1, true) ?? Registry.LocalMachine.CreateSubKey(RegistryPath1);

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
            _registryKey1.Close();
        }

        private void UpdateUsedTime()
        {
            const string RegistryPath1 = "SOFTWARE\\WOW6432Node\\MinCounter";
            const string RegistryValueName1 = "SumOf5";
            using (RegistryKey _registryKey1 = Registry.LocalMachine.OpenSubKey(RegistryPath1, false))
            {
                if (_registryKey1 != null)
                {
                    var regVal = _registryKey1.GetValue(RegistryValueName1, 0);
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
            _registryKey1.SetValue(RegistryValueName1, DateTimeOffset.UtcNow.ToUnixTimeSeconds(), RegistryValueKind.DWord);

            // get available minutes and refresh UI
            UpdateUsedTime();
            lMinutesHave.Text =  myDoc.GetMinutesAvailable().ToString();

            // check if we no longer have enough minutes, close the PC
            double haveMinutes = double.Parse(lMinutesHave.Text);
            double playedMinutes = double.Parse(lPlayed.Text);
            if(haveMinutes <= playedMinutes)
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
                lPlayed.BackColor = Color.Green;
            }
        }
    }
}