using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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
        public void AddValue(double pSellPrice, double pBuyPrice, double pSellSentiment, double pBuySentiment, bool SkipAdd = false)
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
            PrevSellPrice = pSellPrice;
            PrevBuyPrice = pBuyPrice;
            //need to reorganize this. Just testing for now
            if (SkipAdd == false)
                Globals.Persistency.AddInstrumentValue(Name, pSellPrice, pBuyPrice, pSellSentiment);
//            Update();
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
        public double PrevSellPrice;
        public double PrevBuyPrice;
    }
    public class ValueHistory
    {
        public void LoadFromPersistency()
        {
            //try to restore previous values to our list. This is required to not send out alerts unless there is a real value change even after restart
            Globals.Persistency.LoadLastSamplesFromAllTables();
            FinishedLoading = true;
        }

        bool FinishedLoading = false;
        List<StockDataHistory> DataHistory = new List<StockDataHistory>();
        public StockDataHistory GetInstrumentStore(string Name)
        {
            foreach (var itr in DataHistory)
                if (itr.Name == Name)
                    return itr;
            return null;
        }
        public void AddRecord(string Name, double SellPrice, double BuyPrice, double SellSentiment, double BuySentiment)
        {
            bool ValueUpdated = false;
            foreach (var itr in DataHistory)
                if (itr.Name == Name)
                {
                    double SellPriceDifference = Math.Abs( itr.PrevSellPrice - SellPrice );
                    double BuyPriceDifference = Math.Abs(itr.PrevBuyPrice - BuyPrice);
                    double SellPriceChangePCT = Math.Abs(itr.PrevSellPrice / SellPrice);
                    double BuyPriceChangePCT = Math.Abs(itr.PrevBuyPrice / BuyPrice);
//                    if ((SellPriceChangePCT < (1-Globals.IgnorePriceChangePCT) || SellPriceChangePCT > (1 + Globals.IgnorePriceChangePCT))
//                        || (BuyPriceChangePCT < (1 - Globals.IgnorePriceChangePCT) || BuyPriceChangePCT > (1 + Globals.IgnorePriceChangePCT)))
                    if(itr.PrevSellPrice != SellPrice || itr.PrevBuyPrice != BuyPrice)
                        itr.AddValue(SellPrice, BuyPrice, SellSentiment, BuySentiment);
                    ValueUpdated = true;
                    break;
                }
            if (ValueUpdated == false)
            {
                StockDataHistory h = new StockDataHistory();
                h.Name = Name;
                h.AddValue(SellPrice, BuyPrice, SellSentiment, BuySentiment, FinishedLoading == false);
                DataHistory.Add(h);
            }
        }
    }
}
