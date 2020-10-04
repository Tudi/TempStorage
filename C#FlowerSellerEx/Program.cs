using System;

namespace FlowerSeller
{
    class Program
    {
        static void Main(string[] args)
        {
            //our sales report keeper
            SalesReports BookKeeper = new SalesReports();

            //make sales
            for(int i=0;i<20;i++)
            {
                BucketFlower bf = BucketBuilder.BuildBucket(BucketTypes.Large);
                BookKeeper.AddSoldBucket(bf);
            }
            for (int i = 0; i < 15; i++)
            {
                BucketFlower bf = BucketBuilder.BuildBucket(BucketTypes.Medium);
                BookKeeper.AddSoldBucket(bf);
            }
            for (int i = 0; i < 65; i++)
            {
                BucketFlower bf = BucketBuilder.BuildBucket(BucketTypes.Small);
                BookKeeper.AddSoldBucket(bf);
            }
            for (int i = 0; i < 20; i++)
            {
                BucketFlower bf = BucketBuilder.BuildBucket(BucketTypes.Single,Flower.FlowerTypes.Trandafir);
                BookKeeper.AddSoldBucket(bf);
            }
            for (int i = 0; i < 10; i++)
            {
                BucketFlower bf = BucketBuilder.BuildBucket(BucketTypes.Single, Flower.FlowerTypes.Orhidea);
                BookKeeper.AddSoldBucket(bf);
            }
            for (int i = 0; i < 5; i++)
            {
                BucketFlower bf = BucketBuilder.BuildBucket(BucketTypes.Single, Flower.FlowerTypes.Gladiola);
                BookKeeper.AddSoldBucket(bf);
            }

            //print out the monthly report
            BookKeeper.PrintSalesReportShort();
        }
    }
}
