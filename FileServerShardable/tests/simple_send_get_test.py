import socket
import struct

sock = 0

# creates a new connection to the server.
# Once a "get"/"save" request is served, the connection will be closed by the server
def CreateNewConnection():
    global sock
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # Connect the socket to the port where the server is listening
    server_address = ('localhost', 3002)
    print('connecting to %s port %s' % server_address)
    sock.connect(server_address)

def getPacketData(amount_expected):
    amount_received = 0
    print('      Expecting %d bytes' % (amount_expected))
    data = ""
    while amount_received < amount_expected:
        dataNow = sock.recv(amount_expected - amount_received)
        if dataNow == 0:
            break
        amount_received += len(dataNow)
        data = data + dataNow
    print('      Received %d==%d bytes' % (amount_received, len(data)))
    return amount_received, data

# Probably a horrible example how to read data from the network
def getPacketFmt(fmt):
    print('   get packet data with format %s' % (fmt))
    _, data = getPacketData( struct.calcsize(fmt) )
    networkPacket = struct.unpack(fmt,data)
    return networkPacket

# checks if the packet is expected to be a response code packet
# handles response codes : if code value is 0, all is good
def HandleResponseCodes(networkPacket, expectedPacket):
    if networkPacket[1] != 5:
        if expectedPacket == 1:
            print(networkPacket)
            print('Unexpected packet type "%s"' % networkPacket[1])
            print('received "%s"' % hex(int(networkPacket,2)))
            return 1
        else:
            return 0; # was not expecting a response code packet and indeed we got something else

    if expectedPacket == 1:
        if networkPacket[2] == 0:
            print('got response code 0. All good')
            return 0
        else:
            print('got response error code "%d"' % networkPacket[2])
            return 1
    else:
        formatStr = '<I%ds' % (networkPacket[0]-4) # error code is 4 bytes, the rest of the packet is the error string
        networkPacket2 = getPacketFmt(formatStr)
        print('got response error code "%d" and string "%s"' % (networkPacket2[0], networkPacket2[1]))
        return 1

    return 1

# send a BLOB of data to be saved server side. Data index is generated from "type"+"id"
def Send1File(fileType, fileId, file_data):
    # sending almost 2 packets at once
    # first part of the packet is always : (data size(4 bytes) + packet type(1 byte))
    # second packet is : file type ( 1 byte ) + file id ( 4 bytes ) + file data( dynamic size )
    fileSize = len(file_data)
    fmt = '<IBBI' #{data size}{packet type}{file type}{file ID}{file data}
    networkPacket = struct.pack(fmt, fileSize + 5, 2, fileType, fileId) # packet type=2 ( save ) file type = 3, file Id = 4

    print('Send1File:total packet bytes : %d. data size : %d data : "%s"' % (len(networkPacket), fileSize, file_data))

    sock.sendall(networkPacket)
    sock.sendall(file_data) # no idea how to append this data to networkPacket
 
    # Look for the response
    print('waiting for server confirmation')
    respPacket = getPacketFmt('<IBI')
    HandleResponseCodes(respPacket, 1)

# fetch a BLOB of data from the server. Data index is generated from "type"+"id"
# if all is good, returned packet type will be "1"
# on any error, returned packet type will be "5"
def Get1File(fileType, fileId):
    # sending almost 2 packets at once
    # first part of the packet is always : (data size(4 bytes) + packet type(1 byte))
    # second part of this packet is : file type ( 1 byte ) + file id ( 4 bytes )
    fmt = '<IBBI' #{data size}{packet type}{file type}{file ID}
    networkPacket = struct.pack(fmt, 5, 1, fileType, fileId) # data size=5, packet type=1 ( get ), file type = 3, file Id = 4

    print('Get1File:total packet bytes : %d' % (len(networkPacket)))
    
    sock.sendall(networkPacket)
 
    # Look for the response
    # first packet is always : (data size(4 bytes) + packet type(1 byte))
    respPacket = getPacketFmt('<IB')
    if HandleResponseCodes(respPacket, 0) != 0:
        return "";

    dataBlockSize = respPacket[0]
    print('expected data block size : %d' % dataBlockSize)
    
    # second packet is : file type ( 1 byte ) + file id ( 4 bytes ) + file data( dynamic size )
    dataBlock = sock.recv(dataBlockSize)
    if len(dataBlock) != dataBlockSize:
        print('expected data block size : %d, but only read %d' % (dataBlockSize, len(dataBlock)))
    #fileSize = dataBlockSize - 5 # substract the size of type + id

    return dataBlock[5:]


try:
    
    # Send data
    CreateNewConnection()
    file_data = '"I am the binary file content"'
    Send1File(3,4,file_data)
    
    print('##########################')
    # Get data
    CreateNewConnection()
    file_data2 = Get1File(3,4)
    
    print('##########################')
    # Get data
    CreateNewConnection()
    file_data2 = Get1File(3,5) # should produce an error message : "Entry not found"

    print(file_data2)

finally:
    print('closing socket')
    sock.close()
