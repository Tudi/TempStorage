#ifndef _PLUGIN_SERVER_IP_H_
#define _PLUGIN_SERVER_IP_H_

/*
* flow identifier plugin based on SERVER_IP
* mode 1 : Parse a pcap file and insert found SERVER_IP into the DB
* mode 2 : load known SERVER_IP that belongs to an app. Tag live network flows based on known SERVER_IP
*
*/
typedef struct nDPI_pkt_parser_params nDPI_pkt_parser_params;

void init_packetp_SERVER_IP();
void destroy_packetp_SERVER_IP();
size_t process_packet_SERVER_IP(nDPI_pkt_parser_params* pktparams);

#endif // _PLUGIN_SERVER_IP_H_
