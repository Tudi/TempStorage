import socket
import struct
import time
import threading
import queue

# i guess this would go into some init
server_address = ('localhost', 3003)
connectionsMaxCount = 2
connectionsFree = queue.SimpleQueue()
connectionsActive = 0
socketOperationTimeout = 5 # given in seconds
TestIsRunOnWindows = 1 # make sure to disable this on linux

def createNewConnection():
    global socketOperationTimeout, server_address, connectionsActive, connectionsFree
    connectionsActive = connectionsActive + 1
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(server_address)
    sock.settimeout(socketOperationTimeout) # 5 seconds socket operation timeout
    connectionsFree.put(sock, block=False, timeout=None)
    print("Created new connection")

def is_socket_closed(sock: socket.socket) -> bool:
    try:
        # this will try to read bytes without blocking and also without removing them from buffer (peek only)
        if TestIsRunOnWindows :
            sock.settimeout(0)
            data = sock.recv(1, socket.MSG_PEEK)
        else:
            data = sock.recv(1, socket.MSG_DONTWAIT | socket.MSG_PEEK)

        if TestIsRunOnWindows :
            sock.settimeout(socketOperationTimeout)

        if len(data) == 0:
            return True
    except BlockingIOError:
        if TestIsRunOnWindows :
            sock.settimeout(socketOperationTimeout)
        return False  # socket is open and reading from it would block
    except ConnectionResetError:
        return True  # socket was closed for some other reason
    except Exception as e:
        print("unexpected exception when checking if a socket is closed " + str(e))
        return True
    return False

def getConnection():
    global socketOperationTimeout, server_address, connectionsActive, connectionsFree
    # can we create new connections if we have no free connections ?
    print("Trying to get a connection. Size " + str( connectionsFree.qsize() ) + " connections used " + str(connectionsActive))
    if connectionsFree.empty() == True and connectionsActive < connectionsMaxCount:
        createNewConnection()
        
    # try to fetch a connection
    try:
        print("       Trying to get a connection. Size " + str( connectionsFree.qsize() ) + " connections used " + str(connectionsActive))
        sock = connectionsFree.get(block=True, timeout=socketOperationTimeout * 2)
        if is_socket_closed(sock):
            print("       Socket is in a closed state")
            connectionsActive = connectionsActive - 1
            sock.close()
            return getConnection()
        return sock
    except Exception as e:
        print("Failed to get a connection to the server. Exception : " + str(e))
        
    return None

def releaseConnection(sock):
    global socketOperationTimeout, server_address, connectionsActive, connectionsFree
    if sock == None:
        return
    print("Someone released a connection")
    connectionsFree.put(sock, block=False, timeout=None)
    
def killConnection(sock):
    global socketOperationTimeout, server_address, connectionsActive, connectionsFree
    sock.close()
    connectionsActive = connectionsActive - 1
    
# =====================================================================
# need to replace this part with the code Ben made
def getPacketData(sock, amount_expected):
    amount_received = 0
    data = b''
    while amount_received < amount_expected:
        dataNow = sock.recv(amount_expected - amount_received)
        lenNow = len(dataNow)
        if lenNow == 0:
            return 0, None
        amount_received += lenNow
        data = data + dataNow
    return amount_received, data
    
# Probably a horrible example how to read data from the network
def getPacketFmt(sock, fmt):
    amount_received, data = getPacketData(sock, struct.calcsize(fmt) )
    if amount_received < struct.calcsize(fmt):
        print("unpack requires " + str(struct.calcsize(fmt)) + " but we only received " + str(amount_received))
    networkPacket = struct.unpack(fmt,data)
    return networkPacket, data
    
def fileserver_request_save(fileType, fileId, fileContent):
    fileSize = len(fileContent)
    fmt = '<IBBI' #{data size}{packet type}{file type}{file ID}{file data}
    networkPacket = struct.pack(fmt, fileSize + 5, 2, fileType, fileId) # packet type=2 ( save ) file type = 3, file Id = 4
    fullPacketToSend = networkPacket + fileData
    
    sock = getConnection()
    
    if sock != None :
        try :
            print("Sending packet to FS")
            sock.sendall(fullPacketToSend)

            _, rawPacket = getPacketFmt(sock, '<IBI')
            if len(rawPacket)==0:
                print("Server did not receive the packet. We should resend unless error")
            releaseConnection(sock)
            print("Handled request. Done with FS")
        except Exception as e:
            print("Failed to handle request. Exception : " + str(e))
            killConnection(sock)
    else:
        print("Failed to connect to the server")

    
# =====================================================================
# below code is just for testing
max_packet_size = 65000 # ! max packet size is limited to 64K - UDP header
WorkerThreadCount=3
WorkerThreadSendCount=5
fileData = "o" * (max_packet_size)
fileData = fileData.encode('ascii')
fileSize = len(fileData)
fileType = 1
fileId = 666

def WorkerThread():
    global fileId

    # this is not thread safe at all, but whatever
    myfileId = fileId
    fileId = fileId + 1000

    #send the file
    start_time = time.time()
    for _ in range(0,WorkerThreadSendCount):
        fileserver_request_save( 1, myfileId, fileData )
    end_time = time.time()
    
    res = end_time - start_time
    print('Thread Execution time:', res, 'seconds')
    print('1 request duration:', res / WorkerThreadSendCount * 1000, 'ms')

threads=[]
for i in range(0,WorkerThreadCount):
    threads.append( threading.Thread(target=WorkerThread, args=[]) )

for i in range(0,WorkerThreadCount):
    threads[i].start()
    
for i in range(0,WorkerThreadCount):
    threads[i].join()
