using mshtml;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace ReadFortrade1
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            SHDocVw.ShellWindows shellWindows = new SHDocVw.ShellWindows();
            foreach (SHDocVw.WebBrowser ie in shellWindows)
            {
                HTMLDocument doc = ie.Document as mshtml.HTMLDocument;
                if (doc == null)
                    continue;
                string PrevBody = "";
                while (1 == 1)
                {
                    HTMLDocument doc2 = ie.Document as mshtml.HTMLDocument;
                    if (doc2== null)
                        continue;
                    string docBody = doc2.body.outerHTML;
                    if (PrevBody != docBody && PrevBody.Length != 0)
                        PrevBody = docBody;
                    PrevBody = docBody;
//                    Console.WriteLine(docBody);
                }
                
            }        
        }
    }
}
