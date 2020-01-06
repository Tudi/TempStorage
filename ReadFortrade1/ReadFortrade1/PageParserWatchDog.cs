using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using mshtml;
using System.Threading;

namespace ReadFortrade1
{
    public class PageParserWatchDog
    {
        private SHDocVw.WebBrowser FetchCurrentIE()
        {
            SHDocVw.ShellWindows shellWindows = new SHDocVw.ShellWindows();
            foreach (SHDocVw.WebBrowser ie in shellWindows)
            {
                HTMLDocument doc = ie.Document as mshtml.HTMLDocument;
                if (doc == null)
                    continue;
                return ie;
            }
            return null;
        }
        public void StartPageParserWatchdog()
        {
            SHDocVw.WebBrowser ie = FetchCurrentIE();
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
                    Thread.Sleep(Globals.ThreadCycleSleep);
                    ie = FetchCurrentIE();
                    continue;
                }
                if (PrevBody.Equals(docBody) == false || PrevBody.Length == 0)
                {
                    PrevBody = docBody;
                    PageParser.ParseBody(docBody);
                }
                Thread.Sleep(Globals.ThreadCycleSleep);
            }
        }
    }
}
