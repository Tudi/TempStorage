using System;
using System.Collections.Generic;
using System.Text;

namespace FlowerSeller
{
    public class Flower
    {
        //types of flowers supported
        //used enum to detect if unsupported flower type is used
        public enum FlowerTypes
        {
            Unused,
            Trandafir,
            Gladiola,
            Orhidea
        };
        //initialize flower attributes
        public Flower()
        {
            FlowerType = FlowerTypes.Unused;
            cost = 0;
        }
        //create flower instances like this
        public Flower(FlowerTypes NewType)
        {
            SetType(NewType);
        }
        //we set this once an instance is created
        public void SetType(FlowerTypes NewType)
        {
            FlowerType = NewType;
            cost = ConfigHandler.GetFlowerCost(FlowerType);
        }
        //interface for cost property
        public int GetCost() { return cost; }
        //type of the flower
        FlowerTypes FlowerType;
        //cost is not fixed based on type. In case special cases need to be handled. Ex : promotions, old flowers ...
        int cost;
    };
}
