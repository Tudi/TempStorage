using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLFClient.Backend
{
    public class IndexCardManager
    {
        static long IndexCardGUID = 1;
        List<IndexCard> IndexCards;

        public IndexCardManager()
        {
            IndexCards = new List<IndexCard>();
        }

        public void IndexCardAdd(IndexCard ic)
        {
            ic.SetGUID(IndexCardGUID++);
            IndexCards.Add(ic);
        }

        public void IndexCardDelete(IndexCard ic)
        {
            IndexCards.Remove(ic);
        }

        public IndexCard IndexCardGet(long GUID)
        {
            foreach (IndexCard ic in IndexCards)
                if (ic.GetGUID() == GUID)
                    return ic;
            return null;
        }

        public IndexCard IndexCardGet(string Name)
        {
            foreach (IndexCard ic in IndexCards)
                if (ic.GetName() == Name)
                    return ic;
            return null;
        }
        public void SaveIndexCards()
        {
            //remove old index card list
            Globals.Config.RemoveSection("Folders");
            //save new index card list
            if (IndexCards != null)
            {
                int ConfigIndex = 0;
                foreach (var Item in IndexCards)
                {
                    if (Item.SkipSave() == true)
                        continue;
                    Item.Save(ConfigIndex);
                    ConfigIndex++;
                }
            }
        }
    }
}
