import logging
import socket
import struct
import threading
import time
from typing import Tuple

#need to add these to FileServerConfig
host = 'localhost'
port = 3003
socketTimeoutLimit = 30
maxPersistentConnections = 2 # !! make sure this is smaller than FSMaxConnections/ClientCount or workerthreads will block !!
                             # ideally, every worker thread will have it's own persistent connection

TestIsRunOnWindows = 1 # Note to self : remove all code related to windows


class SocketContext:
    sock: socket.socket
    owner: object

    def __init__(self, sock, owner):
        if sock == None:
            print("Trying to initialize socket context with non valid connection")
        self.sock = sock
        self.owner = owner

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.owner.releaseConnection(self.sock, exc_val)
        if exc_val:
            raise exc_val from exc_val

class SocketPool:
    host = ""
    port = ""
    connectionsCreated = 0
    maxPersistentConnections = 1
    isInitialized = 0
    listLock = threading.Lock()
    listIncreaseEvent = threading.Condition()
    connectionList = []

    def __init__(self, host: str, port: int):
        if self.isInitialized == 0:
            self.listLock.acquire()
            if self.isInitialized == 0: # same check, but threadsafe
                self.host = host
                self.port = port
                self.connectionsCreated = 0
                self.maxPersistentConnections = maxPersistentConnections
                print("One time initialization of SocketContext host:port " + self.host + ":" + str(self.port))
                self.isInitialized = 1
            self.listLock.release()
        else:
            if self.port != port or self.host != host:
                print("!! Only supports single host initialization for now !!")

    def getSocketContextInstance(self):
        return SocketContext(self._getConnection(), self)

    def __exit__(self, exc_type, exc_val, exc_tb):
        # maybe exc_val could be used to detect bad(timeout) sockets ?
        if exc_val:
            raise exc_val from exc_val

    def _createConnection(self):
        print("New connection is being created host:port " + self.host + ":" + str(self.port))
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((self.host,self.port))
        sock.settimeout(socketTimeoutLimit)
        return sock

    def releaseConnection(self, sock, badConnection):
        if badConnection: # would be great if socket read or write errors would be passed into this variable
            print("Socket is in bad state. Closing it")
            sock.close()
            self.listLock.acquire()
            self.connectionsCreated = self.connectionsCreated - 1
            self.listLock.release()
        else:
            self.listLock.acquire()
            self.connectionList.append(sock)
            print("Socket has been put back to pool. Available="+str(len(self.connectionList))+",Created="+str(self.connectionsCreated))
            self.listLock.release()
        # good or bad, we are allowed to wake up a thread to either create a new connection or to fetch the one we just released
        with self.listIncreaseEvent:
            self.listIncreaseEvent.notify()

    def _getConnection(self):
        self.listLock.acquire()
        sockLocal = None
        
        # we are out of persistent connection ?
        if len(self.connectionList) == 0:
            print("Have no free connections")
            # are we allowed to create new connections ?
            if self.connectionsCreated < self.maxPersistentConnections:
                try:
                    print("Allowed to create a new connection " + str(self.connectionsCreated) + " < " + str(self.maxPersistentConnections))
                    sockLocal = self._createConnection()
                    self.connectionsCreated = self.connectionsCreated + 1
                except Exception as exc:
                    logging.error("Failed to connect to %s:%s", self.host, self.port)
                    raise exc from exc
                finally:
                    self.listLock.release()
            else:
                self.listLock.release()
                print("wait until a connection is put back into the pool")
                # wait for a connection to be put back into the list of connections
                timeoutAt = time.time() + socketTimeoutLimit * 2
                while sockLocal == None and timeoutAt > time.time() and self.connectionsCreated == self.maxPersistentConnections:
                    while len(self.connectionList) == 0 and timeoutAt > time.time() and self.connectionsCreated == self.maxPersistentConnections:
                        with self.listIncreaseEvent:
                            self.listIncreaseEvent.wait(socketTimeoutLimit) # wait for some thread to finish with a connection
                    self.listLock.acquire()
                    if len(self.connectionList) > 0:
                        sockLocal = self.connectionList.pop()
                    # instead connections getting put back into the pool, they got thrown away
                    elif self.connectionsCreated < self.maxPersistentConnections:
                        self.listLock.release()
                        return self._getConnection()
                    self.listLock.release()
        # reuse existing persistent connection
        else:
            sockLocal = self.connectionList.pop()
            self.listLock.release()

        if sockLocal == None:
            print("Failed to obtain a connection before timeout") # maybe should raise an exception or something ?
        # if the socket we obtained is closed, create a new connection
        elif _is_socket_closed(sockLocal):
            print("socket is closed")
            sockLocal.close() # maybe it was just in a bad state and not fully closed
            try:
                sockLocal = self._createConnection()
            except Exception as exc:
                logging.error("Failed to connect to %s:%s", self.host, self.port)
                raise exc from exc
               
        # at this point we should have a valid connection
        return sockLocal
            
def _is_socket_closed(sock: socket.socket) -> bool:
    try:
        # this will try to read bytes without blocking and also without removing them from buffer (peek only)
        if TestIsRunOnWindows :
            sock.settimeout(0)
            data = sock.recv(1, socket.MSG_PEEK)
        else:
            data = sock.recv(1, socket.MSG_DONTWAIT | socket.MSG_PEEK)
        
        if TestIsRunOnWindows :
            sock.settimeout(socketTimeoutLimit)
            
        if len(data) == 0:
            return True
    except BlockingIOError:
        if TestIsRunOnWindows :
            sock.settimeout(socketTimeoutLimit)
        return False  # socket is open and reading from it would block
    except ConnectionResetError:
        return True  # socket was closed for some other reason
    except Exception as e:
        print("unexpected exception when checking if a socket is closed " + str(e))
        return True
    return False
  
def _get_data(
        sock: socket.socket,
        message_length: int,
        bufsize: int = 65536
) -> bytes:
    chunks = []
    bytes_read = 0
    while bytes_read < message_length:
        chunk = sock.recv(min(message_length - bytes_read, bufsize))
        if chunk == b'':
            raise ConnectionError("Expected data in socket, got none.")
        chunks.append(chunk)
        bytes_read += len(chunk)
    return b''.join(chunks)

def get_data_packet(
        sock: socket.socket,
        packet_format: str
) -> Tuple:
    message_length = struct.calcsize(packet_format)
    return struct.unpack(
        packet_format,
        _get_data(
            sock=sock,
            message_length=message_length
        )
    )

def send_data_packet(
        sock: socket.socket,
        packet_format: str,
        packet_data: Tuple
):
    packet = struct.pack(
        packet_format,
        *packet_data
    )
    packet_size = struct.calcsize(packet_format)
    sent_size = sock.send(packet)
    if sent_size != packet_size:
        raise ConnectionError(("Sent only " + str(sent_size) + " of " + str(packet_size) + " bytes."))
