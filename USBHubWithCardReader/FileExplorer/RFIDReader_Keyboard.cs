using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Forms;
using System.Timers;

namespace FileExplorer
{
    public class RFIDReader_keyboard : IDisposable
    {
        private readonly List<int> _keys;
        private readonly Stopwatch _stopwatch;
        private readonly System.Timers.Timer _timer;
        private int Timeout = ConfigReader.GetConfigValueInt("RFIDCardReadTimeout",1000); // Timeout of 1 second
        private int ChunkSize = ConfigReader.GetConfigValueInt("RFIDCardReadDigits", 10); // Chunk size of 10 digits

//        public event EventHandler<string> CardPresented;

        public RFIDReader_keyboard()
        {
            _keys = new List<int>();
            _stopwatch = new Stopwatch();
            _timer = new System.Timers.Timer(Timeout);
            _timer.Elapsed += Timer_Tick;
            _timer.AutoReset = true;
            _timer.Enabled = true;
            HookKeyboard();
        }

        private void Timer_Tick(object sender, EventArgs e)
        {
            _stopwatch.Stop();
            _timer.Stop();
            _keys.Clear();
        }

        private void HookKeyboard()
        {
            KeyboardHookManager.KeyDown += KeyboardHookManager_KeyDown;
        }

        private void KeyboardHookManager_KeyDown(object sender, KeyEventArgs e)
        {
            if (char.IsDigit((char)e.KeyValue))
            {
                if (_keys.Count == 0 || _stopwatch.ElapsedMilliseconds > Timeout)
                {
                    LogWriter.WriteLog("RFIDReader_keyboard : starting a new card read session");
                    _stopwatch.Restart();
                    _keys.Clear();
                }

                _keys.Add(e.KeyValue);

                if (_keys.Count == ChunkSize)
                {
                    _timer.Stop();
                    _stopwatch.Stop();
                    string cardDataRead = new string(_keys.ConvertAll(k => (char)k).ToArray());
                    LogWriter.WriteLog("RFIDReader_keyboard : read card data : " + cardDataRead);
                    OnCardPresented(cardDataRead);
                    _keys.Clear();
                }
                else
                {
                    LogWriter.WriteLog("RFIDReader_keyboard : failed to read all required digits : " + new string(_keys.ConvertAll(k => (char)k).ToArray()));
                    _timer.Start();
                }
            }
        }

        protected virtual void OnCardPresented(string cardData)
        {
//            CardPresented?.Invoke(this, cardData);
            Plugable_DeviceManager.OnRFIDCardPresented(cardData);
        }

        public void Dispose()
        {
            KeyboardHookManager.KeyDown -= KeyboardHookManager_KeyDown;
            _timer.Dispose();
            _stopwatch.Stop();
        }
    }

    public static class KeyboardHookManager
    {
        public static event KeyEventHandler KeyDown;

        private static HookProc _proc = HookCallback;
        private static IntPtr _hookID = IntPtr.Zero;

        public static void Start()
        {
            LogWriter.WriteLog("KeyboardHookManager : starting module");
            _hookID = SetHook(_proc);
        }

        public static void Stop()
        {
            LogWriter.WriteLog("KeyboardHookManager : stopping module");
            UnhookWindowsHookEx(_hookID);
        }

        private static IntPtr SetHook(HookProc proc)
        {
            using (Process curProcess = Process.GetCurrentProcess())
            using (ProcessModule curModule = curProcess.MainModule)
            {
                return SetWindowsHookEx(WH_KEYBOARD_LL, proc, GetModuleHandle(curModule.ModuleName), 0);
            }
        }

        private delegate IntPtr HookProc(int nCode, IntPtr wParam, IntPtr lParam);

        private static IntPtr HookCallback(int nCode, IntPtr wParam, IntPtr lParam)
        {
            if (nCode >= 0 && wParam == (IntPtr)WM_KEYDOWN)
            {
                int vkCode = Marshal.ReadInt32(lParam);
                KeyDown?.Invoke(null, new KeyEventArgs((Keys)vkCode));
            }
            return CallNextHookEx(_hookID, nCode, wParam, lParam);
        }

        private const int WH_KEYBOARD_LL = 13;
        private const int WM_KEYDOWN = 0x0100;

        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern IntPtr SetWindowsHookEx(int idHook, HookProc lpfn, IntPtr hMod, uint dwThreadId);

        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool UnhookWindowsHookEx(IntPtr hhk);

        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern IntPtr CallNextHookEx(IntPtr hhk, int nCode, IntPtr wParam, IntPtr lParam);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern IntPtr GetModuleHandle(string lpModuleName);
    }

}
