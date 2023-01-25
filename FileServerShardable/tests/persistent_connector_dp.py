import socket
import time
import threading
from typing import Tuple
import data_pipeline.tcp_lib
from data_pipeline.tcp_lib import get_data_packet

SocketPool = data_pipeline.tcp_lib.SocketPool('localhost', 3003)

# copied from "fileserver.py" to not copy it from data-pipeline. Because it had a chain requirement of configs
FS_GET_FILE = 1
FS_SAVE_FILE = 2
FS_END_CONNECTION = 3
FS_PING = 4
FS_RESPONSE_CODE = 5

FS_NO_ERROR_CODE = 0  # Else, send to error_lib

FS_PACKET_HEADER_SIZE = 5  # bytes
FS_OPERATION_PACKET_HEADER_SIZE = 5  # bytes
FS_ERROR_CODE_SIZE = 4  # bytes, one integer

# Defined by the pipeline
PROFILE_FILE_TYPE = 1
COMPANY_FILE_TYPE = 2

def handle_response(
        sock: socket.socket,
        expected_response_type: int
) -> Tuple[int, int]:
    response_size, response_type = get_data_packet(
        sock=sock,
        packet_format='<IB'
    )
    if response_type ==  FS_RESPONSE_CODE:
        error_string_length = response_size - FS_ERROR_CODE_SIZE
        error_code, error_bytes = get_data_packet(
            sock=sock,
            packet_format=f'<I{error_string_length}s'
        )
        error_string = error_bytes.decode("UTF-8")
        if error_code != FS_NO_ERROR_CODE:
            raise Exception(
                key=f"File Server Error of {error_code}, {error_string_length}",
                message=error_string,
                log=True,
                deadpool=False,
                not_found="not found" in error_string,
                save_failed="save failed" in error_string
            )
    elif response_type != expected_response_type:
        raise Exception(
            key="Unexpected Response Type",
            message=f"Expected {expected_response_type} but got {response_type}",
            log=True,
            deadpool=False
        )
    return response_size, response_type


def store_record(record_type_int, record_id, record):
    with SocketPool.getSocketContextInstance() as socket_context:
        file_size = len(record)
        data_pipeline.tcp_lib.send_data_packet(
            sock=socket_context.sock,
            packet_format=f'<IBBI{file_size}s',
            packet_data=(
                file_size + FS_OPERATION_PACKET_HEADER_SIZE,
                FS_SAVE_FILE,
                record_type_int,
                record_id,
                record
            )
        )
        # Expecting a response code
        # If an error or anything else is returned, exception is raised
        _ = handle_response(
            sock=socket_context.sock,
            expected_response_type=FS_RESPONSE_CODE
        )

    
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
        store_record( 1, myfileId, fileData )
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
