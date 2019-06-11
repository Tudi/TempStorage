using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net.Security;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace BLFClient.Backend
{
    /// <summary>
    /// Generic network client implementation
    /// </summary>
    public class NetworkClient
    {       
        // supports 1 read and 1 write threads
        private NetworkStream NetStream = null;
        private SslStream NetStreamssl = null;
        // socket implementation
        private TcpClient client_tcpClient;
        //our way of handling packets
        Thread ReadThread;
        private BlockingCollection<byte[]> ReadQueue = new BlockingCollection<byte[]>();
        Thread WriteThread;
        private BlockingCollection<byte[]> WriteQueue = new BlockingCollection<byte[]>();

        public NetworkClientBuildPacket PacketBuilder = null;
        public NetworkClientInterpretPacket PacketInterpreter = null;
        public string ServerIPAndPort;
//        ushort ConnectionCheckSum = 0; //this is sent by the server and we can compare to check if we are in sync

        public NetworkClient(string IP, int Port)
        {
            ServerIPAndPort = IP + ":" + Port.ToString();
            PacketBuilder = new NetworkClientBuildPacket(this);
            PacketInterpreter = new NetworkClientInterpretPacket(this);
        }

        /// <summary>
        /// Simnple query to see if this resource is still valid
        /// </summary>
        /// <returns></returns>
        public bool IsConnected()
        {
            if (client_tcpClient != null && client_tcpClient.Connected == true && ( NetStream != null || NetStreamssl != null ))
                return true;
            return false;
        }

        /// <summary>
        /// Non blocking function to pop 1 whole packet from the network stream
        /// </summary>
        /// <returns></returns>
        public byte[] ReadPacket()
        {
            // wait until we have a packet to return
//            while (ReadQueue.Count == 0 && ( NetStream != null || NetStreamssl != null ))
//                Thread.Sleep(1);
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
            if(Globals.IniFile.GetConfig("Options","SSLConnection","YES") == "YES")
            {
                this.NetStreamssl = new SslStream(client_tcpClient.GetStream(), false, new RemoteCertificateValidationCallback(ValidateServerCertificate),null);
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagNetwork, "Will try to create SSL connection to the server. This might hang if server has no SSL");
                NetStreamssl.AuthenticateAsClient(Globals.IniFile.GetConfig("Options", "SSLTargetHostName", "Unify"));
            }
            else
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
            if (NetStreamssl != null)
                NetStreamssl.Close();
            if (NetStreamssl != null)
                NetStreamssl.Dispose();
            NetStreamssl = null;
        }

        /// <summary>
        /// Continuesly try to read whole packets as long as we are connected
        /// </summary>
        private void SocketReadThread()
        {
            while ((NetStream != null && NetStream.CanRead) || (NetStreamssl != null && NetStreamssl.CanRead))
            {
                ReadOnePacket();
                //avoid CPU overheating. This is not required for a blocking socket though
                //Thread.Sleep(1);
            }
        }

        /// <summary>
        /// Blocks until we got enough data to read one whole packet
        /// </summary>
        byte[] RecvBytes = null;
        int ReadIndex;
        int WriteIndex;
        private void ReadOnePacket()
        {
            //first time init
            if(RecvBytes==null)
            {
                RecvBytes = new byte[1024 * 4 * 16];
                ReadIndex = 0;
                WriteIndex = 0;
            }

            const int PacketHeaderMarker = 0xAA;
            const int PacketFooterMarker = 0x55;
            const int PacketEnvelopeHeaderSize = 1;
            const int PacketEnvelopeFooterSize = 1;
            const int PacketSizeByteCount = 2;
            const int PacketEnvelopeSize = PacketEnvelopeHeaderSize + PacketSizeByteCount + PacketEnvelopeFooterSize;
            int PacketSize = 0;
            int ReadBlockSize = 3;
            //keep reading until we can find a full packet
            while (NetStream != null || NetStreamssl != null)
            {
                //something went seriously wrong if we got here. Exhausted our write buffer without reaching a valid packet
                if(WriteIndex + ReadBlockSize >= RecvBytes.Length)
                {
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Error:Socket read buffer was too small to resync to a start of a packet");
                    ReadIndex = 0;
                    WriteIndex = 0;
                }

                int BytesRead;
                try
                {
                    if(NetStreamssl != null)
                        BytesRead = NetStreamssl.Read(RecvBytes, WriteIndex, ReadBlockSize);
                    else
                        BytesRead = NetStream.Read(RecvBytes, WriteIndex, ReadBlockSize);
                }
                catch
                {
                    //maybe disconnected
                    ReadIndex = 0;
                    WriteIndex = 0;
                    return;
                }

                //retry reading until more data arrives. Maybe a read timout happened ?
                if (BytesRead == 0)
                    continue;
                WriteIndex += BytesRead;

                //do we have a packet start marker at the start ? Seek to a packet marker
                while (RecvBytes[ReadIndex] != PacketHeaderMarker && ReadIndex < WriteIndex)
                    ReadIndex++;

                //need more data to deduce packet size ?
                if (ReadIndex + PacketSizeByteCount >= WriteIndex)
                    continue;

                //what packet size are we expecting at this marker ?
                PacketSize = RecvBytes[ReadIndex + PacketEnvelopeHeaderSize] + RecvBytes[ReadIndex + PacketEnvelopeHeaderSize + 1] * 256;

                //sanity check if packet size is too large, maybe we resynced to a bad packet start mark3er
                if(PacketSize > 4 * 1024)
                {
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Malformed packet detected. Too large. Seeking to the start of a new packet");
                    ReadIndex++;
                    continue;
                }

                //read 1 whole block instead 1 byte
                int AvailableBytes = WriteIndex - ReadIndex - PacketEnvelopeHeaderSize - PacketSizeByteCount;
                ReadBlockSize = PacketSize + PacketEnvelopeFooterSize - AvailableBytes; //read the end marker also
                if (ReadBlockSize < 0)
                    ReadBlockSize = 0;

                //do we have enough data for the whole packet ?
                if (WriteIndex < PacketSize + PacketEnvelopeSize)
                    continue;

                //check packet integrity
                if (RecvBytes[ReadIndex + PacketSize + PacketEnvelopeSize - 1] != PacketFooterMarker)
                {
                    Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Malformed packet detected. Missing end marker. Seeking to the start of a new packet");
                    //try to sync to a new packet start. This packet seems to be incomplete
                    ReadIndex++;
                    ReadBlockSize = 0;
                }
                else
                    break; //we received 1 full packet
            }

            byte[] Packet = new byte[PacketSize + PacketSizeByteCount];
            //rip out the usefull data. Skip copy of envelop data
            Array.Copy(RecvBytes, ReadIndex + PacketEnvelopeHeaderSize, Packet, 0, PacketSize + PacketSizeByteCount);
            if (ReadIndex != 0 && WriteIndex != ReadIndex + PacketSize + PacketEnvelopeSize)
            {
                Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Skipped " + ReadIndex + " network bytes while seaking for a new packet start");
                //shift remaining data to the left
                Array.Copy(RecvBytes, ReadIndex + PacketSize + PacketEnvelopeSize, RecvBytes, 0, WriteIndex - (ReadIndex + PacketSize + PacketEnvelopeSize));
            }
            //reset read and write index to the start
            WriteIndex = WriteIndex - (ReadIndex + PacketSize + PacketEnvelopeSize);
            ReadIndex = 0;
            Globals.Logger.LogPacket(Packet, false);
            //push the prepared packet in our packet queue
            ReadQueue.Add(Packet);// if we got here, we read the whole packet
        }

        /// <summary>
        /// Continuesly check our send buffer and send one or multiple packets at a time
        /// </summary>
        private void SocketWriteThread()
        {
            while (NetStream != null || NetStreamssl != null)
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
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Could not send to server buffer size " + buffer.Length.ToString() + "Error : " + e.ToString());
                        };
                    }
                    else if (this.NetStreamssl != null)
                    {
                        try
                        {
                            this.NetStreamssl.Write(buffer, 0, buffer.Length);
                        }
                        catch (Exception e)
                        {
                            Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Could not send to ssl server buffer size " + buffer.Length.ToString() + "Error : " + e.ToString());
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

 /*       public ushort GetConnectionChecksum()
        {
            return ConnectionCheckSum;
        }*/
/*
        public void SetConnectionChecksum(ushort NewVal)
        {
            ConnectionCheckSum = NewVal;
        }*/

        public static bool ValidateServerCertificate( object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors)
        {
            //all good
            if (sslPolicyErrors == SslPolicyErrors.None)
            {
                return true;
            }
            // certificate might be valid, just not trusted. We trust anything as long as it comes from the server
            if(sslPolicyErrors == SslPolicyErrors.RemoteCertificateChainErrors && chain.ChainStatus.First().Status == X509ChainStatusFlags.UntrustedRoot)
            {
                return true;
            }

            Globals.Logger.LogString(LogManager.LogLevels.LogFlagError, "Certificate error: " + sslPolicyErrors.ToString() );

            // refuse connection
            return false;
        }
    }
}
