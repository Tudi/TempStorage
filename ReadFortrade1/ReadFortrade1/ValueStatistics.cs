using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ReadFortrade1
{
    public class InstrumentValuePair
    {
        public InstrumentValuePair(long pStamp, double pSell)
        {
            Stamp = pStamp;
            SellValue = pSell;
        }
        public long Stamp;
        public double SellValue;
    }
    public class ValueStatistics
    {
        /*
         * DaysInPast = 0 means current day
         */
        public static void GetInstrumentDailyInversionCount(string InstrumentName, int DaysInPast, int DaysLong, double ExpectedProfitFlat, double ExpectedProfitPCTSpread)
        {
            //get all values 
            long StartStamp = DBHandler.GetUnixStampStartDay(0, 0, -DaysInPast);
            long EndStamp = DBHandler.GetUnixStampStartDay(0, 0, -DaysInPast + DaysLong);
            List<InstrumentValuePair> values = Globals.Persistency.GetInstrumentValues(InstrumentName, StartStamp, EndStamp);
            double SpreadAvg = Globals.Persistency.GetInstrumentSpread(InstrumentName, StartStamp, EndStamp);
            //if sell value drops below spread + profit, we call it an inversion point
            //check each inversion point. We presume we buy/sell at first value
            int TimesWeCouldSell = 0;
            int TimesWeCouldBuy = 0;
            double PrevBuyAtValue = 0;
            double PrevSellAtValue = 0;
            double ExpectedProfitFlatSpread = SpreadAvg * ExpectedProfitPCTSpread / 100;
            foreach (InstrumentValuePair itr in values)
            {
                double CurSellValue = itr.SellValue;
                double CurBuyValue = itr.SellValue + SpreadAvg;
                if (PrevSellAtValue == 0)
                    PrevSellAtValue = CurSellValue - SpreadAvg;
                if (PrevBuyAtValue == 0)
                    PrevBuyAtValue = CurBuyValue + SpreadAvg;

                if (CurSellValue + ExpectedProfitFlat < PrevSellAtValue || CurSellValue + ExpectedProfitFlatSpread < PrevSellAtValue)
                {
                    PrevSellAtValue = CurSellValue - SpreadAvg;
                    TimesWeCouldSell++;
                }
                if (CurBuyValue - ExpectedProfitFlat > PrevBuyAtValue || CurBuyValue - ExpectedProfitFlatSpread > PrevBuyAtValue)
                {
                    PrevBuyAtValue = CurBuyValue + SpreadAvg;
                    TimesWeCouldBuy++;
                }
            }
            Console.WriteLine("Could flip " + InstrumentName + " sell = " + TimesWeCouldSell.ToString() + " buy = " + TimesWeCouldBuy.ToString() + " times ");
        }

        public static void CalcInversionEachInstrument(int SinceDaysInPast,int NumberOfDays, double ExpectedProfitFlat, double ExpectedProfitPCTSpread)
        {
            List<string> InstrumentsWithValues = Globals.Persistency.GetAllInstrumentNames();
            foreach (string itr in InstrumentsWithValues)
                GetInstrumentDailyInversionCount(itr, SinceDaysInPast, NumberOfDays, ExpectedProfitFlat, ExpectedProfitPCTSpread);
        }

            /*        public List<InstrumentValuePair> ExpandToPrecision(List<InstrumentValuePair> db, int PrecisionSeconds)
                    {
                        long SecondsInDay = 24 * 1 * 60 * 60;
                        long TimeSlots = SecondsInDay/PrecisionSeconds;
                        return db;
                    }*/
    }
}
