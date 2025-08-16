#ifndef _PLUGIN_JA3_H_
#define _PLUGIN_JA3_H_

/*
* flow identifier plugin based on TLS JA3 hash
* mode 1 : Parse a pcap file and insert found JA3 hashes into the DB
* mode 2 : load known JA3 that belong to an app. Tag live network flows based on known JA3
* 
* JA3s is derived from : TLS Version, Selected Cipher Suite, Selected Extensions
*/
typedef struct nDPI_pkt_parser_params nDPI_pkt_parser_params;

void init_packetp_JA3();
void destroy_packetp_JA3();
size_t process_packet_JA3(nDPI_pkt_parser_params* pktparams);

#endif // _PLUGIN_JA3_H_
