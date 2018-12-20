using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace BLFClient.Backend
{
    /// <summary>
    /// Generic network client implementation
    /// </summary>
    class NetworkClient
    {       
        // supports 1 read and 1 write threads
        private NetworkStream NetStream;
        // socket implementation
        private TcpClient client_tcpClient;
        //our way of handling packets
        Thread ReadThread;
        private BlockingCollection<byte[]> ReadQueue = new BlockingCollection<byte[]>();
        Thread WriteThread;
        private BlockingCollection<byte[]> WriteQueue = new BlockingCollection<byte[]>();
        ushort ConnectionCheckSum = 0; //this is sent by the server and we can compare to check if we are in sync

        /// <summary>
        /// Simnple query to see if this resource is still valid
        /// </summary>
        /// <returns></returns>
        public bool IsConnected()
        {
            if (client_tcpClient != null && client_tcpClient.Connected == true && NetStream != null)
                return true;
            return false;
        }

        /// <summary>
        /// Blocking function to pop 1 whole packet from the network stream
        /// </summary>
        /// <returns></returns>
        public byte[] ReadPacket()
        {
            // wait until we have a packet to return
            while (ReadQueue.Count == 0 && NetStream != null)
                Thread.Sleep(1);
            // pop a packet from our queue
            if (ReadQueue.Count == 0)
                return null;
            return ReadQueue.Take();
        }

        /// <summary>
        /// Queue a packet for sending. Queued packet are sent as a bytestream. Might not be 1 packet / send !
        /// </summary>
        /// <param name="p"></param>
        public void SendPacket(object p)
        {
            //create a byte array out of the packet and queue it to the sender thread
            WriteQueue.Add(NetworkPacketTools.StructureToByteArray(p));
        }

        /// <summary>
        /// Sonnect to a server and intialize write / read
        /// </summary>
        /// <param name="HostName">localhost</param>
        /// <param name="Port">7778</param>
        public void ConnectToServer(string HostName, int Port)
        {
            this.client_tcpClient = new TcpClient();
            //the actual connection
            try
            {
                this.client_tcpClient.Connect(HostName, Port);
            }
            catch
            {
                // Log error that we were not able to connect to the server
                client_tcpClient = null;
                return;
            }
            //remember our reader
            this.NetStream = this.client_tcpClient.GetStream();
            //create a writer thread
            WriteThread = new Thread(new ThreadStart(SocketWriteThread));
            //create a reader thread
            ReadThread = new Thread(new ThreadStart(SocketReadThread));
            WriteThread.Start();
            ReadThread.Start();
        }

        /// <summary>
        /// Dispose of resources that could be blocking
        /// This function needs to support consecutive calls
        /// </summary>
        public void Disconnect()
        {
            //close the socket and mark blocked read write threads as invalid
            if (client_tcpClient != null && client_tcpClient.Connected == true)
            {
                client_tcpClient.Close();
                client_tcpClient.Dispose();
                client_tcpClient = null;
            }
            //should force read / write threads to stop doing stuff
            if (NetStream != null)
                NetStream.Close();
            if (NetStream != null)
                NetStream.Dispose();
            NetStream = null;
        }

        /// <summary>
        /// Continuesly try to read whole packets as long as we are connected
        /// </summary>
        private void SocketReadThread()
        {
            while (NetStream != null && NetStream.CanRead)
            {
                ReadOnePacket();
                //avoid CPU overheating. This is not required for a blocking socket though
                //Thread.Sleep(1);
            }
        }

        /// <summary>
        /// Blocks until we got enough data to read one whole packet
        /// </summary>
        private void ReadOnePacket()
        {
            //nothing to read at this point
//            if (NetStream.DataAvailable == false)
//                return;

            // !!! should remake this to accept event larger packets. Once we have packet size, we should rename it to fit
            byte[] receiveBuffer = null;
            try
            {
                //read the header of the packet
                BLFWinHeader pkt = new BLFWinHeader();
                int BytesToReadHeader = System.Runtime.InteropServices.Marshal.SizeOf(pkt);
                byte[] receiveBuffer2 = new byte[BytesToReadHeader];
                receiveBuffer2[0] = 0; // making sure the packet delimiter is set to an invalid value
                //keep on reading until we actually sync to the beggining of a packet. This should be instant 99.9% of the case
                int BytesRead = 0;
                int BytesReadUntilHeader = 0;
                while (NetStream != null && receiveBuffer2[0] != 0xaa)
                {
                    BytesRead = NetStream.Read(receiveBuffer2, 0, 1);
                    BytesReadUntilHeader += BytesRead;
                }
                if (BytesRead != BytesReadUntilHeader)
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Network error. Had to resync to next packet header. Skipped " + BytesReadUntilHeader.ToString() + " Bytes ");

                //read the rest of the header
                BytesRead += NetStream.Read(receiveBuffer2, BytesRead, BytesToReadHeader - BytesRead);

                pkt = NetworkPacketTools.ByteArrayToStructure<BLFWinHeader>(receiveBuffer2);
                int BytesToRead = pkt.Length + NetworkPacketTools.PCK_HDR_TRM_LEN();
                receiveBuffer = new byte[BytesToRead];
                receiveBuffer2.CopyTo(receiveBuffer,0);
                //this might be an error. Ignore what we just read. Next time we will search for a packet header delimiter
                if (BytesToRead > 1024 * 4)
                {
                    // NetStream = null;
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Could not recv buffer from server. Thrown it. Too large " + BytesToRead.ToString());
                    return;
                }
                //keep on reading until we have a full packet
                while (BytesRead < BytesToRead && NetStream != null && NetStream.CanRead)
                    BytesRead += NetStream.Read(receiveBuffer, BytesToReadHeader, BytesToRead - BytesRead);

                //safety check
                if(receiveBuffer[BytesRead-1] != 0x55)
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Malformed packet. End delimiter is not matching. Wasted size " + BytesRead.ToString());
            }
            catch
            {
                NetStream = null;
                return;
            }

            //something bad happened ?
            if (NetStream == null || NetStream.CanRead == false)
                return;

            // if we got here, we read the whole packet
            ReadQueue.Add(receiveBuffer);
        }

        /// <summary>
        /// Continuesly check our send buffer and send one or multiple packets at a time
        /// </summary>
        private void SocketWriteThread()
        {
            while (NetStream != null)
            {
                //try to send all bytes in our buffer
                if (WriteQueue.Count != 0)
                {
                    byte[] buffer = WriteQueue.Take();
                    //                foreach (byte[] buffer in WriteQueue.GetConsumingEnumerable())
                    if (this.NetStream != null)
                    {
                        try
                        {
                            this.NetStream.Write(buffer, 0, buffer.Length);
                        }
                        catch(Exception e)
                        {
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Could not send to selver buffer size " + buffer.Length.ToString() + "Error : " + e.ToString());
                        };
                    }
                    else break;
                }
                //if we want to force send content. Maybe we should leave this to the socket implementation
//                this.NetStream.Flush();
                //sleep a bit
                Thread.Sleep(1);
            }
        }

        public ushort GetConnectionChecksum()
        {
            return ConnectionCheckSum;
        }

        public void SetConnectionChecksum(ushort NewVal)
        {
            ConnectionCheckSum = NewVal;
        }
    }
}
