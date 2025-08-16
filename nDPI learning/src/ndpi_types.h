#ifndef _NDPI_TYPES_
#define _NDPI_TYPES_

typedef enum nDPI_pkt_parser_ret_type {
    RETC_NO_ERR = 0,
    RETC_BREAK_EXECUTION,
}nDPI_pkt_parser_ret_type;

typedef struct nDPI_pkt_parser_params {
//    const void* l4_ptr;
//    size_t l4_len;
    struct ndpi_flow_info* flow_to_process;
//    struct nDPI_reader_thread* reader_thread;
}nDPI_pkt_parser_params;

#endif