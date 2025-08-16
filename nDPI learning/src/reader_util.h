/*
 * ndpi_util.h
 *
 * Copyright (C) 2011-25 - ntop.org
 *
 * nDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nDPI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * This module contains routines to help setup a simple nDPI program.
 *
 * If you concern about performance or have to integrate nDPI in your
 * application, you could need to reimplement them yourself.
 *
 * WARNING: this API is just a demo od nDPI usage: Use it at your own risk!
 */
#ifndef __NDPI_UTIL_H__
#define __NDPI_UTIL_H__

#include "third_party/include/uthash.h"
#include <pcap.h>
#include "ndpi_includes.h"
#include "ndpi_classify.h"
#include "ndpi_typedefs.h"

#ifdef USE_DPDK
#include <rte_eal.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

#define RX_RING_SIZE     128
#define TX_RING_SIZE     512
#define NUM_MBUFS       8191
#define MBUF_CACHE_SIZE  250
#define BURST_SIZE        32
#define PREFETCH_OFFSET    3

extern int dpdk_port_init(int port, struct rte_mempool *mbuf_pool);
extern int dpdk_port_deinit(int port);
#endif

#define PLEN_MAX         1504
#define PLEN_BIN_LEN     32
#define PLEN_NUM_BINS    48 /* 47*32 = 1504 */
#define MAX_NUM_BIN_PKTS 256

/* ETTA Spec defiintions for feature readiness */
#define ETTA_MIN_PACKETS 10
#define ETTA_MIN_OCTETS 4000
/** maximum line length */
#define LINEMAX 512
#define MAX_BYTE_COUNT_ARRAY_LENGTH 256
#define MAX_NUM_PKTS               10

#define MAX_NUM_READER_THREADS     1
#define MAX_NUM_WORKSPLIT_THREADS  1
#define MAX_NUM_WORKER_THREADS     64
#define IDLE_SCAN_PERIOD           10 /* msec (use TICK_RESOLUTION = 1000) */
#define MAX_IDLE_TIME           30000
#define IDLE_SCAN_BUDGET         1024
#define NUM_ROOTS                 512
#define MAX_EXTRA_PACKETS_TO_CHECK  7
#define MAX_NDPI_FLOWS      200000000
#define TICK_RESOLUTION          1000
#define MAX_NUM_IP_ADDRESS          5  /* len of ip address array */
#define UPDATED_TREE                1
#define AGGRESSIVE_PERCENT      95.00
#define DIR_SRC                    10
#define DIR_DST                    20
#define PORT_ARRAY_SIZE            20
#define HOST_ARRAY_SIZE            20
#define FLOWS_PACKETS_THRESHOLD   0.9
#define FLOWS_PERCENT_THRESHOLD   1.0
#define FLOWS_PERCENT_THRESHOLD_2 0.2
#define FLOWS_THRESHOLD          1000
#define PKTS_PERCENT_THRESHOLD    0.1
#define MAX_TABLE_SIZE_1         4096
#define MAX_TABLE_SIZE_2         8192
#define INIT_VAL                   -1
#define SERIALIZATION_BUFSIZ     (8192 * 2)


#ifdef __cplusplus
extern "C" {
#endif

// inner hash table (ja -> security state)
typedef struct ndpi_ja_info {
  char * ja;
  ndpi_cipher_weakness unsafe_cipher;
  UT_hash_handle hh;
} ndpi_ja_info;

// external hash table (host ip -> <ip string, hash table ja4c, hash table ja3s>)
// used to aggregate ja3 fingerprints by hosts
typedef struct ndpi_host_ja_fingerprints {
  u_int32_t ip;
  char *ip_string;
  char *dns_name;
  ndpi_ja_info *host_client_info_hasht;
  ndpi_ja_info *host_server_info_hasht;

  UT_hash_handle hh;
} ndpi_host_ja_fingerprints;


//inner hash table
typedef struct ndpi_ip_dns{
  u_int32_t ip;
  char *ip_string;
  char *dns_name; //server name if any;
  UT_hash_handle hh;
} ndpi_ip_dns;

//hash table ja -> <host, ip, security>, used to aggregate host by ja fingerprints
typedef struct ndpi_ja_fingerprints_host{
  char *ja; //key
  ndpi_cipher_weakness unsafe_cipher;
  ndpi_ip_dns *ipToDNS_ht;
  UT_hash_handle hh;
} ndpi_ja_fingerprints_host;

struct flow_metrics {
  float entropy, average, stddev;
};

struct ndpi_entropy {
  // Entropy fields
  u_int16_t src2dst_pkt_len[MAX_NUM_PKTS];                     /*!< array of packet appdata lengths */
  pkt_timeval src2dst_pkt_time[MAX_NUM_PKTS];               /*!< array of arrival times          */
  u_int16_t dst2src_pkt_len[MAX_NUM_PKTS];                     /*!< array of packet appdata lengths */
  pkt_timeval dst2src_pkt_time[MAX_NUM_PKTS];               /*!< array of arrival times          */
  pkt_timeval src2dst_start;                                /*!< first packet arrival time       */
  pkt_timeval dst2src_start;                                /*!< first packet arrival time       */
  u_int32_t src2dst_opackets;                                  /*!< non-zero packet counts          */
  u_int32_t dst2src_opackets;                                  /*!< non-zero packet counts          */
  u_int16_t src2dst_pkt_count;                                 /*!< packet counts                   */
  u_int16_t dst2src_pkt_count;                                 /*!< packet counts                   */
  u_int32_t src2dst_l4_bytes;                                  /*!< packet counts                   */
  u_int32_t dst2src_l4_bytes;                                  /*!< packet counts                   */
  u_int32_t src2dst_byte_count[MAX_BYTE_COUNT_ARRAY_LENGTH];   /*!< number of occurences of each byte   */
  u_int32_t dst2src_byte_count[MAX_BYTE_COUNT_ARRAY_LENGTH];   /*!< number of occurences of each byte   */
  u_int32_t src2dst_num_bytes;
  u_int32_t dst2src_num_bytes;
  double src2dst_bd_mean;
  double src2dst_bd_variance;
  double dst2src_bd_mean;
  double dst2src_bd_variance;
  float score;
};

enum info_type {
    INFO_INVALID = 0,
    INFO_GENERIC,
    INFO_KERBEROS,
    INFO_SOFTETHER,
    INFO_TIVOCONNECT,
    INFO_FTP_IMAP_POP_SMTP,
    INFO_NATPMP,
    INFO_SIP,
    INFO_FASTCGI,
};

typedef struct {
  ndpi_address_port *aps;
  unsigned int num_aps;
  unsigned int num_aps_allocated;
} ndpi_address_port_list;

// uthash works on 12 byte chunks. Try to be compliant as much as possible
#pragma pack(push, 1)
typedef struct ndpi_flow_info_hash_key {
    union {
        struct ndpi_in6_addr src_ip_v6;
        u_int32_t src_ip_v4;
    }src_addr; // 16 bytes
    union {
        struct ndpi_in6_addr dst_ip_v6;
        u_int32_t dst_ip_v4;
    }dst_addr; // 16 bytes
    u_int16_t src_port; // layer 4 info
    u_int16_t dst_port; // layer 4 info
    u_int16_t vlan_id; // this is layer 2 info
    u_int8_t protocol_layer3; // this is ex TCP, UDP .. Application layer protocol will be at layer 7
}ndpi_flow_info_hash_key;
#pragma pack(pop)
_Static_assert(sizeof(struct ndpi_flow_info_hash_key) == 39, "ndpi_flow_info_hash_key was exepected to have 48 bytes");

// flow tracking
typedef struct ndpi_flow_info {
  // all these values are used for hash key
  ////////////////////////////////////////
  UT_hash_handle hh;
  u_int32_t hashval; // this is precomputed hash value based on our full key
  ndpi_flow_info_hash_key hash_key_full_val; // used uppon hash key collision
  ////////////////////////////////////////

#ifdef USE_OLD_TSEARCH_FLOW_STORE
  // old tsearch keys
  ////////////////////////////////////////
  u_int32_t src_ip_v4; /* network order */
  struct ndpi_in6_addr src_ip_v6; /* network order */
  u_int32_t dst_ip_v4; /* network order */
  struct ndpi_in6_addr dst_ip_v6; /* network order */
  u_int16_t src_port; /* network order */
  u_int16_t dst_port; /* network order */
  u_int8_t protocol; 
  u_int16_t vlan_id;
  ////////////////////////////////////////
#endif

  // disectors store values here. we copy values from here into this struct
  struct ndpi_flow_struct* ndpi_flow;

  u_int64_t last_seen_ms; // required to detect idle flows

  // custom data added by app_disector
  u_int16_t app_is_identified : 1; // should have a confidence factor to identify and app
  u_int16_t app_is_identified_unsure : 1; // these are below the treshold flows. Could be anything like CDNs
  u_int16_t app_reported : 1; // once we identified an app, we want to report to it something like ntopn
  u_int16_t checked_plugin_JA3 : 1;
  u_int16_t checked_plugin_JA4 : 1;
  u_int16_t checked_plugin_SHA1 : 1;
  u_int16_t checked_plugin_S_IP : 1;
  u_int16_t checked_plugin_SNI : 1;
  u_int16_t sha1_cert_fingerprint_set : 1;
  u_int16_t payload_bin_destroyed : 1;
  u_int16_t unsused_marked_for_idle_deletion : 1;
  u_int16_t unused_flags : 5;
  // might be used later to remove malloc
  struct BufferPool* buff_pool; // alloc memory from here. Will be wiped on flow expire. No leaks
  struct fsa_plugin_results* detection_results; // plugin results will be stored here
  struct ndpi_workflow* workflow; // worker thread storage

  u_int8_t detection_completed;
  u_int8_t ip_version; // 4 or 6 
  u_int32_t src2dst_packets, dst2src_packets;

  // result only, not used for flow identification
  ndpi_protocol detected_protocol;
  u_int32_t flow_id; // mostly used to make logs pretty

  char host_server_name[80]; /* Hostname/SNI */

  struct {
      char ja3_server[33], ja4_client[37], sha1_cert_fingerprint[20];
      time_t notBefore, notAfter;
#ifdef TLS_NOT_USED_FIELDS
    char server_info[64],
        client_hassh[33], server_hassh[33], * server_names,
        * advertised_alpns, * negotiated_alpn, * tls_supported_versions,
        * tls_issuerDN, * tls_subjectDN, * ja4_client_raw;

    struct {
        u_int16_t version;
  } encrypted_ch;
    u_int16_t ssl_version;
    u_int16_t server_cipher;
    ndpi_cipher_weakness client_unsafe_cipher, server_unsafe_cipher;
    u_int32_t quic_version;
#endif
#ifdef TLS_BROWSER_HEURISTICS
    struct tls_heuristics browser_heuristics;
#endif
  } ssh_tls;

#ifdef STORE_HTTP_INFO
  struct {
    char url[256], request_content_type[64], content_type[64],
      user_agent[256], server[128], nat_ip[32], username[64], password[64], filename[256];
    u_int response_status_code;
  } http;
#endif

#ifdef COPY_FLOW_INFO_TO_FLOW
  struct rtp_info rtp[2 /* directions */];
#endif

#ifdef STORE_STUN_INFO
  struct {
    ndpi_address_port_list mapped_address, peer_address,
      relayed_address, response_origin, other_address;
    u_int16_t rtp_counters[2];
  } stun;
#endif

#ifdef COPY_FLOW_INFO_TO_FLOW // might want to restore this later
  struct {
    char geolocation_iata_code[4];
    char ptr_domain_name[64];
    u_int16_t transaction_id;
  } dns;
#endif

#ifdef STORE_CONFIDENCE_SCORE
  ndpi_confidence_t confidence;
  struct ndpi_fpc_info fpc;
#endif

#ifdef CALCULATE_ENTROPY_SCORE
  struct ndpi_entropy *entropy;
  struct ndpi_entropy *last_entropy;
#endif

#ifdef NDI_USE_BINS // DOH_DOT is (DNS over HTTPS + DNS over TLS). Would be great to have these working
  /* Payload lenght bins */
#ifdef DIRECTION_BINS
  struct ndpi_bin payload_len_bin_src2dst, payload_len_bin_dst2src;
#else
  struct ndpi_bin payload_len_bin;
#endif
#endif

#ifdef COPY_FLOW_INFO_TO_FLOW
  enum info_type info_type;
  union {
      char info[256];

      struct {
          unsigned char auth_failed;
          char username[127];
          char password[128];
      } ftp_imap_pop_smtp;

      struct {
          char domain[85];
          char hostname[85];
          char username[86];
      } kerberos;

      struct {
          char ip[16];
          char port[6];
          char hostname[48];
          char fqdn[48];
      } softether;

      struct {
          char identity_uuid[36];
          char machine[48];
          char platform[32];
          char services[48];
      } tivoconnect;

      struct {
          uint16_t result_code;
          uint16_t internal_port;
          uint16_t external_port;
          char ip[16];
      } natpmp;

      struct {
          char from[256];
          char from_imsi[16];
          char to[256];
          char to_imsi[16];
      } sip;

      struct {
          ndpi_http_method method;
          char user_agent[32];
          char url[64];
      } fast_cgi;
};
#endif

#ifdef ENABLE_FLOW_DATA_ANALYSIS
  // Flow data analysis
  pkt_timeval src2dst_last_pkt_time, dst2src_last_pkt_time, flow_last_pkt_time;
  struct ndpi_analyze_struct* iat_c_to_s, * iat_s_to_c, * iat_flow,
      * pktlen_c_to_s, * pktlen_s_to_c;
#endif

#ifdef _FLOW_SERIALIZER
  ndpi_serializer ndpi_flow_serializer;
  uint8_t ndpi_flow_serializer_destroyed;
#endif

#ifdef TCP_FLAG_STATISTICS
  u_int32_t cwr_count, src2dst_cwr_count, dst2src_cwr_count;
  u_int32_t ece_count, src2dst_ece_count, dst2src_ece_count;
  u_int32_t urg_count, src2dst_urg_count, dst2src_urg_count;
  u_int32_t ack_count, src2dst_ack_count, dst2src_ack_count;
  u_int32_t psh_count, src2dst_psh_count, dst2src_psh_count;
  u_int32_t syn_count, src2dst_syn_count, dst2src_syn_count;
  u_int32_t fin_count, src2dst_fin_count, dst2src_fin_count;
  u_int32_t rst_count, src2dst_rst_count, dst2src_rst_count;
  u_int32_t c_to_s_init_win, s_to_c_init_win;
  u_int64_t first_seen_ms;
  u_int64_t src2dst_bytes, dst2src_bytes;
  u_int64_t src2dst_goodput_bytes, dst2src_goodput_bytes;
#endif

#ifdef ENABLE_HUMAN_READABLE_STRING_EXTRACTION
  u_int32_t has_human_readeable_strings;
  char human_readeable_string_buffer[32];
#endif

#ifdef ENABLE_ALL_NDPI_FEATURES
  struct {
      char currency[16];
  } mining;
  char* bittorent_hash;
  char* risk_str;
  ndpi_risk risk;
  uint32_t idle_timeout_sec;
  u_int16_t num_packets_before_monitoring;
  u_int8_t monitoring_state;
  char src_name[INET6_ADDRSTRLEN], dst_name[INET6_ADDRSTRLEN];
  u_int8_t multimedia_flow_types;

  struct {
      char* username, * password;
  } telnet;
  u_int8_t current_pkt_from_client_to_server, check_extra_packets;
  ndpi_packet_tunnel tunnel_type;
  u_int16_t num_dissector_calls;
  u_int16_t dpi_packets;
  u_int8_t bidirectional;
  char* server_hostname; // this should be based on some chache that needs to be enabled
  char* dhcp_fingerprint;
  char* dhcp_class_ident;
  char* tcp_fingerprint;
#endif

#ifdef MAINTAIN_FLOW_PAYLOAD_COPY
  /* Flow payload */
  u_int16_t flow_payload_len;
  char *flow_payload;  
#endif
} ndpi_flow_info;


// flow statistics info
typedef struct ndpi_stats {
  u_int32_t guessed_flow_protocols;
  u_int64_t raw_packet_count;
  u_int64_t ip_packet_count;
  u_int64_t total_wire_bytes, total_ip_bytes, total_discarded_bytes;
  u_int64_t protocol_counter[NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS + 1];
  u_int64_t protocol_counter_bytes[NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS + 1];
  u_int32_t protocol_flows[NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS + 1];
  u_int64_t fpc_protocol_counter[NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS + 1];
  u_int64_t fpc_protocol_counter_bytes[NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS + 1];
  u_int32_t fpc_protocol_flows[NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS + 1];
  u_int32_t ndpi_flow_count;
  u_int32_t flow_count[3];
  u_int64_t tcp_count, udp_count;
  u_int64_t mpls_count, pppoe_count, vlan_count, fragmented_count;
  u_int64_t packet_len[6];
  u_int16_t max_packet_len;
  u_int64_t dpi_packet_count[3];
  u_int64_t flow_confidence[NDPI_CONFIDENCE_MAX];
  u_int64_t fpc_flow_confidence[NDPI_FPC_CONFIDENCE_MAX];
  u_int64_t num_dissector_calls;

  struct ndpi_lru_cache_stats lru_stats[NDPI_LRUCACHE_MAX];
  struct ndpi_automa_stats automa_stats[NDPI_AUTOMA_MAX];
  struct ndpi_patricia_tree_stats patricia_stats[NDPI_PTREE_MAX];
} ndpi_stats_t;


// flow preferences
typedef struct ndpi_workflow_prefs {
  u_int8_t decode_tunnels;
  u_int8_t quiet_mode;
  u_int8_t ignore_vlanid;
  u_int32_t num_roots;
  u_int32_t max_ndpi_flows;
} ndpi_workflow_prefs_t;

struct ndpi_workflow;

/** workflow, flow, user data */
typedef void (*ndpi_workflow_callback_ptr) (struct ndpi_workflow *, struct ndpi_flow_info *, void *);


// workflow main structure. Each worker thread will have it's own workflow struct
typedef struct ndpi_workflow {
  ndpi_flow_info *all_flows; // head node for all flows available for this worker thread
  u_int64_t last_time;
  struct PoolManager* buff_pool_mgr; // used for handing out pools to flows on which the flow can operate
  struct StructPool* flow_info_pool;
  struct ndpi_workflow_prefs prefs;
#ifdef ENABLE_FLOW_STATS_GENERATION
  struct ndpi_stats stats;
#endif
  uint32_t ndpi_flow_count;

#ifdef ENABLE_FLOW_DISCOVER_CALLBACK
  ndpi_workflow_callback_ptr flow_callback;
  void* flow_callback_userdata;
#endif

  /* outside referencies */
  pcap_t *pcap_handle;

  /* allocated by prefs */
#ifdef USE_OLD_TSEARCH_FLOW_STORE
  void **ndpi_flows_root;
#endif
  struct ndpi_detection_module_struct *ndpi_struct;
  struct ndpi_global_context *g_ctx;
#ifndef _RELEASE_BUILD
  u_int32_t num_allocated_flows;
#endif
} ndpi_workflow_t;


/* TODO: remove wrappers parameters and use ndpi global, when their initialization will be fixed... */
struct ndpi_workflow * ndpi_workflow_init(const struct ndpi_workflow_prefs * prefs, pcap_t * pcap_handle, int do_init_flows_root, ndpi_serialization_format serialization_format, struct ndpi_global_context *g_ctx);


/* workflow main free function */
void ndpi_workflow_free(struct ndpi_workflow * workflow);

/* Process a packet and update the workflow  */
void ndpi_workflow_process_packet(struct ndpi_workflow * workflow,
					       const struct pcap_pkthdr *header,
					       const u_char *packet,
#ifdef ENABLE_PCAP_DUMPER
					       ndpi_risk *flow_risk,
#endif
					       struct ndpi_flow_info **flow);


/* Flow callback for completed flows, before the flow memory will be freed. */
static inline void ndpi_workflow_set_flow_callback(struct ndpi_workflow * workflow, ndpi_workflow_callback_ptr callback, void * userdata) {
#ifdef ENABLE_FLOW_DISCOVER_CALLBACK
    workflow->flow_callback = callback;
  workflow->flow_callback_userdata = userdata;
#else
    (void)workflow;
    (void)callback;
    (void)userdata;
#endif
}

int ndpi_is_datalink_supported(int datalink_type);

/* compare two nodes in workflow */
int ndpi_workflow_node_cmp(const void *a, const void *b);
void process_ndpi_collected_info(struct ndpi_workflow * workflow, struct ndpi_flow_info *flow);
void ndpi_flow_info_freer(void *node);
int parse_proto_name_list(char *str, NDPI_PROTOCOL_BITMASK *bitmask, int inverted_logic);

extern int reader_log_level;

// get the direction agnostic number that can be used to find the same flow
int get_worker_thread_index_full(int pcap_datalink, struct pcap_pkthdr const* const header, uint8_t const* const packet);
#if defined(NDPI_ENABLE_DEBUG_MESSAGES) && !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
#define LOG(log_level, args...)			\
  {						\
    if(log_level <= reader_log_level)		\
      printf(args);				\
  }
#else
#define LOG(...) {}
#endif

#ifndef LINKTYPE_LINUX_SLL2
#define LINKTYPE_LINUX_SLL2 276
#endif

#ifdef __cplusplus
}
#endif

#endif
