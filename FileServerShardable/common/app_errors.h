#ifndef _APP_ERRORS_H_
#define _APP_ERRORS_H_

typedef enum FileServerAppErrorCodes
{
    ERR_FS_NO_ERROR = 0,
    ERR_FS_UNKNOWN_FILE_TYPE = 1,
    ERR_FS_NETWORK_SIZE_FILE_SIZE_MISMATCH = 2,
    ERR_FS_FAILED_TO_OPEN_FILE_FOR_WRITE = 3,
    ERR_FS_FILE_WRITE_NOT_COMPLETED = 4,
    ERR_FS_VALUE_NOT_INITIALIZED = 5,
    ERR_FS_PACKET_SIZE_TOO_LARGE = 6,
    ERR_FS_SOCKET_READ_NOT_ENOUGH_BYTES = 7,
    ERR_FS_SOCKET_WRITE_INTERRUPTED = 8,
    ERR_FS_NON_HANDLED_PACKET_TYPE = 9,
    ERR_FS_UNKNOWN_PACKET_TYPE = 10,
    ERR_FS_MEMORY_ALLOC_FAILED = 11,
    ERR_FS_ID_ARRAY_TOO_LARGE = 12,
    ERR_FS_FAILED_TO_OPEN_FILE_FOR_READ = 13,
    ERR_FS_FAILED_TO_MAP_FILE_FOR_READ = 14,
    ERR_FS_FAILED_TO_UNMAP_FILE = 15,
    ERR_FS_NO_INPUT_SIMILARITY_FILES = 16,
    ERR_FS_FAILED_TO_PARSE_GET_REQUEST_DATA = 17,
    ERR_FS_MALFORMED_END_CONNECTION_PACKET = 18,
    ERR_FS_MALFORMED_PING_PACKET = 19,
    ERR_FS_MALFORMED_GET_FILE_PACKET = 20,
    ERR_FS_FAILED_TO_INITIALIZE_RW_LOCK = 21,
    ERR_FS_FAILED_TO_UNITIALIZE_RW_LOCK = 22,
    ERR_FS_FAILED_TO_READ_LOCK = 23,
    ERR_FS_FAILED_TO_WRITE_LOCK = 24,
    ERR_FS_FAILED_TO_UNLOCK_READ_LOCK = 25,
    ERR_FS_FAILED_TO_UNLOCK_WRITE_LOCK = 26,
    ERR_FS_ENTRY_NOT_FOUND = 27,
    ERR_FS_ENTRY_SAVE_FAILED = 28,
    ERR_FS_DAO_INIT_FAILED = 29,
    ERR_FS_MALFORMED_SAVE_FILE_PACKET = 30,
    ERR_FS_SOCKET_TIMEOUT = 31,
    ERR_FS_SOCKET_SELECT_FAILED = 32,
    ERR_FS_SOCKET_READ_INTERRUPTED = 33,
    ERR_FS_SOCKET_WRITE_FAILED = 34,
    ERR_FS_FILE_READ_FAILED = 35,
    ERR_FS_PARAM_DAO_NULL = 36,
    ERR_FS_FILE_RENAME_FAILED = 37,
    ERR_FS_PARAM_SERVERDATA_NULL = 38,
    ERR_FS_SOCKET_LISTEN_FAILED = 39,
    ERR_FS_SOCKET_ACCEPT_FAILED = 40,
    ERR_FS_EVENT_CREATE_FAILED = 41,
    ERR_FS_SOCKET_INIT_FAILED = 42,
    ERR_FS_THREAD_CREATE_FAILED = 43,
    ERR_FS_MT_QUEUE_INIT_FAILED = 44,
    ERR_FS_SOCKET_CREATE_FAILED = 45,
    ERR_FS_SOCKET_BIND_FAILED = 46,
    ERR_FS_SOCKET_CONFIG_FAILED = 47,
    ERR_FS_SOCKET_CONFIG_RTIMEOUT_FAILED = 48,
    ERR_FS_SOCKET_CONFIG_WTIMEOUT_FAILED = 49,
    ERR_FS_SOCKET_CONFIG_RETRY_FAILED = 50,
    ERR_FS_SOCKET_CONFIG_TIMEOUT_FAILED = 51,
    ERR_FS_SOCKET_CONFIG_REUSEADDR_FAILED = 52,
    ERR_FS_SOCKET_CONFIG_NAGGLE = 53,
    ERR_FS_SOCKET_CONFIG_QUICK_ACK = 54,
    ERR_FS_MAX_KNOWN // make sure always the last
}FileServerAppErrorCodes;

static const char *g_err_code_str[ERR_FS_MAX_KNOWN] __attribute__((unused)) =
{
    [ERR_FS_NO_ERROR] = "", // No error
    [ERR_FS_UNKNOWN_FILE_TYPE] = "Unknown similarity score type",
    [ERR_FS_NETWORK_SIZE_FILE_SIZE_MISMATCH] = "Network bytes count does not match similarity score file size",
    [ERR_FS_FAILED_TO_OPEN_FILE_FOR_WRITE] = "Failed to open score file for writing",
    [ERR_FS_FILE_WRITE_NOT_COMPLETED] = "Failed to write all bytes to file",
    [ERR_FS_VALUE_NOT_INITIALIZED] = "Value has not been initialized",
    [ERR_FS_PACKET_SIZE_TOO_LARGE] = "Packet size is greater than max allowed",
    [ERR_FS_SOCKET_READ_NOT_ENOUGH_BYTES] = "Failed to read full packet from network",
    [ERR_FS_SOCKET_WRITE_INTERRUPTED] = "Failed to write full packet to network",
    [ERR_FS_NON_HANDLED_PACKET_TYPE] = "No handler attached to packet type",
    [ERR_FS_UNKNOWN_PACKET_TYPE] = "Unknown packet type",
    [ERR_FS_MEMORY_ALLOC_FAILED] = "Failed to allocate memory",
    [ERR_FS_ID_ARRAY_TOO_LARGE] = "Similarity ID array size too large",
    [ERR_FS_FAILED_TO_OPEN_FILE_FOR_READ] = "Failed to open score file for reading",
    [ERR_FS_FAILED_TO_MAP_FILE_FOR_READ] = "Failed to map score file for reading",
    [ERR_FS_FAILED_TO_UNMAP_FILE] = "Failed to unmap score file",
    [ERR_FS_NO_INPUT_SIMILARITY_FILES] = "Found no similarity files to process",
    [ERR_FS_FAILED_TO_PARSE_GET_REQUEST_DATA] = "Failed to parse get request data block",
    [ERR_FS_MALFORMED_END_CONNECTION_PACKET] = "Malformed end connection packet",
    [ERR_FS_MALFORMED_PING_PACKET] = "Malformed ping packet",
    [ERR_FS_MALFORMED_GET_FILE_PACKET] = "Malformed get file packet",
    [ERR_FS_FAILED_TO_INITIALIZE_RW_LOCK] = "Failed to initialize RW lock",
    [ERR_FS_FAILED_TO_UNITIALIZE_RW_LOCK] = "Failed to unitialize RW lock",
    [ERR_FS_FAILED_TO_READ_LOCK] = "Failed to aquire read lock",
    [ERR_FS_FAILED_TO_WRITE_LOCK] = "Failed to aquire write lock",
    [ERR_FS_FAILED_TO_UNLOCK_READ_LOCK] = "Failed to release read lock",
    [ERR_FS_FAILED_TO_UNLOCK_WRITE_LOCK] = "Failed to release write lock",
    [ERR_FS_ENTRY_NOT_FOUND] = "Entry not found",
    [ERR_FS_ENTRY_SAVE_FAILED] = "Entry save failed",
    [ERR_FS_DAO_INIT_FAILED] = "DAO init failed",
    [ERR_FS_MALFORMED_SAVE_FILE_PACKET] = "Malformed save file packet",
    [ERR_FS_SOCKET_TIMEOUT] = "Socket timed out", 
    [ERR_FS_SOCKET_SELECT_FAILED] = "Socket select failed",
    [ERR_FS_SOCKET_READ_INTERRUPTED] = "Socket read interrupted",
    [ERR_FS_SOCKET_WRITE_FAILED] = "Socket write failed",
    [ERR_FS_FILE_READ_FAILED] = "Failed to read enough bytes from file",
    [ERR_FS_PARAM_DAO_NULL] = "Invalid parameter value",
    [ERR_FS_FILE_RENAME_FAILED] = "Failed to rename file",
    [ERR_FS_PARAM_SERVERDATA_NULL] = "Invalid parameter value",
    [ERR_FS_SOCKET_LISTEN_FAILED] = "Failed to listen socket",
    [ERR_FS_SOCKET_ACCEPT_FAILED] = "Failed to accept connection",
    [ERR_FS_EVENT_CREATE_FAILED] = "Failed to create event",
    [ERR_FS_SOCKET_INIT_FAILED] = "Failed to initalize socket",
    [ERR_FS_THREAD_CREATE_FAILED] = "Failed to create thread",
    [ERR_FS_MT_QUEUE_INIT_FAILED] = "Failed to create MTQUEUE",
    [ERR_FS_SOCKET_CREATE_FAILED] = "Failed to create socket",
    [ERR_FS_SOCKET_BIND_FAILED] = "Failed to bind socket",
    [ERR_FS_SOCKET_CONFIG_FAILED] = "Failed to configure socket",
    [ERR_FS_SOCKET_CONFIG_WTIMEOUT_FAILED] = "Failed to set socket write timeout",
    [ERR_FS_SOCKET_CONFIG_RETRY_FAILED] = "Failed to set socket read timeout",
    [ERR_FS_SOCKET_CONFIG_TIMEOUT_FAILED] = "Failed to set socket timeout",
    [ERR_FS_SOCKET_CONFIG_REUSEADDR_FAILED] = "Failed to set socket port reuse",
    [ERR_FS_SOCKET_CONFIG_NAGGLE] = "Failed to set socket Naggle",
    [ERR_FS_SOCKET_CONFIG_QUICK_ACK] = "Failed to set socket quick Ack",
};

#define LOG_DEFAULT_APP_ERROR(err) LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: %d = %s", \
    err, g_err_code_str[err]);
#endif