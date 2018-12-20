using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLFClient.Backend
{
    public class ServerAntiFloodManager
    {
        const long AntiSpamIntervalMS = 1;  // avoid bursting down the server on loadup
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
