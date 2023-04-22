using System.Diagnostics;
using System.Runtime.InteropServices;

namespace ShowAvailableMinutes
{
    internal static class Program
    {
        [DllImport("user32.dll")]
        static extern bool ExitWindowsEx(uint uFlags, uint dwReason);

        /// <summary>
        ///  The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            // Attach a handler to the ApplicationClosing event
            AppDomain.CurrentDomain.ProcessExit += new EventHandler(ApplicationClosing);

            // To customize application configuration such as set high DPI settings or default font,
            // see https://aka.ms/applicationconfiguration.
            ApplicationConfiguration.Initialize();
            Application.Run(new Form1());
        }
        static void ApplicationClosing(object sender, EventArgs e)
        {
            // Call the WinAPI function to shut down the PC
//            ExitWindowsEx(0x00000008, 0);
//            Process.Start("shutdown", "/s /t 900");
        }
    }
}