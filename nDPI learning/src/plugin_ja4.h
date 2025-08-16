#ifndef _PLUGIN_JA4_H_
#define _PLUGIN_JA4_H_

/*
* flow identifier plugin based on TLS JA4 hash
* mode 1 : Parse a pcap file and insert found JA4 hashes into the DB
* mode 2 : load known JA4 that belong to an app. Tag live network flows based on known JA4
* 
* JA4 is derived from : TLS version, Cipher suites, ALPN protocols, TLS extensions, Elliptic curves, EC point formats, Session ID, SNI presence, Compression methods
*/
typedef struct nDPI_pkt_parser_params nDPI_pkt_parser_params;

void init_packetp_JA4();
void destroy_packetp_JA4();
size_t process_packet_JA4(nDPI_pkt_parser_params* pktparams);

#endif // _PLUGIN_JA4_H_
