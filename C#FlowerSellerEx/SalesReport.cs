using System;
using System.Collections.Generic;
using System.Text;

namespace FlowerSeller
{
    //contains all the data required by the financing group to represent a sale
    public class Sale
    {
        public Sale(BucketTypes bt, Flower.FlowerTypes ft, int c)
        {
            BucketType = bt;
            FlowerType = ft;
            cost = c;
        }
        //one of the many ways to describe a sale
        public void PrintSale()
        {
            if(BucketType != BucketTypes.Single)
                Console.WriteLine(date.ToString() + " : Bucket : " + BucketType.ToString() + " cost : " + cost.ToString());
            else
                Console.WriteLine(date.ToString() + " : Flower : " + FlowerType.ToString() + " cost : " + cost.ToString());
        }
        //it's actually the money and not the cost
        public int GetCost() { return cost; }
        BucketTypes BucketType;
        Flower.FlowerTypes FlowerType;
        int cost;
        //auto filled when the sale is made. If you wish to load the sale from a DB, this needs to be made dynamic
        DateTime date = DateTime.Now;
    }
    //keep a list of sales that happened in a period of time. Monthly sales ?
    public class SalesReports
    {
        List<Sale> SalesMade = new List<Sale>();
        //every time we make a sale of a bucket, we should add it here also
        public void AddSoldBucket(BucketFlower bucket)
        {
            Sale sale = new Sale(bucket.GetBucketType(), bucket.GetSubType(), bucket.GetCost());
            SalesMade.Add(sale);
        }
        //generate report. One of the many ways to show what sales happened in the past
        public void PrintSalesReportShort()
        {
            int TotalSales = 0;
            foreach (Sale a in SalesMade)
            {
                a.PrintSale();
                TotalSales += a.GetCost();
            }
            Console.WriteLine("Sales made in this period : " + TotalSales.ToString());
        }
    }
}
