using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace CradleWindowsAgent
{
    public class FileDragEventArgs : EventArgs
    {
        public string SourceFilePath { get; set; }
        public string DestinationApp { get; set; }
    }

    public class MonitorFileDragDrop
    {
        private const int WH_MOUSE_LL = 14;
        private const int WM_LBUTTONDOWN = 0x0201;
        private const int WM_LBUTTONUP = 0x0202;
        private const int WM_MOUSEMOVE = 0x0200;
        private const int WM_DROPFILES = 0x0233;
        private const int WM_COPYDATA = 0x004A;

        private delegate IntPtr LowLevelMouseProc(int nCode, IntPtr wParam, IntPtr lParam);

        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern IntPtr SetWindowsHookEx(int idHook, LowLevelMouseProc lpfn, IntPtr hMod, uint dwThreadId);

        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool UnhookWindowsHookEx(IntPtr hhk);

        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern IntPtr CallNextHookEx(IntPtr hhk, int nCode, IntPtr wParam, IntPtr lParam);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern IntPtr GetModuleHandle(string lpModuleName);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern IntPtr WindowFromPoint(POINT point);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint processId);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr OpenProcess(ProcessAccessFlags processAccess, bool bInheritHandle, uint processId);

        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern int GetWindowText(IntPtr hWnd, System.Text.StringBuilder lpString, int nMaxCount);

        [DllImport("user32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool EnumWindows(EnumWindowsProc lpEnumFunc, IntPtr lParam);

        private delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool ChangeClipboardChain(IntPtr hWndRemove, IntPtr hWndNewNext);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool IsClipboardFormatAvailable(uint format);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern IntPtr GetClipboardData(uint uFormat);

        [DllImport("shell32.dll")]
        private static extern uint DragQueryFile(IntPtr hDrop, uint iFile, StringBuilder lpszFile, uint cch);



        private static LowLevelMouseProc _proc = HookCallback;
        private static IntPtr _hookID = IntPtr.Zero;
        private static bool _isDragging = false;
        private static bool _isMouseDown = false;
        private static POINT _lastMouseDownPos;
        private const int DragThreshold = 5; // Set a threshold distance for drag start detection (in pixels)

        public void StartTracking()
        {
            _hookID = SetHook(_proc);
        }
        ~MonitorFileDragDrop()
        {
            UnhookWindowsHookEx(_hookID);
        }
        private static IntPtr SetHook(LowLevelMouseProc proc)
        {
            using (var curProcess = System.Diagnostics.Process.GetCurrentProcess())
            using (var curModule = curProcess.MainModule)
            {
                return SetWindowsHookEx(WH_MOUSE_LL, proc, GetModuleHandle(curModule.ModuleName), 0);
            }
        }

        private static IntPtr HookCallback(int nCode, IntPtr wParam, IntPtr lParam)
        {
            if (nCode >= 0 && (wParam == (IntPtr)WM_LBUTTONDOWN || wParam == (IntPtr)WM_LBUTTONUP || wParam == (IntPtr)WM_MOUSEMOVE))
            {
                MSLLHOOKSTRUCT hookStruct = (MSLLHOOKSTRUCT)Marshal.PtrToStructure(lParam, typeof(MSLLHOOKSTRUCT));

                if (wParam == (IntPtr)WM_LBUTTONDOWN)
                {
                    _isMouseDown = true;
                    _lastMouseDownPos = hookStruct.pt;
                }
                else if (wParam == (IntPtr)WM_LBUTTONUP)
                {
                    if(_isMouseDown && _isDragging)
                    {
                        Console.WriteLine("Drag Stopped");

                        int mouseX = hookStruct.pt.X;
                        int mouseY = hookStruct.pt.Y;
                        // Get the window handle (HWND) at the mouse position
                        IntPtr windowHandle = WindowFromPoint(new POINT { X = mouseX, Y = mouseY });

                        // Get the process ID and main window handle of the window
                        uint processId;

                        GetWindowThreadProcessId(windowHandle, out processId);
                        IntPtr processHandle = OpenProcess(ProcessAccessFlags.QueryInformation | ProcessAccessFlags.VirtualMemoryRead, false, processId);

                        using (Process process = Process.GetProcessById((int)processId))
                        {
                            Console.WriteLine("Process name where dropped: =" + process.ProcessName);
                        }
                            
                    }
                    _isMouseDown = false;
                    _isDragging = false;
                    
                }
                else if (wParam == (IntPtr)WM_MOUSEMOVE)
                {
                    if (_isMouseDown && !_isDragging)
                    {
                        // Calculate the distance between the current mouse position and the last mouse down position
                        int deltaX = Math.Abs(hookStruct.pt.X - _lastMouseDownPos.X);
                        int deltaY = Math.Abs(hookStruct.pt.Y - _lastMouseDownPos.Y);
                        if (deltaX >= DragThreshold || deltaY >= DragThreshold)
                        {
                            _isDragging = true;
                            Console.WriteLine("Drag Started");
                        }
                    }
                }
            }
            else if (nCode >= 0 && (wParam == (IntPtr)WM_DROPFILES || wParam == (IntPtr)WM_COPYDATA))
            {
                // If a file drop event is detected, consider it a file drag operation
                Console.WriteLine("DROPPED");
            }

            return CallNextHookEx(_hookID, nCode, wParam, lParam);
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct POINT
        {
            public int X;
            public int Y;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct MSLLHOOKSTRUCT
        {
            public POINT pt;
            public uint mouseData;
            public uint flags;
            public uint time;
            public IntPtr dwExtraInfo;
        }

        [Flags]
        private enum ProcessAccessFlags : uint
        {
            QueryInformation = 0x0400,
            VirtualMemoryRead = 0x0010,
        }
    }
}
