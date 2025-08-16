#ifndef _PLUGIN_SHA1_H_
#define _PLUGIN_SHA1_H_

/*
* flow identifier plugin based on TLS SHA1
* mode 1 : Parse a pcap file and insert found SHA1 into the DB
* mode 2 : load known SHA1 that belong to an app. Tag live network flows based on known SHA1
* 
* SHA1 certificate is derived from hashing the server sent certificate
*/
typedef struct nDPI_pkt_parser_params nDPI_pkt_parser_params;

void init_packetp_SHA1();
void destroy_packetp_SHA1();
size_t process_packet_SHA1(nDPI_pkt_parser_params* pktparams);

#endif // _PLUGIN_SHA1_H_
