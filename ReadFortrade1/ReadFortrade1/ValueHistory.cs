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
            //need to reorganize this. Just testing for now
            Globals.Persistency.AddInstrumentValue(Name, pSellPrice, pBuyPrice, pSellSentiment);
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
    public class ValueHistory
    {
        public void AddRecord(string Name, double SellPrice, double BuyPrice, double SellSentiment, double BuySentiment)
        {
            bool ValueUpdated = false;
            foreach (var itr in Globals.DataHistory)
                if (itr.Name == Name)
                {
                    itr.AddValue(SellPrice, BuyPrice, SellSentiment, BuySentiment);
                    ValueUpdated = true;
                    break;
                }
            if (ValueUpdated == false)
            {
                StockDataHistory h = new StockDataHistory();
                h.Name = Name;
                h.AddValue(SellPrice, BuyPrice, SellSentiment, BuySentiment);
                Globals.DataHistory.Add(h);
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
