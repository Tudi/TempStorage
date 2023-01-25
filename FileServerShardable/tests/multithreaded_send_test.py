import socket
import struct
import threading
import time

server_address = ('localhost', 3002)
max_packet_size = 65000 # ! max packet size is limited to 64K - UDP header
WorkerThreadCount=5
WorkerThreadSendCount=100
file_id = 0

def getPacketData(sock, amount_expected):
    amount_received = 0
    data = ""
    dataNow = sock.recv(amount_expected - amount_received)
    amount_received += len(dataNow)
    data = data + dataNow
    return amount_received, data

# Probably a horrible example how to read data from the network
def getPacketFmt(sock, fmt):
    _, data = getPacketData(sock, struct.calcsize(fmt) )
    networkPacket = struct.unpack(fmt,data)
    return networkPacket, data

def WorkerThread():
    global file_id

    fmt = '<IBBI' #{data size}{packet type}{file type}{file ID}{file data}
    networkPacket = struct.pack(fmt, fileSize + 5, 2, fileType, file_id) # packet type=2 ( save ) file type = 3, file Id = 4
    file_id = file_id + 1000

#    print >>sys.stderr, 'Send1File:total packet bytes : %d. data size : %d' % (len(networkPacket), fileSize)
    fullPacketToSend = networkPacket+fileData

    #send the file
    start_time = time.time()
    for _ in range(0, WorkerThreadSendCount):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = ('localhost', 3002)
        sock.connect(server_address)

        sock.settimeout(5)
        sock.sendall(fullPacketToSend)

        #wait for ack
        sock.settimeout(5)
        _, rawPacket = getPacketFmt(sock, '<IBI')
        if len(rawPacket)==0:
            print("Server did not receive the packet. We should resent unless error")
        sock.close()

    end_time = time.time()
    
    res = end_time - start_time
    print('Thread Execution time:', res, 'seconds')
    print('1 request duration:', res / WorkerThreadSendCount * 1000, 'ms')
    
    
fileData = "o" * (max_packet_size)
fileSize = len(fileData)
fileType = 1
file_id = 666

threads=[]
for i in range(0,WorkerThreadCount):
    threads.append( threading.Thread(target=WorkerThread, args=[]) )

for i in range(0,WorkerThreadCount):
    threads[i].start()
    
for i in range(0,WorkerThreadCount):
    threads[i].join()
