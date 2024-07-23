using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Runtime.InteropServices;
using WinForms = System.Windows.Forms;
using System.Diagnostics;

namespace CradleWindowsAgent
{
    class KeyboardEvents
    {
        internal class InterceptKeys
        {
            #region Delegates

            public delegate IntPtr LowLevelKeyboardProc(int nCode, IntPtr wParam, IntPtr lParam);

            #endregion

            private const int WH_KEYBOARD_LL = 13;
            private const int WM_KEYDOWN = 0x0100;

            public static IntPtr SetHook(LowLevelKeyboardProc proc)
            {
                using (Process curProcess = Process.GetCurrentProcess())
                using (ProcessModule curModule = curProcess.MainModule)
                {
                    return SetWindowsHookEx(WH_KEYBOARD_LL, proc, GetModuleHandle(curModule.ModuleName), 0);
                }
            }

            [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
            private static extern IntPtr SetWindowsHookEx(int idHook,
                                                          LowLevelKeyboardProc lpfn, IntPtr hMod, uint dwThreadId);

            [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
            [return: MarshalAs(UnmanagedType.Bool)]
            public static extern bool UnhookWindowsHookEx(IntPtr hhk);

            [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
            public static extern IntPtr CallNextHookEx(IntPtr hhk, int nCode,
                                                       IntPtr wParam, IntPtr lParam);

            [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
            private static extern IntPtr GetModuleHandle(string lpModuleName);
        }

        private static readonly InterceptKeys.LowLevelKeyboardProc _proc = HookCallback;
        private static IntPtr _hookID = IntPtr.Zero;
        private const uint WM_KEYUP = 0x0101;
        /// <summary>
        /// Detach the keyboard hook; call during shutdown to prevent calls as we unload
        /// </summary>
        public static void DetachKeyboardHook()
        {
            if (_hookID != IntPtr.Zero)
                InterceptKeys.UnhookWindowsHookEx(_hookID);
        }

        public static void AddKeyboardHook()
        {
            try
            {
                _hookID = InterceptKeys.SetHook(_proc);
            }
            catch
            {
                DetachKeyboardHook();
            }
        }
        public static IntPtr HookCallback(int nCode, IntPtr wParam, IntPtr lParam)
        {
            if (nCode >= 0)
            {
                bool alt = (WinForms.Control.ModifierKeys & WinForms.Keys.Alt) != 0;
                bool control = (WinForms.Control.ModifierKeys & WinForms.Keys.Control) != 0;
                bool shiftL = (WinForms.Control.ModifierKeys & WinForms.Keys.LShiftKey) != 0;
                bool shiftR = (WinForms.Control.ModifierKeys & WinForms.Keys.RShiftKey) != 0;
                bool shift = (WinForms.Control.ModifierKeys & WinForms.Keys.Shift) != 0;

                int vkCode = Marshal.ReadInt32(lParam);
                WinForms.Keys key = (WinForms.Keys)vkCode;

                if (control && key == WinForms.Keys.V && wParam == (IntPtr)WM_KEYUP)
                {
                    Console.WriteLine("control + V pressed");
                    ((App)(Application.Current)).wndMain.AddToList("Clipboard content pasted: ");

                    return (IntPtr)1; // Handled.
                }
                if (shiftL && key == WinForms.Keys.Insert && wParam == (IntPtr)WM_KEYUP)
                {
                    Console.WriteLine("shiftL + Insert pressed");
                    ((App)(Application.Current)).wndMain.AddToList("Clipboard content pasted: ");

                    return (IntPtr)1; // Handled.
                }
                if (shiftR && key == WinForms.Keys.Insert && wParam == (IntPtr)WM_KEYUP)
                {
                    Console.WriteLine("shiftR + Insert pressed");
                    ((App)(Application.Current)).wndMain.AddToList("Clipboard content pasted: ");

                    return (IntPtr)1; // Handled.
                }
                if (shift && key == WinForms.Keys.Insert && wParam == (IntPtr)WM_KEYUP)
                {
                    Console.WriteLine("shift + Insert pressed");
                    ((App)(Application.Current)).wndMain.AddToList("Clipboard content pasted: ");

                    return (IntPtr)1; // Handled.
                }

            }

            return InterceptKeys.CallNextHookEx(_hookID, nCode, wParam, lParam);
        }

        
    }
}
