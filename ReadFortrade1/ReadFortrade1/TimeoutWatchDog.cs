using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using mshtml;

namespace ReadFortrade1
{
    public class TimeoutWatchDog
    {
        [DllImport("user32.dll")]
        private static extern IntPtr FindWindow(string lpClassName, string lpWindowName);

        [DllImport("user32.dll")]

        private static extern bool SetForegroundWindow(IntPtr hWnd);
        public void StartPageTimoutWatchdog()
        {
            DateTime LastDifferentStamp = DateTime.Now;
            SHDocVw.ShellWindows shellWindows = new SHDocVw.ShellWindows();
            foreach (SHDocVw.WebBrowser ie in shellWindows)
            {
                HTMLDocument doc = ie.Document as mshtml.HTMLDocument;
                if (doc == null)
                    continue;
                string PrevBody = "";
                while (1 == 1)
                {
                    HTMLDocument doc2 = null;
                    string docBody = null;
                    try
                    {
                        doc2 = ie.Document as mshtml.HTMLDocument;
                        if (doc2 == null || doc2.body == null || doc2.body.outerHTML == null)
                            continue;
                        docBody = doc2.body.outerHTML;
                        if (docBody == null)
                            continue;
                    }
                    catch
                    {
                        Thread.Sleep(500);
                        continue;
                    }
                    if (PrevBody != docBody && PrevBody.Length == 0)
                    {
                        PrevBody = docBody;
                        LastDifferentStamp = DateTime.Now;
                        PageParser.ParseBody(docBody);
                    }
                    if (DateTime.Now.Subtract(LastDifferentStamp).Seconds >= 20)
                    {
                        LastDifferentStamp = DateTime.Now;
                        TryRefreshWindow();
                    }
                }

            }
        }
        public void TryRefreshWindow()
        {
            IntPtr zero = IntPtr.Zero;
            for (int i = 0; (i < 60) && (zero == IntPtr.Zero); i++)
            {
                Thread.Sleep(500);
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
