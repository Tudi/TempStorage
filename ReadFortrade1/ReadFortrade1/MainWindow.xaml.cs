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
using System.Windows.Threading;

/*
 * main goal is to guess which way the graph will go today
 * - inspect today market activity
 * - inspect past market activity
 * - inspect location of the price ( is it high or is it low ? )
 * - we need to detect trends as soon as possible and ride a trend. It's only worth to ride a trend as long as we have enough swings of value
 * 
 * todo :
 * - make statistics of daily change PCT
 * - make statistics of trend(minute/30min/60min) period lengths
 * - make statistics of what it means to have a large jump in value
 * - statistics of : transaction (number of T, size of T, Volatility) at specific hour of the day
 * - make statistics of rate of change. How many times, how many small changes occure. Lots of people trading it ? Sentimental trading
 * - implement block based data store : if value is on 4 bytes, and we store 60 values in 1 minute
 * - statistics should be available for hour/day/week/month/year
 * - check which instrument has the highest probability of inversion in one day
 * - be able to calculate previous day pivot point
 * - check / confirm if day starts bearish or bulish for an instrument
 * - check confirm if there is a trend going on for a specific instrument
 * - check the probablity of trend flip based on daily / weekly / monthly pivots.
 * - propose budget for a specific instrument based on pivot location and trend chances
 * - create statistics of how many trades swings are done per period. This should also take into count the size of the swings. Are there big traders or small traders in the market ?
 * - create statistics for most stable instruments. Should not stay low or high for too much amount of time below or above pivot. How fast will our bad trade vome back to a zero ?
 * - create statistics. Is it trending ? Is it daily fluctuating ?
 * - make some statistics to try to guess what actions could create a specific trend
 * - generate 3 / 4 bar patterns based on past days / weeks. The older the pattern, the less influential
 * 
 * Self notes :
 * - heating oil-natural gas-coal are quite linked. If one increases the other one falls
 * - ! Avoid validating your decision before inspection ! Try to take decisions based on news and numbers. First seach news, check numbers. If all match, take actions. You need to make good deals and not feel sentimental
 * - never ever be greedy. Invest only a portion of balance in a specific type of stock trend
 * - try to make short term deals. Money gets blocked on wrong decisions and maybe never recover
 * - trading is sentiment based. What other traders are doing and hardly about what in reality happens
 * - no fixed strategy will work forever. there are many bots out there that will adapt to your actions and take counter actions
 */
namespace ReadFortrade1
{
    public class Globals
    {
        public static MessageLogger Logger = null;
        public static DBHandler Persistency = null;
        public static ValueHistory vHistory = null;
        public static PageParserWatchDog PageParserMonitor = null;
        public static TimeoutWatchDog TimeoutMonitor = null;
        public static NotificationWatchdog PriceChangeMonitor = null;
        public static long doubleScaler = 100000;
        public static double IgnorePriceChangePCT = 0.0001; // if Old/New is smaller than this ratio, ignore recording it to the DB
        public static DateTime LastFavoriteSectionParseStamp = DateTime.Now;
        public static bool AppIsRunning = true;
        public static int ThreadCycleSleep = 1000;
        public static int IETimeoutSeconds = 15;
    }
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            Globals.Logger = new MessageLogger();
            Globals.Persistency = new DBHandler();
            Globals.vHistory = new ValueHistory();
            Globals.vHistory.LoadFromPersistency();

            int ExecuteFunction = 1;
            if (ExecuteFunction == 1)
                CalcFlipChance();
            else if (ExecuteFunction == 2)
                Task.Factory.StartNew(() => ImportExtenalDB());
            else if (ExecuteFunction == 3)
            {
                Globals.PageParserMonitor = new PageParserWatchDog();
                Globals.TimeoutMonitor = new TimeoutWatchDog();
                Globals.PriceChangeMonitor = new NotificationWatchdog();
                //starting background threads for value fetching and processing
                Task.Factory.StartNew(() => Globals.PageParserMonitor.StartPageParserWatchdog());
                Task.Factory.StartNew(() => Globals.TimeoutMonitor.StartPageTimoutWatchdog());
                Task.Factory.StartNew(() => Globals.PriceChangeMonitor.StartPriceChangeWatchdog());
            }
            //            SendNotification.SendMessage("Test meail to sms");
        }
        private void CalcFlipChance()
        {
            ValueStatistics.CalcInversionEachInstrument(10, 10 + 2, 0.0001, 2);
            ValueStatistics.CalcTransactionsEachInstrument();
            ValueStatistics.GetTopXLastestTransactionsAllInstruments();
            ValueStatistics.GetChangePCTAllInstruments();
        }
        private void ImportExtenalDB()
        {
            Globals.Logger.Log("Starting importing Database");
            Globals.Persistency.ImportFromDB("Fortrade_live.db");
            Globals.Logger.Log("Done importing Database");
        }
    }
}
