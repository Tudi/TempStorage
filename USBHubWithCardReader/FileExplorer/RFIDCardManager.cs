using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FileExplorer
{
    public static class RFIDCardManager
    {
        public class RFIDCardDetails
        {
            public string cardID;
            public string EA_Name;
        }

        private static Dictionary<string, RFIDCardDetails> _cards;

        static RFIDCardManager()
        {
            _cards = new Dictionary<string, RFIDCardDetails>();
        }
        public static void LoadCardDetails()
        {
            for (int i = 0; i < 100; i++)
            {
                string confName = "Card_" + i.ToString();
                try
                {
                    string cardDetails = ConfigReader.GetConfigValue(confName);
                    if (cardDetails != null)
                    {
                        string[] cardDetailsParts = cardDetails.Split(',');
                        if (cardDetailsParts.Length > 0)
                        {
                            RFIDCardDetails card = new RFIDCardDetails();
                            card.cardID = cardDetailsParts[0];
                            card.EA_Name = cardDetailsParts[1];
                            _cards[card.cardID] = card;
                        }
                    }
                }
                catch { }
            }
        }
        public static string GetCardUser(string CardId)
        {
            if(_cards.ContainsKey(CardId) == false)
            {
                return "";
            }
            return _cards[CardId].EA_Name;
        }
    }
}
