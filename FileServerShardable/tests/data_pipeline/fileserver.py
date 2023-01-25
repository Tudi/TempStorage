import socket
from typing import Dict, Tuple

from .tcp_lib import get_data_packet, send_data_packet, SocketContext
from ..config import FileServerConfig
from .fileserver_exception import FileServerException

# FS_* variables are defined by the fileserver
# data-pipeline-fileserver/common/request_response_definitions.h
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


class FileServerClient:
    """
        Message Structure (to and from file server)

            Packet Header
                - (4 bytes) operation packet size (does not include Packet Header)
                - (1 byte)  message type, FS_*

            Choose One
                Operation Packet (FS_RESPONSE_CODE)
                    - (4 bytes) error code
                    - (? bytes) error string

                Operation Packet (FSPT_<GET/SAVE>_FILE)
                    - (1 byte)  record type
                    - (4 bytes) record id
                    - (? bytes) data, size = packet_header.operation_packet_size - 5
                                        The 5 is the Operation Packet Header of
                                            - operation type
                                            - record id

                Operation Packet (anything else)
                    - No support for _END_CONNECTION/PING yet

                    If an un-expected operation type is received, call connection error.
    """
    config: FileServerConfig
    file_type_map: Dict[str, int]

    def __init__(
            self,
            config: FileServerConfig
    ):
        self.config = config

        self.file_type_map = {
            'profile': PROFILE_FILE_TYPE,
            'company': COMPANY_FILE_TYPE
        }

    def get_socket_context(self, file_id: int) -> SocketContext:
        return SocketContext(
            host=self.config.hosts[file_id % self.config.num_shards],
            port=self.config.port
        )

    def handle_response(
            self,
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
                raise FileServerException(
                    key=f"File Server Error of {error_code}, {error_string_length}",
                    message=error_string,
                    log=True,
                    deadpool=False,
                    not_found="not found" in error_string,
                    save_failed="save failed" in error_string
                )
        elif response_type != expected_response_type:
            raise FileServerException(
                key="Unexpected Response Type",
                message=f"Expected {expected_response_type} but got {response_type}",
                log=True,
                deadpool=False
            )
        return response_size, response_type

    def store_record(
            self,
            record: bytes,
            record_type: str,
            record_id: int
    ) -> None:
        record_type_int = self.file_type_map[record_type]
        with self.get_socket_context(file_id=record_id) as socket_context:
            file_size = len(record)
            send_data_packet(
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
            _ = self.handle_response(
                sock=socket_context.sock,
                expected_response_type=FS_RESPONSE_CODE
            )

    def retrieve_record(
            self,
            record_type: str,
            record_id: int
    ) -> bytes:
        record_type_int = self.file_type_map[record_type]
        with self.get_socket_context(file_id=record_id) as socket_context:
            # Retrieving the following file:
            send_data_packet(
                sock=socket_context.sock,
                packet_format='<IBBI',
                packet_data=(
                    FS_OPERATION_PACKET_HEADER_SIZE,
                    FS_GET_FILE,
                    record_type_int,
                    record_id
                )
            )
            # Get the response
            response_size, response_type = self.handle_response(
                sock=socket_context.sock,
                expected_response_type=FS_GET_FILE
            )
            file_size = response_size - FS_OPERATION_PACKET_HEADER_SIZE
            # Connection error raised if less than expected data is read.
            record_type, record_id, record = get_data_packet(
                sock=socket_context.sock,
                packet_format=f'<BI{file_size}s'
            )
            return record
