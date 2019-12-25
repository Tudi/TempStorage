using mshtml;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
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
    public class StockDataHistory
    {
        public StockDataHistory()
        {
            ValuesAdded = 0;
            SellPriceSum = 0;
            BuyPriceSum = 0;
            RecordedFirst = DateTime.Now;
            SellPriceMin = 999999999;
            SellPriceMax = -1;
            BuyPriceMin = 999999999;
            BuyPriceMax = -1;
        }
        public void Update()
        {
            if (ValuesAdded == 0)
                return;
            SellPrice = SellPriceSum / ValuesAdded;
            BuyPrice = BuyPriceSum / ValuesAdded;
        }
        public void AddValue(double pSellPrice, double pBuyPrice, double pSellSentiment, double pBuySentiment)
        {
            ValuesAdded++;
            SellPriceSum += pSellPrice;
            BuyPriceSum += pBuyPrice;
            if (pSellPrice < SellPriceMin)
                SellPriceMin = pSellPrice;
            if (pSellPrice > SellPriceMax)
                SellPriceMax = pSellPrice;
            if (pBuyPrice < BuyPriceMin)
                BuyPriceMin = pBuyPrice;
            if (pBuyPrice > BuyPriceMax)
                BuyPriceMax = pBuyPrice;
            SellSentiment = pSellSentiment;
            BuySentiment = pBuySentiment;
        }
        public string Name;
        public double SellPrice;
        public double BuyPrice;
        public double SellSentiment;
        public double BuySentiment;
        double SellPriceSum;
        double BuyPriceSum;
        int ValuesAdded;
        double SellPriceMin;
        double SellPriceMax;
        double BuyPriceMin;
        double BuyPriceMax;
        public DateTime RecordedFirst;
        public DateTime RecordedLast;
    }

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        [DllImport("user32.dll")]
        private static extern IntPtr FindWindow(string lpClassName, string lpWindowName);

        [DllImport("user32.dll")]
        private static extern bool SetForegroundWindow(IntPtr hWnd);

        List<StockDataHistory> DataHistory = new List<StockDataHistory>();
        public MainWindow()
        {
            InitializeComponent();

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
                        ParseBody(docBody);
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
        public void ParseBody(string docBody)
        {
            //search for the favorites section
            //<div class="instrumentsTable" id="instrumentsTable" style="height: 256px; z-index: 2;">
            int StartOfDiv = docBody.IndexOf("<div class=\"instrumentsTable\" id=\"instrumentsTable\"");
            int OpenDivCount = 1;
            int ClosedDivCount = 0;
            string FavoritesSection = "";
            for (int i = StartOfDiv + 1; i < docBody.Length-4; i++)
            {
                if (docBody[i] == '<' && docBody[i + 1] == 'd' && docBody[i + 2] == 'i' && docBody[i + 3] == 'v')
                    OpenDivCount++;
                if (docBody[i] == '<' && docBody[i + 1] == '/' && docBody[i + 2] == 'd' && docBody[i + 3] == 'i' && docBody[i + 4] == 'v')
                    ClosedDivCount++;
                if (OpenDivCount == ClosedDivCount)
                    FavoritesSection = docBody.Substring(StartOfDiv, i - StartOfDiv);
            }
            //find the closing </div>
            if (FavoritesSection != "")
                ParseFavoritesSection(FavoritesSection);
        }
        public void ParseFavoritesSection(string FavoritesSection)
        {
            string ToSearch = "CFD on the daily spot price of";
            string []Sections = FavoritesSection.Split(new[] { ToSearch }, StringSplitOptions.None);
            for(int i=1;i<Sections.Length;i++)
            {
                //get name
                string Name;
                int NameStart = 0;
                for (; NameStart < Sections[i].Length; NameStart++)
                    if (Sections[i][NameStart] == '.' || Sections[i][NameStart + 1] == '<')
                        break;
                Name = Sections[i].Substring(1, NameStart - 1);

                //get Sell price
                ToSearch = "id=\"SellRate";
                int SellDivStart = Sections[i].IndexOf(ToSearch) + ToSearch.Length;
                for (; SellDivStart < Sections[i].Length; SellDivStart++)
                    if (Sections[i][SellDivStart] == '>')
                        break;
                SellDivStart++;
                int SellDivEnd = Sections[i].IndexOf('<', SellDivStart);
                double SellPrice = 0;
                double.TryParse(Sections[i].Substring(SellDivStart, SellDivEnd - SellDivStart), out SellPrice);

                //sell sentiment
                ToSearch = "Currently, ";
                int SellSentimentStart = Sections[i].IndexOf(ToSearch, SellDivStart) + ToSearch.Length;
                int SellSentimentEnd = Sections[i].IndexOf('%', SellSentimentStart);
                double SellSentiment = 0;
                double.TryParse(Sections[i].Substring(SellSentimentStart, SellSentimentEnd - SellSentimentStart), out SellSentiment);

                //buy sentiment
                ToSearch = "are BUY and ";
                int BuySentimentStart = Sections[i].IndexOf(ToSearch, SellSentimentStart) + ToSearch.Length;
                int BuySentimentEnd = Sections[i].IndexOf('%', BuySentimentStart);
                double BuySentiment = 0;
                double.TryParse(Sections[i].Substring(BuySentimentStart, BuySentimentEnd - BuySentimentStart), out BuySentiment);

                //get Buy price
                ToSearch = "id=\"BuyRate";
                int BuyDivStart = Sections[i].IndexOf(ToSearch, BuySentimentEnd) + ToSearch.Length;
                for (; BuyDivStart < Sections[i].Length; BuyDivStart++)
                    if (Sections[i][BuyDivStart] == '>')
                        break;
                BuyDivStart++;
                int BuyDivEnd = Sections[i].IndexOf('<', BuyDivStart);
                double BuyPrice = 0;
                double.TryParse(Sections[i].Substring(BuyDivStart, BuyDivEnd - BuyDivStart), out BuyPrice);

                if (SellPrice != 0 && SellSentiment != 0 && BuySentiment != 0 && BuyPrice != 0)
                    AddRecord(Name, SellPrice, BuyPrice, SellSentiment, BuySentiment);
            }
        }

        public void AddRecord(string Name, double SellPrice, double BuyPrice, double SellSentiment, double BuySentiment)
        {
            bool ValueUpdated = false;
            foreach (var itr in DataHistory)
                if (itr.Name == Name)
                {
                    itr.AddValue(SellPrice, BuyPrice, SellSentiment, BuySentiment);
                    ValueUpdated = true;
                    break;
                }
            if(ValueUpdated==false)
            {
                StockDataHistory h = new StockDataHistory();
                h.AddValue(SellPrice, BuyPrice, SellSentiment, BuySentiment);
                DataHistory.Add(h);
            }
 /*           StockDataSample sample = new StockDataSample();
            sample.Name = Name;
            sample.SellPrice = SellPrice;
            sample.BuyPrice = BuyPrice;
            sample.SellSentiment = SellSentiment;
            sample.BuySentiment = BuySentiment;*/
        }
    }
}
