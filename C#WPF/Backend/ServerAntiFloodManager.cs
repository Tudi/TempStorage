using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLFClient.Backend
{
    public class ServerAntiFloodManager
    {
        const long AntiSpamIntervalMS = 0;  // avoid bursting down the server on loadup. Can put 1 for each packet to wait 1 ms before a new one is sent
        long LastPacketStamp = 0;

        public bool CanSendNewPacket()
        {
            if(LastPacketStamp + AntiSpamIntervalMS <= Environment.TickCount)
                return true;
            return false;
        }

        public void OnPacketSent()
        {
            LastPacketStamp = Environment.TickCount;
        }
    }
}
