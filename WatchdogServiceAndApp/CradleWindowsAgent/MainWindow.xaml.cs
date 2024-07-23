using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Windows.Interop;
using System.Drawing;
using CradleWindowsAgent.Tools;
using CradleWindowsAgent.PushNotifications;
using AppHeartbeatManager;

namespace CradleWindowsAgent
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public bool bTracking = false;

        /// <summary>
        /// Places the given window in the system-maintained clipboard format listener list.
        /// </summary>
        [DllImport("user32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool AddClipboardFormatListener(IntPtr hwnd);

        /// <summary>
        /// Removes the given window from the system-maintained clipboard format listener list.
        /// </summary>
        [DllImport("user32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool RemoveClipboardFormatListener(IntPtr hwnd);

        /// <summary>
        /// Sent when the contents of the clipboard have changed.
        /// </summary>
        private const int WM_CLIPBOARDUPDATE = 0x031D;

        // Import the "GetClipboardOwner" function from the User32 library
        [DllImport("user32.dll", SetLastError = true)]
        public static extern IntPtr GetClipboardOwner();

        [DllImport("user32.dll", SetLastError = true)]
        public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint lpdwProcessId);

        private MonitorFileDragDrop _fileDragHook;
        private MonitorOpenPanel _openPanelHook;
        private AppHeartbeatVerifier _heartbeatVerifier;

        /// <summary>
        /// </summary>
        public MainWindow()
        {
            InitializeComponent();

            _heartbeatVerifier = new AppHeartbeatVerifier(VerifiedApplicationTypes.CradleWindowsAgent);
            _heartbeatVerifier.StartBackground();

            try
            {
                PusherHandler pusher = new PusherHandler();
            }
            catch (Exception ex)
            {
                LogWriter.WriteLog("PusherHandler() Initialization EX =" + ex.Message);
            }
        }
        
        protected override void OnSourceInitialized(EventArgs e)
        {
            base.OnSourceInitialized(e);

            if (!(PresentationSource.FromVisual(this) is HwndSource source))
                return;

            source.AddHook(this.WndProc);
        }

        private IntPtr WndProc(IntPtr hWnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            //base.WndProc(ref msg);
            if (msg == WM_CLIPBOARDUPDATE)
            {
                try
                {

                    IntPtr owner = GetClipboardOwner();
                    var threadId = GetWindowThreadProcessId(owner, out uint processId);
                    string processName = string.Empty;
                    if (processId != 0)
                    {
                        using (var proc = Process.GetProcessById((int)processId))
                        {
                            processName = proc?.ProcessName;
                        }
                    }

                    IDataObject iData = Clipboard.GetDataObject();      // Clipboard's data.

                    /* Depending on the clipboard's current data format we can process the data differently. 
                     * Feel free to add more checks if you want to process more formats. */
                    if (iData.GetDataPresent(DataFormats.Text))
                    {
                        string text = (string)iData.GetData(DataFormats.Text);
                        // do something with it
                        AddToList("Text Copied: " + text + "; Process: " + processName);
                    }
                    else if (iData.GetDataPresent(DataFormats.Bitmap))
                    {
                        // Bitmap image = (Bitmap)iData.GetData(DataFormats.Bitmap);
                        // do something with it
                        AddToList("Screenshot Taken or portion image copied" + "; Process: " + processName);
                    }
                    else if (iData.GetDataPresent(DataFormats.FileDrop))
                    {
                        // Bitmap image = (Bitmap)iData.GetData(DataFormats.Bitmap);
                        // do something with it
                        string[] paths = (string[])iData.GetData(DataFormats.FileDrop);
                        string sPaths = "File(s) copied to clipboard: ";
                        foreach (string file in paths)
                        {
                            sPaths += file;
                            sPaths += ", ";
                        }
                        AddToList(sPaths + "; Process: " + processName);
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }
            }
            return IntPtr.Zero;
        }
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
           
        }
        private void BtnStartTracking_Click(object sender, RoutedEventArgs e)
        {
            if (bTracking)
            {
                BtnStartTracking.Content = "Start Tracking";
                StopWatchers();
            }
            else
            {
                //Start tracking
                BtnStartTracking.Content = "Stop Tracking";
                FileWatcher.StartWatcher("C:\\");

                //Start clipboard watcher
                Window window = Window.GetWindow(this);
                var wih = new WindowInteropHelper(window);
                IntPtr hWnd = wih.Handle;
                AddClipboardFormatListener(hWnd);

                //Start keyboard hook for paste events
                KeyboardEvents.AddKeyboardHook();


                // Initialize the file drag hook
                _fileDragHook = new MonitorFileDragDrop();

                // Subscribe to the FileDragAction event
                _fileDragHook.StartTracking();

                _openPanelHook = new MonitorOpenPanel();
                _openPanelHook.StartTracking();


                bTracking = true;
            }
        }
        public void AddToList(string sItem)
        {
            this.Dispatcher.Invoke(() =>
            {
                try
                {
                    ListEvents.Items.Add(sItem);

                    //Let's write to log file too
                    LogWriter.WriteLog(sItem);
                }
                catch (Exception ex)
                {
                    LogWriter.WriteLog("AddToList ex - " + ex.Message);
                }
            });
        }

        private void BtnClearLogs_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                ListEvents.Items.Clear();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        void StopWatchers()
        {
            LogWriter.WriteLog("StopWatchers()");
            if (bTracking)
            {
                FileWatcher.StopTracking();
                bTracking = false;

                //Stop Clipboard watcher
                Window window = Window.GetWindow(this);
                var wih = new WindowInteropHelper(window);
                IntPtr hWnd = wih.Handle;
                RemoveClipboardFormatListener(hWnd);

                //unhokk keyboard
                KeyboardEvents.DetachKeyboardHook();
            }
        }
        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            StopWatchers();
            _heartbeatVerifier.StopBackground();

            Application.Current.Shutdown(0);
        }
    }

    

}
