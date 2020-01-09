using System;
using System.Collections.Generic;
using System.Diagnostics;
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
    //number of seconds in an interval
    public class TimeValues
    {
        public static int MinuteToSecond = 60; // 60 seconds in 1 minute
        public static int HourToSecond = 60 * MinuteToSecond;
        public static int DayToSecond = 24 * HourToSecond;
        public static int WeekToSecond = 7 * DayToSecond;
        public static int DayToMinute = 24 * 60;
    }
    public class ValueStatistics
    {
        static string StrValueSeparator = " | ";

        public static double DynamicRound(double val, int digits = 4)
        {
            double tval = val - (long)val;
            int LeftShifts = 0;
            while(val<1)
            {
                val = val * 10;
                LeftShifts++;
            }
            if (digits > LeftShifts)
                LeftShifts = digits;
            return Math.Round(val, LeftShifts);
        }
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
            SpreadAvg = SpreadAvg * 2; //seems like this is applied at buy and sell. The buy and sell spread might be different !
            //if sell value drops below spread + profit, we call it an inversion point
            //check each inversion point. We presume we buy/sell at first value
            int TimesWeCouldSell = 0;
            int TimesWeCouldBuy = 0;
            double PrevBuyAtValue = 0;
            double PrevSellAtValue = 0;
            double ExpectedProfitFlatSpread = SpreadAvg * ExpectedProfitPCTSpread / 100;
            double SumOfBuyValues = 0;
            int NumberOfBuyValues = 0;
            foreach (InstrumentValuePair itr in values)
            {
                double CurSellValue = itr.SellValue;
                double CurBuyValue = itr.SellValue + SpreadAvg;
                if (PrevSellAtValue == 0)
                    PrevSellAtValue = CurSellValue - SpreadAvg;
                if (PrevBuyAtValue == 0)
                    PrevBuyAtValue = CurBuyValue + SpreadAvg;

                SumOfBuyValues += CurBuyValue;
                NumberOfBuyValues++;

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
//            Globals.Logger.Log("Could flip " + InstrumentName + " sell=" + TimesWeCouldSell.ToString() + " buy=" + TimesWeCouldBuy.ToString() + " times ");
            int FlipCount = TimesWeCouldSell;
            if (TimesWeCouldBuy < FlipCount)
                FlipCount = TimesWeCouldBuy;
            double Levarage = 10;
            double AmountWeCouldBuy = 10000 * Levarage / (SumOfBuyValues / NumberOfBuyValues);
            double AmountWeCouldGain = Math.Round(ExpectedProfitFlatSpread * FlipCount * AmountWeCouldBuy,1);
            Globals.Logger.Log("Could flip " + InstrumentName + ":" + FlipCount.ToString() + " gain " + AmountWeCouldGain.ToString() + " $ ");
        }

        public static void CalcInversionEachInstrument(int SinceDaysInPast,int NumberOfDays, double ExpectedProfitFlat, double ExpectedProfitPCTSpread)
        {
            List<string> InstrumentsWithValues = Globals.Persistency.GetAllInstrumentNames();
            foreach (string itr in InstrumentsWithValues)
                GetInstrumentDailyInversionCount(itr, SinceDaysInPast, NumberOfDays, ExpectedProfitFlat, ExpectedProfitPCTSpread);
        }

        public static void TransactionsDoneInPeriod(string InstrumentName, int PeriodInMinutes, int DurationMinutes)
        {
            int NumberOfPeriods = DurationMinutes / PeriodInMinutes;
            long Now = DBHandler.GetUnixStamp();
            string ToPrint = "";
            for (int PeriodIndex = 0; PeriodIndex < NumberOfPeriods; PeriodIndex++)
            {
                long StartStamp = Now - (PeriodIndex-0) * PeriodInMinutes * TimeValues.MinuteToSecond;
                long EndStamp = Now - (PeriodIndex-1) * PeriodInMinutes * TimeValues.MinuteToSecond;
                List<InstrumentValuePair> values = Globals.Persistency.GetInstrumentValues(InstrumentName, StartStamp, EndStamp);
                int TransactionsDoneInPeriod = values.Count;
                ToPrint += TransactionsDoneInPeriod.ToString() + ",";
            }
            Globals.Logger.Log("Transaction count for " + InstrumentName + " : " + ToPrint);
        }

        public static void CalcTransactionsEachInstrument()
        {
            List<string> InstrumentsWithValues = Globals.Persistency.GetAllInstrumentNames();
            foreach (string itr in InstrumentsWithValues)
                TransactionsDoneInPeriod(itr, TimeValues.DayToMinute, TimeValues.DayToMinute * 10);
        }

        public static void GetTopXLargestTransactions(string InstrumentName, int PeriodInMinutes)
        {
            long Now = DBHandler.GetUnixStamp();
            long StartStamp = Now - PeriodInMinutes * TimeValues.MinuteToSecond;
            long EndStamp = Now;
            List<InstrumentValuePair> values = Globals.Persistency.GetInstrumentValues(InstrumentName, StartStamp, EndStamp);
            //convert the values to differences
            double PrevValue = 0;
            foreach(var itr in values)
            {
                if(PrevValue == 0)
                {
                    PrevValue = itr.SellValue;
                    itr.SellValue = 0;
                    continue;
                }
                double NewValue = Math.Abs(itr.SellValue - PrevValue);
                PrevValue = itr.SellValue;
                itr.SellValue = NewValue;
            }

            //sort list by transaction value
            values.Sort(delegate (InstrumentValuePair x, InstrumentValuePair y)
            {
                if (y.SellValue < x.SellValue) return -1;
                if (y.SellValue > x.SellValue) return 1;
                return 0;
            });
            int PrintedValueCount = 0;
            string ToPrint = "";
            foreach(var itr in values)
            {
                ToPrint += DynamicRound(itr.SellValue,1).ToString() + ",";
                PrintedValueCount++;
                if (PrintedValueCount > 10)
                    break;
            }
            Globals.Logger.Log("Top transactoins for " + InstrumentName + " : " + ToPrint);
        }

        public static void GetTopXLastestTransactionsAllInstruments()
        {
            List<string> InstrumentsWithValues = Globals.Persistency.GetAllInstrumentNames();
            foreach (string itr in InstrumentsWithValues)
                GetTopXLargestTransactions(itr, TimeValues.DayToMinute * 10);
        }

        public static string GetChangePCT(string InstrumentName, int PeriodInSeconds, int PeriodShiftSeconds, int NumberOfPeriods = 1, bool Print = true)
        {
            long Now = DBHandler.GetUnixStamp();
            string ToPrint = "";
            int ValuesAdded = 0;
            for (int i = 0; i < NumberOfPeriods; i++)
            {
                long StartStamp1 = Now - i * PeriodShiftSeconds - 1 * PeriodInSeconds;
                long EndStamp1 = Now - i * PeriodShiftSeconds - 0 * PeriodInSeconds;
                double Average1 = GetInstrumentAveragePricePeriod(InstrumentName, StartStamp1, EndStamp1);
                long StartStamp2 = Now - i * PeriodShiftSeconds - 2 * PeriodInSeconds;
                long EndStamp2 = Now - i * PeriodShiftSeconds - 1 * PeriodInSeconds;
                double Average2 = GetInstrumentAveragePricePeriod(InstrumentName, StartStamp2, EndStamp2);
                double PCTChange = Math.Round(( Average1 - Average2 ) * 100 / Average2,2);
                ToPrint += PCTChange.ToString() + StrValueSeparator;
                if (!Double.IsNaN(PCTChange) && !Double.IsInfinity(PCTChange))
                    ValuesAdded++;
            }
            if (ToPrint.Length > 0)
                ToPrint = ToPrint.Substring(0, ToPrint.Length - StrValueSeparator.Length);
            if (ValuesAdded > 0 && Print == true)
                Globals.Logger.Log("Change PCT for " + InstrumentName + " : " + ToPrint);
            return ToPrint;
        }

        public static double GetInstrumentAveragePricePeriod(string InstrumentName, long StartStamp, long EndStamp)
        {
            List<InstrumentValuePair> values = Globals.Persistency.GetInstrumentValues(InstrumentName, StartStamp, EndStamp);
            double ValueSum = 0;
            double ValueCount = 0;
            InstrumentValuePair Prev = null;
            foreach (var itr in values)
            {
                if(Prev == null)
                {
                    Prev = itr;
                    continue;
                }
                //calculate the average value between the 2 points
                Debug.Assert(Prev.Stamp< itr.Stamp); // values are in theory ordered by increasing timestamps
                long TimeDiff = itr.Stamp - Prev.Stamp;
                double AvgValue = (itr.SellValue + Prev.SellValue) / 2;
                Debug.Assert(AvgValue < itr.SellValue * 1.10); // no more than 10% change is expected ?
                Debug.Assert(AvgValue > itr.SellValue * 0.90); // no more than 10% change is expected ?
                //we estimate 1 value for every second
                ValueSum += AvgValue * TimeDiff;
                ValueCount += TimeDiff;
                Prev = itr;
            }
            return ValueSum / ValueCount;
        }

        public static void GetChangePCTAllInstruments()
        {
            List<string> InstrumentsWithValues = Globals.Persistency.GetAllInstrumentNames();
            foreach (string itr in InstrumentsWithValues)
                GetChangePCT(itr, TimeValues.DayToSecond, TimeValues.DayToSecond, 10);
        }

        public static string CalcPivot(string InstrumentName, int PeriodInSeconds, int PeriodShiftSeconds, int NumberOfPeriods = 1, bool Print = true)
        {
            int PrecisionRequired = Globals.Persistency.GetInstrumentPrecision(InstrumentName);
            long Now = DBHandler.GetUnixStamp();
            string ToPrint = "";
            int ValuesAdded = 0;
            for (int i = 0; i < NumberOfPeriods; i++)
            {
                long StartStamp1 = Now - i * PeriodShiftSeconds - 1 * PeriodInSeconds;
                long EndStamp1 = Now - i * PeriodShiftSeconds - 0 * PeriodInSeconds;
                double Average1 = GetInstrumentAveragePricePeriod(InstrumentName, StartStamp1, EndStamp1);
                ToPrint += Math.Round(Average1, PrecisionRequired).ToString() + StrValueSeparator;
                if (!Double.IsNaN(Average1) && !Double.IsInfinity(Average1))
                    ValuesAdded++;
            }
            if (ToPrint.Length > 0)
                ToPrint = ToPrint.Substring(0, ToPrint.Length - StrValueSeparator.Length);
            if (ValuesAdded > 0 && Print == true)
                Globals.Logger.Log("Pivot for " + InstrumentName + " : " + ToPrint);
            return ToPrint;
        }

    }
}
