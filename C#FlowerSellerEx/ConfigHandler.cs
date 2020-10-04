using System;
using System.Collections.Generic;
using System.Text;

namespace FlowerSeller
{
    //values loaded from some sort of database or config file
    public class ConfigHandler
    {
        //this gets loaded from some database
        //right now it's hardcoded
        public static int GetFlowerCost(Flower.FlowerTypes FlowerType)
        {
            if (FlowerType == Flower.FlowerTypes.Trandafir)
                return 10;
            else if (FlowerType == Flower.FlowerTypes.Orhidea)
                return 30;
            else if (FlowerType == Flower.FlowerTypes.Gladiola)
                return 15;
           return -1;
        }
        public static int GetPackagingCost()
        {
            return 2;
        }
        //event function to let know next generation of coders that static data is used
        public static void EventBucketFactoryUsedConstants()
        {

        }
    }
}
