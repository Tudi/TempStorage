#ifndef _PLUGIN_SNI_H_
#define _PLUGIN_SNI_H_

/*
* flow identifier plugin based on SNI
* mode 1 : Parse a pcap file and insert found SNI into the DB
* mode 2 : load known SNI that belong to an app. Tag live network flows based on known SNI
* 
* SNI is : host_server_name
*/
typedef struct nDPI_pkt_parser_params nDPI_pkt_parser_params;

void init_packetp_SNI();
void destroy_packetp_SNI();
size_t process_packet_SNI(nDPI_pkt_parser_params* pktparams);

#endif // _PLUGIN_SNI_H_
