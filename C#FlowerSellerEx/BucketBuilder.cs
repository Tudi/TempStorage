using System;
using System.Collections.Generic;
using System.Text;

namespace FlowerSeller
{
    //bucket types we can create by our factory
    public enum BucketTypes
    {
        Single,
        Small,
        Medium,
        Large
    };
    //store required properties for a bucket
    public class BucketFlower
    {
        public BucketFlower(BucketTypes bt, Flower.FlowerTypes ft)
        {
            BucketType = bt;
            FlowerType = ft;
        }
        //function to help us construct a bucket of flowers
        public void Add(Flower f)
        {
            flowers.Add(f);
        }
        //get the sum of flower costs
        public int GetCost()
        {
            int ret = 0;
            ret += ConfigHandler.GetPackagingCost(); //price of packaging. This should be made more dynamic ?
            foreach (Flower a in flowers)
                ret += a.GetCost();
            return ret;
        }
        //interface to the type property
        public BucketTypes GetBucketType() { return BucketType;  }
        //special bucket when it contains only 1 flower
        public Flower.FlowerTypes GetSubType() { return FlowerType; }
        //bucket will contain a list of flowers
        List<Flower> flowers = new List<Flower>();
        //type of the
        BucketTypes BucketType;
        Flower.FlowerTypes FlowerType;
    }
    //contains static Data. Make sure to update
    public class BucketBuilder
    {
        //bucket factory. Can build buckets with specific presets
        public static BucketFlower BuildBucket(BucketTypes BucketType, Flower.FlowerTypes FlowerType = Flower.FlowerTypes.Unused)
        {
            BucketFlower ret = new BucketFlower(BucketType, FlowerType);
            if (BucketType == BucketTypes.Single)
            {
                if(FlowerType == Flower.FlowerTypes.Unused)
                {
                    Console.WriteLine("Invalif flower type!");
                    return null;
                }
                ret.Add(new Flower(FlowerType));
            }
            else if (BucketType == BucketTypes.Small)
            {
                ConfigHandler.EventBucketFactoryUsedConstants();
                for (int i = 0; i < 5; i++)
                    ret.Add(new Flower(Flower.FlowerTypes.Trandafir));
            }
            else if (BucketType == BucketTypes.Medium)
            {
                ConfigHandler.EventBucketFactoryUsedConstants();
                for (int i = 0; i < 6; i++)
                    ret.Add(new Flower(Flower.FlowerTypes.Trandafir));
                for (int i = 0; i < 5; i++)
                    ret.Add(new Flower(Flower.FlowerTypes.Gladiola));
            }
            else if (BucketType == BucketTypes.Large)
            {
                ConfigHandler.EventBucketFactoryUsedConstants();
                for (int i = 0; i < 9; i++)
                    ret.Add(new Flower(Flower.FlowerTypes.Trandafir));
                for (int i = 0; i < 10; i++)
                    ret.Add(new Flower(Flower.FlowerTypes.Gladiola));
                for (int i = 0; i < 3; i++)
                    ret.Add(new Flower(Flower.FlowerTypes.Orhidea));
            }
            return ret;
        }
    }
}
