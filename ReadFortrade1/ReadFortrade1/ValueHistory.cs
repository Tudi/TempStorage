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
            TableName = null;
            PrevSellPrice = 0;
            PrevBuyPrice = 0;
            PrevBuySentiment = 0; 
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
            if (Name != null && TableName == null)
                TableName = Globals.Persistency.GetTableNameForInstrument(Name);
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
            //need to reorganize this. Just testing for now
            if (SkipAdd == false)
            {
                if((PrevSellPrice != pSellPrice) || PrevBuyPrice != pBuyPrice)
                    Globals.Persistency.AddInstrumentValue(Name, pSellPrice, TableName);
                if (PrevSpread != Math.Round(pBuyPrice - pSellPrice,7))
                    Globals.Persistency.AddSpreadValue(Name, pBuyPrice - pSellPrice, TableName);
                if (PrevBuySentiment != pBuySentiment)
                    Globals.Persistency.AddSentimentValue(Name, pBuySentiment, TableName);
            }
            SellSentiment = pSellSentiment;
            BuySentiment = pBuySentiment;
            PrevSellPrice = pSellPrice;
            PrevBuyPrice = pBuyPrice;
            PrevBuySentiment = pBuySentiment;
            PrevSpread = Math.Round(pBuyPrice - pSellPrice,7);
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
        public double PrevBuySentiment;
        public double PrevSpread;
        string TableName;
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
     //               if(itr.PrevSellPrice != SellPrice || itr.PrevBuyPrice != BuyPrice)
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
