#ifndef _ndpiReader_H_
#define _ndpiReader_H_

// probably will remove stats altogether. This is the first step
//#define ENABLE_FLOW_STATS_GENERATION
//#define CHECK_PACKET_CONTENT_INTEGRITY_AT_PROCESS
//#define ENABLE_PCAP_INTERVAL_ANALISYS
//#define ENABLE_PCAP_DUMPER
//#define ENABLE_ADDR_DUMP
//#define ENABLE_PAYLOAD_ANALYZER
//#define ENABLE_HUMAN_READABLE_STRING_EXTRACTION

#define ntohl64(x) ( ( (uint64_t)(ntohl( (uint32_t)((x << 32) >> 32) )) << 32) | ntohl( ((uint32_t)(x >> 32)) ) )
#define htonl64(x) ntohl64(x)

//#define HEURISTICS_CODE 1
#define MAX_FARGS 64

struct cfg {
	char* proto;
	char* param;
	char* value;
};
#define MAX_NUM_CFGS 32

struct flow_info {
	struct ndpi_flow_info* flow;
	u_int16_t thread_id;
};

struct info_pair {
	u_int32_t addr;
	u_int8_t version; /* IP version */
	char proto[16]; /*app level protocol*/
	int count;
};

typedef struct node_a {
	u_int32_t addr;
	u_int8_t version; /* IP version */
	char proto[16]; /*app level protocol*/
	int count;
	struct node_a* left, * right;
}addr_node;

// struct to add more statitcs in function printFlowStats
typedef struct hash_stats {
	char* domain_name;
	int occurency;       /* how many time domain name occury in the flow */
	UT_hash_handle hh;   /* hashtable to collect the stats */
}hash_stats;


struct port_stats {
	u_int32_t port; /* we'll use this field as the key */
	u_int32_t num_pkts, num_bytes;
	u_int32_t num_flows;
	u_int32_t num_addr; /*number of distinct IP addresses */
	u_int32_t cumulative_addr; /*cumulative some of IP addresses */
	addr_node* addr_tree; /* tree of distinct IP addresses */
	struct info_pair top_ip_addrs[MAX_NUM_IP_ADDRESS];
	u_int8_t hasTopHost; /* as boolean flag */
	u_int32_t top_host;  /* host that is contributed to > 95% of traffic */
	u_int8_t version;    /* top host's ip version */
	char proto[16];      /* application level protocol of top host */
	UT_hash_handle hh;   /* makes this structure hashable */
};

// struct to hold count of flows received by destination ports
struct port_flow_info {
	u_int32_t port; /* key */
	u_int32_t num_flows;
	UT_hash_handle hh;
};

// struct to hold single packet tcp flows sent by source ip address
struct single_flow_info {
	u_int32_t saddr; /* key */
	u_int8_t version; /* IP version */
	struct port_flow_info* ports;
	u_int32_t tot_flows;
	UT_hash_handle hh;
};

// struct to hold top receiver hosts
struct receiver {
	u_int32_t addr; /* key */
	u_int8_t version; /* IP version */
	u_int32_t num_pkts;
	UT_hash_handle hh;
};

#define WIRESHARK_NTOP_MAGIC 0x19680924
#define WIRESHARK_METADATA_SIZE		192
#define WIRESHARK_FLOW_RISK_INFO_SIZE	128

#define WIRESHARK_METADATA_SERVERNAME	0x01
#define WIRESHARK_METADATA_JA4C		0x02

#define LIBPCAP_SNAPLEN		1536		// RTE_MBUF_DEFAULT_BUF_SIZE if using dpdk ?

struct ndpi_packet_tlv {
	u_int16_t type;
	u_int16_t length;
	unsigned char data[];
};

PACK_ON
struct ndpi_packet_trailer {
	u_int32_t magic; /* WIRESHARK_NTOP_MAGIC */
	ndpi_master_app_protocol proto;
	char name[16];
	u_int8_t flags;
	ndpi_risk flow_risk;
	u_int16_t flow_score;
	u_int16_t flow_risk_info_len;
	char flow_risk_info[WIRESHARK_FLOW_RISK_INFO_SIZE];
	/* TLV of attributes. Having a max and fixed size for all the metadata
	   is not efficient but greatly improves detection of the trailer by Wireshark */
	u_int16_t metadata_len;
	unsigned char metadata[WIRESHARK_METADATA_SIZE];
} PACK_OFF;

// only used when using reader / worker thread architecture
typedef struct packet_buffer_store {
	struct pcap_pkthdr header;
	uint8_t data[LIBPCAP_SNAPLEN];
	size_t length;
	struct packet_buffer_store *prev;
	size_t pcap_datalink;
	uint8_t reader_thread_id; // we borrow packet buffers from reader threads and push it back once we are done with it
	uint8_t processing_stage; // 0 = figure out flow_key, 1 = reassign to proper worker thread, 2 = process flow
}packet_buffer_store;

// struct associated to a workflow for a thread
typedef struct nDPI_reader_thread {
	struct ndpi_workflow* workflow;
	pthread_t pthread;
	u_int64_t last_idle_scan_time;
#ifdef USE_OLD_TSEARCH_FLOW_STORE
	u_int32_t idle_scan_idx;
	u_int32_t num_idle_flows;
	struct ndpi_flow_info* idle_flows[IDLE_SCAN_BUDGET];
#endif
	size_t assign_work_to_thread_idx; // circular assignement packet to next worker thread
	pthread_mutex_t packet_handover_lock;	// used when reader thread passes it to the worker thread
	pthread_cond_t packet_available_signal;	// tell a worker thread to fetch and process this packet
	struct StructPool* packet_buffer_pool;		// because libpcap snaplen is given by us. DPDK might be a different cake
	packet_buffer_store* packet_queue_head, *packet_queue_tail;		// packets will be queued here
}nDPI_reader_thread;

// ID tracking
typedef struct ndpi_id {
	u_int8_t ip[4];                   // Ip address
	struct ndpi_id_struct* ndpi_id;  // nDpi worker structure
} ndpi_id_t;

extern struct ndpi_bin malloc_bins;
extern u_int32_t current_ndpi_memory, max_ndpi_memory;
extern int enable_malloc_bins;
extern int max_malloc_bins;
extern int malloc_size_stats;
extern u_int8_t num_reader_threads;
extern u_int8_t num_worksplit_threads; // parse packet header so it can be assigned to the appropriate worker thread
extern u_int8_t num_worker_threads; // libpcap will use single reader thread. If we want to go faster, we want to split the work to worker threads
extern char* _pcap_file[MAX_NUM_READER_THREADS];
extern struct timeval startup_time, begin, end;
extern struct nDPI_reader_thread ndpi_thread_info[MAX_NUM_READER_THREADS + MAX_NUM_WORKSPLIT_THREADS + MAX_NUM_WORKER_THREADS];
extern time_t capture_for;
extern time_t capture_until;
extern u_int16_t decode_tunnels;
extern u_int8_t shutdown_app, quiet_mode;
extern u_int8_t live_capture;
extern char* addr_dump_path;
extern int monitoring_enabled;
extern u_int8_t enable_doh_dot_detection;
extern u_int32_t max_num_packets_per_flow, max_packet_payload_dissection, max_num_reported_top_payloads;
extern u_int16_t min_pattern_len, max_pattern_len;
extern FILE* playlist_fp[MAX_NUM_READER_THREADS];
extern char* bpfFilter;
extern struct bpf_program* bpf_cfilter;
extern FILE* results_file;
#ifndef USE_DPDK
extern struct bpf_program bpf_code;
#endif
extern u_int8_t enable_realtime_output;
extern u_int8_t ignore_vlanid;
extern ndpi_serialization_format serialization_format;
extern char* _disabled_protocols;
extern char* _categoriesDirPath;
extern char* _domain_suffixes;
extern char* _riskyDomainFilePath;
extern char* _maliciousJA4Path;
extern char* _maliciousSHA1Path;
extern char* _customCategoryFilePath;
extern char* _protoFilePath;
extern struct cfg cfgs[MAX_NUM_CFGS];
extern int num_cfgs;
extern struct ndpi_stats cumulative_stats;
extern u_int8_t dump_internal_stats;
extern int dump_fpc_stats;
extern struct timeval pcap_start, pcap_end;
extern u_int8_t enable_realtime_output, enable_payload_analyzer, num_bin_clusters, extcap_exit;
extern FILE* serialization_fp;
extern FILE* csv_fp;
extern u_int8_t verbose, enable_flow_stats;
extern u_int8_t undetected_flows_deleted;
#ifdef __linux__
extern int core_affinity[MAX_NUM_READER_THREADS];
#endif
extern char* domain_to_check;
extern int do_extcap_capture;
extern u_int8_t max_num_udp_dissected_pkts, max_num_tcp_dissected_pkts;
extern char* ip_port_to_check;
extern u_int16_t extcap_packet_filter;
extern char* results_path;
extern u_int32_t pcap_analysis_duration;
extern char* extcap_capture_fifo;
extern FILE* fingerprint_fp;
extern u_int16_t num_loops;
extern char* fargv[MAX_FARGS];
extern int fargc;
extern int app_id_for_pcap;

#endif