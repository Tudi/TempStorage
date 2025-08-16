#define _GNU_SOURCE

#include "ndpi_config.h"

#ifdef __linux__
#include <sched.h>
#endif

#include "ndpi_api.h"
#include "third_party/include/uthash.h"
#include "third_party/include/ahocorasick.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <float.h> /* FLT_EPSILON */
#ifdef WIN32
#include <winsock2.h> /* winsock.h is included automatically */
#include <windows.h>
#include <ws2tcpip.h>
#include <process.h>
#include <io.h>
#else
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/mman.h>
#endif
#include <string.h>
#include <stdarg.h>
#include <search.h>
#include <pcap.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>  
#include <assert.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef _MSC_BUILD
#include <libgen.h>
#endif
#include <errno.h>

#include "reader_util.h"
#include "ndpiReader.h"

#include "utils.h"
#include "plugin_globals.h"
#include "log_manager.h"
#include "ini_file_handler.h"
#include "buffer_pool.h"
#include "mysql_interface.h"
#include "ndpi_types.h"
#include "plugin_ja3.h"
#include "plugin_ja4.h"
#include "plugin_SHA1.h"
#include "plugin_serv_ip.h"

/** Client parameters **/

char* _pcap_file[MAX_NUM_READER_THREADS]; /**< Ingress pcap file/interfaces */
#ifndef USE_DPDK
FILE* playlist_fp[MAX_NUM_READER_THREADS] = { NULL }; /**< Ingress playlist */
#endif
FILE* results_file = NULL;
char* results_path = NULL;
char* bpfFilter = NULL; /**< bpf filter  */
char* _protoFilePath = NULL; /**< Protocol file path */
char* _customCategoryFilePath = NULL; /**< Custom categories file path  */
char* _maliciousJA4Path = NULL; /**< Malicious JA4 signatures */
char* _maliciousSHA1Path = NULL; /**< Malicious SSL certificate SHA1 fingerprints */
char* _riskyDomainFilePath = NULL; /**< Risky domain files */
char* _domain_suffixes = NULL; /**< Domain suffixes file */
char* _categoriesDirPath = NULL; /**< Directory containing domain files */
u_int8_t live_capture = 0;
u_int8_t undetected_flows_deleted = 0;
FILE* csv_fp = NULL; /**< for CSV export */
FILE* serialization_fp = NULL; /**< for TLV,CSV,JSON export */
ndpi_serialization_format serialization_format = ndpi_serialization_format_unknown;
char* domain_to_check = NULL;
char* ip_port_to_check = NULL;
u_int8_t ignore_vlanid = 0;
FILE* fingerprint_fp = NULL; /**< for flow fingerprint export */
char* fargv[MAX_FARGS];
int fargc = 0;
int dump_fpc_stats = 0;

#ifdef CUSTOM_NDPI_PROTOCOLS
#include "../../nDPI-custom/ndpiReader_defs.c"
#endif

/** User preferences **/
char* addr_dump_path = NULL;
u_int8_t enable_realtime_output = 0, enable_payload_analyzer = 0, num_bin_clusters = 0, extcap_exit = 0;
u_int8_t verbose = 0, enable_flow_stats = 0;

struct cfg cfgs[MAX_NUM_CFGS];
int num_cfgs = 0;

int reader_log_level = 0;
char* _disabled_protocols = NULL;
u_int8_t max_num_udp_dissected_pkts = 24 /* 8 is enough for most protocols, Signal and SnapchatCall require more */, max_num_tcp_dissected_pkts = 80 /* due to telnet */;
u_int32_t pcap_analysis_duration = (u_int32_t)-1;
struct ndpi_stats cumulative_stats;
u_int16_t decode_tunnels = 0;
u_int16_t num_loops = 1;
u_int8_t shutdown_app = 0, quiet_mode = 0, num_reader_threads_running = 0;
u_int8_t num_reader_threads = 1;
// !! multi threaded packet processing has been abandoned due to project discontinuity !!
u_int8_t num_worksplit_threads = 0; // 1 ?
u_int8_t num_worker_threads = 0; // num cores - 1 ?
struct timeval startup_time, begin, end;
#ifdef __linux__
int core_affinity[MAX_NUM_READER_THREADS];
#endif
struct timeval pcap_start = { 0, 0 }, pcap_end = { 0, 0 };
#ifndef USE_DPDK
struct bpf_program bpf_code;
#endif
struct bpf_program* bpf_cfilter = NULL;
/** Detection parameters **/
time_t capture_for = 0;
time_t capture_until = 0;

u_int8_t dump_internal_stats;

struct ndpi_bin malloc_bins;
int enable_malloc_bins = 0;
int max_malloc_bins = 14;
int malloc_size_stats = 0;

int monitoring_enabled;

static pcap_dumper_t* extcap_dumper = NULL;
static pcap_t* extcap_fifo_h = NULL;
#ifdef ENABLE_PCAP_DUMPER
static char extcap_buf[65536 + sizeof(struct ndpi_packet_trailer)];
#endif
char* extcap_capture_fifo = NULL;
u_int16_t extcap_packet_filter = (u_int16_t)-1;
int do_extcap_capture = 0;
static int extcap_add_crc = 0;
int app_id_for_pcap = -1; // the app id we will use when we parse the input pcap file to generate DB content
#ifdef _APP_MODE_BUILD_DB
int g_app_table_updated = 0;
#endif
#ifndef _APP_MODE_BUILD_DB
static uint64_t flows_categorized = 0;
static uint64_t flows_uncategorized = 0;
#endif

// array for every thread created for a flow
struct nDPI_reader_thread ndpi_thread_info[MAX_NUM_READER_THREADS + MAX_NUM_WORKSPLIT_THREADS + MAX_NUM_WORKER_THREADS];

// used memory counters
u_int32_t current_ndpi_memory = 0, max_ndpi_memory = 0;
#ifdef USE_DPDK
static int dpdk_port_id = 0, dpdk_run_capture = 1;
#endif

void start_process_loops(); /* Forward */
void run_unittests();

void init_doh_bins();
void port_stats_walker(const void* node, ndpi_VISIT which, int depth, void* user_data);
void printResults(u_int64_t processing_time_usec, u_int64_t setup_time_usec);
void parseOptions(int argc, char** argv);
void ndpiCheckIPMatch(char* testChar);
void ndpiCheckHostStringMatch(char* testChar);
void dump_realtime_protocol(struct ndpi_workflow* workflow, struct ndpi_flow_info* flow);
char const* ndpi_cfg_error2string(ndpi_cfg_error const err);
// expecting to remove these 2 before release
void on_memory_allocated(int64_t size);
void print_memory_allocation_stats();
void toggle_allocation_statistics(int enable);

/* ********************************** */

// #define DEBUG_TRACE

#ifdef DEBUG_TRACE
FILE* trace = NULL;
#endif


#ifndef USE_DPDK

/**
 * @brief Get the next pcap file from a passed playlist
 */
int getNextPcapFileFromPlaylist(u_int16_t thread_id, char filename[], u_int32_t filename_len) {

    if (playlist_fp[thread_id] == NULL) {
        if ((playlist_fp[thread_id] = fopen(_pcap_file[thread_id], "r")) == NULL)
            return -1;
    }

next_line:
    if (fgets(filename, filename_len, playlist_fp[thread_id])) {
        int l = strlen(filename);
        if (filename[0] == '\0' || filename[0] == '#') goto next_line;
        if (filename[l - 1] == '\n') filename[l - 1] = '\0';
        return 0;
    }
    else {
        fclose(playlist_fp[thread_id]);
        playlist_fp[thread_id] = NULL;
        return -1;
    }
}

/**
 * @brief Configure the pcap handle
 */
void configurePcapHandle(pcap_t* pcap_handle) {
    if (!pcap_handle)
        return;

    if (bpfFilter != NULL) {
        if (!bpf_cfilter) {
            if (pcap_compile(pcap_handle, &bpf_code, bpfFilter, 1, 0xFFFFFF00) < 0) {
                printf("pcap_compile error: '%s'\n", pcap_geterr(pcap_handle));
                return;
            }
            bpf_cfilter = &bpf_code;
        }

        if (pcap_setfilter(pcap_handle, bpf_cfilter) < 0) {
            printf("pcap_setfilter error: '%s'\n", pcap_geterr(pcap_handle));
        }
        else {
            printf("Successfully set BPF filter to '%s'\n", bpfFilter);
        }
    }
}

#endif

/* ********************************** */

void extcap_capture(int datalink_type) {
#ifdef DEBUG_TRACE
    if (trace) fprintf(trace, " #### %s #### \n", __FUNCTION__);
#endif

    if ((extcap_fifo_h = pcap_open_dead(datalink_type, 16384 /* MTU */)) == NULL) {
        fprintf(stderr, "Error pcap_open_dead");

#ifdef DEBUG_TRACE
        if (trace) fprintf(trace, "Error pcap_open_dead\n");
#endif
        return;
    }

    if ((extcap_dumper = pcap_dump_open(extcap_fifo_h,
        extcap_capture_fifo)) == NULL) {
        fprintf(stderr, "Unable to open the pcap dumper on %s", extcap_capture_fifo);

#ifdef DEBUG_TRACE
        if (trace) fprintf(trace, "Unable to open the pcap dumper on %s\n",
            extcap_capture_fifo);
#endif
        return;
    }

#ifdef DEBUG_TRACE
    if (trace) fprintf(trace, "Starting packet capture [%p]\n", extcap_dumper);
#endif
}

/**
 * @brief Proto Guess Walker
 */
void node_proto_guess_walker(const void* node, ndpi_VISIT which, int depth, void* user_data) {
    struct ndpi_flow_info* flow = *(struct ndpi_flow_info**)node;
    u_int16_t thread_id = *((u_int16_t*)user_data);

    (void)depth;

    if (flow == NULL) return;

    if ((which == ndpi_preorder) || (which == ndpi_leaf)) { /* Avoid walking the same node multiple times */
        if ((!flow->detection_completed) && flow->ndpi_flow) {
            u_int8_t proto_guessed;

            malloc_size_stats = 1;
            flow->detected_protocol = ndpi_detection_giveup(ndpi_thread_info[0].workflow->ndpi_struct, flow->ndpi_flow, &proto_guessed);
            malloc_size_stats = 0;

#ifdef ENABLE_FLOW_STATS_GENERATION
            if (proto_guessed) ndpi_thread_info[thread_id].workflow->stats.guessed_flow_protocols++;
#endif
        }

        process_ndpi_collected_info(ndpi_thread_info[thread_id].workflow, flow);

#ifdef ENABLE_FLOW_STATS_GENERATION
        u_int16_t proto, fpc_proto;
        proto = flow->detected_protocol.proto.app_protocol ? flow->detected_protocol.proto.app_protocol : flow->detected_protocol.proto.master_protocol;
        proto = ndpi_map_user_proto_id_to_ndpi_id(ndpi_thread_info[thread_id].workflow->ndpi_struct, proto);

        fpc_proto = flow->fpc.proto.app_protocol ? flow->fpc.proto.app_protocol : flow->fpc.proto.master_protocol;
        fpc_proto = ndpi_map_user_proto_id_to_ndpi_id(ndpi_thread_info[thread_id].workflow->ndpi_struct, fpc_proto);

        ndpi_thread_info[thread_id].workflow->stats.protocol_counter[proto] += flow->src2dst_packets + flow->dst2src_packets;
        ndpi_thread_info[thread_id].workflow->stats.protocol_counter_bytes[proto] += flow->src2dst_bytes + flow->dst2src_bytes;
        ndpi_thread_info[thread_id].workflow->stats.protocol_flows[proto]++;
        ndpi_thread_info[thread_id].workflow->stats.flow_confidence[flow->confidence]++;
        ndpi_thread_info[thread_id].workflow->stats.num_dissector_calls += flow->num_dissector_calls;
        ndpi_thread_info[thread_id].workflow->stats.fpc_protocol_counter[fpc_proto] += flow->src2dst_packets + flow->dst2src_packets;
        ndpi_thread_info[thread_id].workflow->stats.fpc_protocol_counter_bytes[fpc_proto] += flow->src2dst_bytes + flow->dst2src_bytes;
        ndpi_thread_info[thread_id].workflow->stats.fpc_protocol_flows[fpc_proto]++;
        ndpi_thread_info[thread_id].workflow->stats.fpc_flow_confidence[flow->fpc.confidence]++;
#endif
    }
}

/* *********************************************** */

/**
 * @brief Idle Scan Walker
 */
static void node_idle_scan_walker(const void* node, ndpi_VISIT which, int depth, void* user_data) {
    struct ndpi_flow_info* flow = *(struct ndpi_flow_info**)node;
    u_int16_t thread_id = *((u_int16_t*)user_data);

#ifdef USE_OLD_TSEARCH_FLOW_STORE
    if (ndpi_thread_info[thread_id].num_idle_flows == IDLE_SCAN_BUDGET) /* TODO optimise with a budget-based walk */
        return;
#endif

    if ((which == ndpi_preorder) || (which == ndpi_leaf)) { /* Avoid walking the same node multiple times */
        if (flow->last_seen_ms + MAX_IDLE_TIME < ndpi_thread_info[thread_id].workflow->last_time) {

            /* update stats */
            node_proto_guess_walker(node, which, depth, user_data);
            if (verbose == 3)
                port_stats_walker(node, which, depth, user_data);

            if ((flow->detected_protocol.proto.app_protocol == NDPI_PROTOCOL_UNKNOWN) && !undetected_flows_deleted)
                undetected_flows_deleted = 1;

            if (flow->detected_protocol.proto.app_protocol == NDPI_PROTOCOL_UNKNOWN) {
                AddLogEntryB(LDF_LOCAL, LogSeverityDebug, LogSourceWorkerThread, "Failed to detect protocol for flow %ld\n",
                    flow->flow_id);
            }

            // should have processed this sooner. We might get flows that failed to detect proto fast enough
            if (flow->app_is_identified == 0) {
                nDPI_pkt_parser_params plugin_params;
                plugin_params.flow_to_process = flow;
                check_flow_app_assoc(&plugin_params);
            }
//            print_flow_info_debug(flow);

#ifndef _APP_MODE_BUILD_DB
            if (flow->app_is_identified == 0 && flow->app_is_identified_unsure == 0) {
                flows_uncategorized++;
                const char* protoname = ndpi_protocol_id_to_str(flow->detected_protocol.proto.app_protocol);
                AddLogEntryB(LDF_LOCAL, LogSeverityDebug, LogSourceWorkerThread, "Failed to detect app_id for flow %ld, proto %s(%d). Total unknown %ld/%ld\n",
                    flow->flow_id, protoname, flow->detected_protocol.proto.app_protocol, flows_uncategorized, flows_categorized);
                print_flow_info_debug(flow);
            }
            else {
                flows_categorized++;
            }
#endif
            // should have reported as soon as possible. Maybe we should report it as not detected
            push_results_to_reporting(flow);

#ifdef USE_OLD_TSEARCH_FLOW_STORE
            /* adding to a queue (we can't delete it from the tree inline ) */
            ndpi_thread_info[thread_id].idle_flows[ndpi_thread_info[thread_id].num_idle_flows++] = flow;
#else
            HASH_DEL(flow->workflow->all_flows, flow);
            ndpi_flow_info_freer(flow);
#endif
        }
    }
}

/* *********************************************** */

/**
 * @brief Force a pcap_dispatch() or pcap_loop() call to return
 */
static void breakPcapLoop(u_int16_t thread_id) {
#ifdef USE_DPDK
    dpdk_run_capture = 0;
#else
    if (ndpi_thread_info[thread_id].workflow->pcap_handle != NULL) {
        pcap_breakloop(ndpi_thread_info[thread_id].workflow->pcap_handle);
    }
#endif
}

/**
 * @brief Sigproc is executed for each packet in the pcap file
 */
void sigproc(int sig) {

    static int called = 0;
    int thread_id;

    (void)sig;

    if (called) return; else called = 1;
    shutdown_app = 1;

    for (thread_id = 0; thread_id < num_reader_threads; thread_id++)
        breakPcapLoop(thread_id);
}

static void ndpi_check_idle_flows(u_int16_t thread_id) {
    if (ndpi_thread_info[thread_id].last_idle_scan_time + IDLE_SCAN_PERIOD < ndpi_thread_info[thread_id].workflow->last_time) {
        /* scan for idle flows */
#ifdef USE_OLD_TSEARCH_FLOW_STORE
        const u_int32_t idle_scan_idx = ndpi_thread_info[thread_id].idle_scan_idx;
        ndpi_twalk(ndpi_thread_info[thread_id].workflow->ndpi_flows_root[idle_scan_idx], node_idle_scan_walker, &thread_id);

        /* remove idle flows (unfortunately we cannot do this inline) */
        while (ndpi_thread_info[thread_id].num_idle_flows > 0) {
//printf("handling idle flow id %d\n", ndpi_thread_info[thread_id].idle_flows[ndpi_thread_info[thread_id].num_idle_flows-1]->flow_id);
            /* search and delete the idle flow from the "ndpi_flow_root" (see struct reader thread) - here flows are the node of a b-tree */
            struct ndpi_flow_info* flow = ndpi_thread_info[thread_id].idle_flows[--ndpi_thread_info[thread_id].num_idle_flows];
            ndpi_tdelete(flow, &ndpi_thread_info[thread_id].workflow->ndpi_flows_root[idle_scan_idx], ndpi_workflow_node_cmp);
            ndpi_flow_info_freer(flow);
        }

        if (++ndpi_thread_info[thread_id].idle_scan_idx == ndpi_thread_info[thread_id].workflow->prefs.num_roots)
            ndpi_thread_info[thread_id].idle_scan_idx = 0;
#else
        ndpi_flow_info* entry = NULL, * tmp = NULL;
        HASH_ITER(hh, ndpi_thread_info[thread_id].workflow->all_flows, entry, tmp) {
            node_idle_scan_walker(&entry, ndpi_leaf, 1, &thread_id);
        }
#endif
        // Todo, this should be root specific ?
        ndpi_thread_info[thread_id].last_idle_scan_time = ndpi_thread_info[thread_id].workflow->last_time;
    }
}

/**
 * @brief Check pcap packet
 */
static void ndpi_process_packet(u_char* args, const struct pcap_pkthdr* header, const u_char* packet) {
#ifdef ENABLE_PCAP_DUMPER
    ndpi_risk flow_risk;
#endif
    struct ndpi_flow_info* flow;
    u_int16_t thread_id_reader = *((u_int16_t*)args);

#ifdef CHECK_PACKET_CONTENT_INTEGRITY_AT_PROCESS
    /* allocate an exact size buffer to check overflows */
    uint8_t* packet_checked = ndpi_malloc(header->caplen);
    if (packet_checked == NULL) {
        return;
    }
    memcpy(packet_checked, packet, header->caplen);
#else
    const uint8_t* packet_checked = packet;
#endif

    // if we have worker threads, than we simply copy packet content and move along
    if (num_worker_threads > 0) {
        // this is a reader thread. Give this packet to a worksplit thread
        if (thread_id_reader < num_worksplit_threads) {
            if (header->caplen < LIBPCAP_SNAPLEN) {
                // get a buffer to store the packet
                pthread_mutex_lock(&ndpi_thread_info[thread_id_reader].packet_handover_lock);
                packet_buffer_store* bs = struct_pool_alloc(ndpi_thread_info[thread_id_reader].packet_buffer_pool);
                pthread_mutex_unlock(&ndpi_thread_info[thread_id_reader].packet_handover_lock);

                // prepare the buffer that will be processed by worker threads
                memcpy(&bs->header, header, sizeof(bs->header));
                memcpy(bs->data, packet, header->caplen); // the actual packet content
                bs->length = header->caplen;
                uint8_t target_worker_thread_idx = ndpi_thread_info[thread_id_reader].assign_work_to_thread_idx % num_worksplit_threads;
                ndpi_thread_info[thread_id_reader].assign_work_to_thread_idx++;
                bs->prev = NULL;
                bs->pcap_datalink = pcap_datalink(ndpi_thread_info[thread_id_reader].workflow->pcap_handle);
                bs->reader_thread_id = thread_id_reader; // return the packet buffer here
                bs->processing_stage = 0;

                // give the packet to a worker thread
                pthread_mutex_lock(&ndpi_thread_info[target_worker_thread_idx].packet_handover_lock);
                if( ndpi_thread_info[target_worker_thread_idx].packet_queue_head )
                    ndpi_thread_info[target_worker_thread_idx].packet_queue_head->prev = bs;
                if (ndpi_thread_info[target_worker_thread_idx].packet_queue_tail == NULL) {
                    ndpi_thread_info[target_worker_thread_idx].packet_queue_tail = bs;
                }
                pthread_cond_signal(&ndpi_thread_info[target_worker_thread_idx].packet_available_signal); // signal one waiting worker
                pthread_mutex_unlock(&ndpi_thread_info[target_worker_thread_idx].packet_handover_lock);

                // reader thread is done
                return;
            }
            else {
                AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceNDPI, "ndpi_process_packet : packet size %ld, is greater than max %ld. Throwing it away",
                    header->caplen, LIBPCAP_SNAPLEN);
                return;
            }
        }
    }

    // get as little info as possible to identify worker thread id
    ndpi_workflow_process_packet(ndpi_thread_info[thread_id_reader].workflow, header, packet_checked, 
#ifdef ENABLE_PCAP_DUMPER
        &flow_risk, 
#endif
        &flow);

    // run one or multiple plugins to try to identify this flow
    if (flow && flow->app_is_identified == 0) {
        nDPI_pkt_parser_params plugin_params;
        plugin_params.flow_to_process = flow;
        check_flow_app_assoc(&plugin_params);
    }

#ifdef ENABLE_PCAP_DUMPER
    if (!pcap_start.tv_sec) pcap_start.tv_sec = header->ts.tv_sec, pcap_start.tv_usec = header->ts.tv_usec;
    pcap_end.tv_sec = header->ts.tv_sec, pcap_end.tv_usec = header->ts.tv_usec;
#endif

    /* Idle flows cleanup */
//    if (live_capture) 
    {
        ndpi_check_idle_flows(thread_id_reader);
    }

#ifdef DEBUG_TRACE
    if (trace) fprintf(trace, "Found %u bytes packet %u.%u\n", header->caplen, p.proto.app_protocol, p.proto.master_protocol);
#endif

#ifdef ENABLE_PCAP_DUMPER
    if (extcap_dumper
        && ((extcap_packet_filter == (u_int16_t)-1)
            || (p.proto.app_protocol == extcap_packet_filter)
            || (p.proto.master_protocol == extcap_packet_filter)
            )
        ) {
        struct pcap_pkthdr h;
        u_int32_t* crc, delta = sizeof(struct ndpi_packet_trailer);
        struct ndpi_packet_trailer* trailer;
        u_int16_t cli_score, srv_score;

        memcpy(&h, header, sizeof(h));

        if (extcap_add_crc)
            delta += 4; /* ethernet trailer */

        if (h.caplen > (sizeof(extcap_buf) - delta)) {
            printf("INTERNAL ERROR: caplen=%u\n", h.caplen);
            h.caplen = sizeof(extcap_buf) - delta;
        }

        trailer = (struct ndpi_packet_trailer*)&extcap_buf[h.caplen];
        memcpy(extcap_buf, packet, h.caplen);
        memset(trailer, 0, sizeof(struct ndpi_packet_trailer));
        trailer->magic = htonl(WIRESHARK_NTOP_MAGIC);
        if (flow) {
            trailer->flags = flow->current_pkt_from_client_to_server;
            trailer->flags |= (flow->detection_completed << 2);
        }
        else {
            trailer->flags = 0 | (2 << 2);
        }
        trailer->flow_risk = htonl64(flow_risk);
        trailer->flow_score = htons(ndpi_risk2score(flow_risk, &cli_score, &srv_score));
        trailer->flow_risk_info_len = ntohs(WIRESHARK_FLOW_RISK_INFO_SIZE);
        if (flow && flow->risk_str) {
            strncpy(trailer->flow_risk_info, flow->risk_str, sizeof(trailer->flow_risk_info));
        }
        trailer->flow_risk_info[sizeof(trailer->flow_risk_info) - 1] = '\0';
        trailer->proto.master_protocol = htons(p.proto.master_protocol), trailer->proto.app_protocol = htons(p.proto.app_protocol);
        ndpi_protocol2name(ndpi_thread_info[thread_id_reader].workflow->ndpi_struct, p, trailer->name, sizeof(trailer->name));

        /* Metadata */
        /* Metadata are (all) available in `flow` only after nDPI completed its work!
           We export them only once */
           /* TODO: boundary check. Right now there is always enough room, but we should check it if we are
              going to extend the list of the metadata exported */
        trailer->metadata_len = ntohs(WIRESHARK_METADATA_SIZE);
        struct ndpi_packet_tlv* tlv = (struct ndpi_packet_tlv*)trailer->metadata;
        int tot_len = 0;
        if (flow && flow->detection_completed == 1) {
            if (flow->host_server_name[0] != '\0') {
                tlv->type = ntohs(WIRESHARK_METADATA_SERVERNAME);
                tlv->length = ntohs(sizeof(flow->host_server_name));
                memcpy(tlv->data, flow->host_server_name, sizeof(flow->host_server_name));
                /* TODO: boundary check */
                tot_len += 4 + htons(tlv->length);
                tlv = (struct ndpi_packet_tlv*)&trailer->metadata[tot_len];
            }
            if (flow->ssh_tls.ja4_client[0] != '\0') {
                tlv->type = ntohs(WIRESHARK_METADATA_JA4C);
                tlv->length = ntohs(sizeof(flow->ssh_tls.ja4_client));
                memcpy(tlv->data, flow->ssh_tls.ja4_client, sizeof(flow->ssh_tls.ja4_client));
                /* TODO: boundary check */
                tot_len += 4 + htons(tlv->length);
                tlv = (struct ndpi_packet_tlv*)&trailer->metadata[tot_len];
            }

            flow->detection_completed = 2; /* Avoid exporting metadata again.
                                              If we really want to have the metadata on Wireshark for *all*
                                              the future packets of this flow, simply remove that assignment */
        }
        /* Last: padding */
        tlv->type = 0;
        tlv->length = ntohs(WIRESHARK_METADATA_SIZE - tot_len - 4);
        /* The remaining bytes are already set to 0 */

        if (extcap_add_crc) {
            crc = (uint32_t*)&extcap_buf[h.caplen + sizeof(struct ndpi_packet_trailer)];
            *crc = ndpi_crc32((const void*)extcap_buf, h.caplen + sizeof(struct ndpi_packet_trailer), 0);
        }
        h.caplen += delta, h.len += delta;

#ifdef DEBUG_TRACE
        if (trace) fprintf(trace, "Dumping %u bytes packet\n", h.caplen);
#endif

        pcap_dump((u_char*)extcap_dumper, &h, (const u_char*)extcap_buf);
        pcap_dump_flush(extcap_dumper);
    }
#endif

#ifdef CHECK_PACKET_CONTENT_INTEGRITY_AT_PROCESS
    /* check for buffer changes */
    if (memcmp(packet, packet_checked, header->caplen) != 0)
        printf("INTERNAL ERROR: ingress packet was modified by nDPI: this should not happen [thread_id_reader=%u, packetId=%lu, caplen=%u]\n",
            thread_id_reader, (unsigned long)ndpi_thread_info[thread_id_reader].workflow->stats.raw_packet_count, header->caplen);
#endif

#ifdef ENABLE_PCAP_INTERVAL_ANALISYS
    if ((u_int32_t)(pcap_end.tv_sec - pcap_start.tv_sec) > pcap_analysis_duration) {
        unsigned int i;
        u_int64_t processing_time_usec, setup_time_usec;

        gettimeofday(&end, NULL);
        processing_time_usec = (u_int64_t)end.tv_sec * 1000000 + end.tv_usec - ((u_int64_t)begin.tv_sec * 1000000 + begin.tv_usec);
        setup_time_usec = (u_int64_t)begin.tv_sec * 1000000 + begin.tv_usec - ((u_int64_t)startup_time.tv_sec * 1000000 + startup_time.tv_usec);

        printResults(processing_time_usec, setup_time_usec);

        for (i = 0; i < ndpi_thread_info[thread_id_reader].workflow->prefs.num_roots; i++) {
            ndpi_tdestroy(ndpi_thread_info[thread_id_reader].workflow->ndpi_flows_root[i], ndpi_flow_info_freer);
            ndpi_thread_info[thread_id_reader].workflow->ndpi_flows_root[i] = NULL;

#ifdef ENABLE_FLOW_STATS_GENERATION
            memset(&ndpi_thread_info[thread_id_reader].workflow->stats, 0, sizeof(struct ndpi_stats));
#endif
        }

        if (!quiet_mode)
            printf("\n-------------------------------------------\n\n");

        memcpy(&begin, &end, sizeof(begin));
        memcpy(&pcap_start, &pcap_end, sizeof(pcap_start));
    }
#endif

#ifdef CHECK_PACKET_CONTENT_INTEGRITY_AT_PROCESS
    /*
      Leave the free as last statement to avoid crashes when ndpi_detection_giveup()
      is called above by printResults()
    */
    if (packet_checked) {
        ndpi_free(packet_checked);
        packet_checked = NULL;
    }
#endif
}

#ifndef USE_DPDK
/**
 * @brief Call pcap_loop() to process packets from a live capture or savefile
 */
static void runPcapLoop(u_int16_t thread_id) {
    if ((!shutdown_app) && (ndpi_thread_info[thread_id].workflow->pcap_handle != NULL)) {
        int datalink_type = pcap_datalink(ndpi_thread_info[thread_id].workflow->pcap_handle);

        /* When using as extcap interface, the output/dumper pcap must have the same datalink
           type of the input traffic [to be able to use, for example, input pcaps with
           Linux "cooked" capture encapsulation (i.e. captured with "any" interface...) where
           there isn't an ethernet header] */
        if (do_extcap_capture) {
            extcap_capture(datalink_type);
            if (datalink_type == DLT_EN10MB)
                extcap_add_crc = 1;
        }

        if (!ndpi_is_datalink_supported(datalink_type)) {
            printf("Unsupported datalink %d. Skip pcap\n", datalink_type);
            return;
        }
        int ret = pcap_loop(ndpi_thread_info[thread_id].workflow->pcap_handle, -1, &ndpi_process_packet, (u_char*)&thread_id);
        if (ret == -1)
            printf("Error while reading pcap file: '%s'\n", pcap_geterr(ndpi_thread_info[thread_id].workflow->pcap_handle));
    }
}
#endif

/**
 * @brief Process a running thread
 */
void* processing_thread(void* _thread_id) {
    num_reader_threads_running++;
#ifdef WIN64
    long long int thread_id = (long long int)_thread_id;
#else
    long int thread_id = (long int)_thread_id;
#endif
#ifndef USE_DPDK
    char pcap_error_buffer[PCAP_ERRBUF_SIZE];
#endif

#if defined(__linux__) && defined(HAVE_PTHREAD_SETAFFINITY_NP)
    if (core_affinity[thread_id] >= 0) {
        cpu_set_t cpuset;

        CPU_ZERO(&cpuset);
        CPU_SET(core_affinity[thread_id], &cpuset);

        if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0)
            fprintf(stderr, "Error while binding thread %ld to core %d\n", thread_id, core_affinity[thread_id]);
        else {
            if (!quiet_mode) printf("Running reader thread %ld on core %d...\n", thread_id, core_affinity[thread_id]);
        }
    }
    else
#endif
        if ((!quiet_mode)) {
#ifdef WIN64
            printf("Running reader thread %lld...\n", thread_id);
#else
            printf("Running reader thread %ld...\n", thread_id);
#endif
        }

#ifdef USE_DPDK
    while (dpdk_run_capture) {
        struct rte_mbuf* bufs[BURST_SIZE];
        u_int16_t num = rte_eth_rx_burst(dpdk_port_id, 0, bufs, BURST_SIZE);
        u_int i;

        if (num == 0) {
            usleep(1);
            continue;
        }

        for (i = 0; i < PREFETCH_OFFSET && i < num; i++)
            rte_prefetch0(rte_pktmbuf_mtod(bufs[i], void*));

        for (i = 0; i < num; i++) {
            char* data = rte_pktmbuf_mtod(bufs[i], char*);
            int len = rte_pktmbuf_pkt_len(bufs[i]);
            struct pcap_pkthdr h;

            h.len = h.caplen = len;
            gettimeofday(&h.ts, NULL);

            ndpi_process_packet((u_char*)&thread_id, &h, (const u_char*)data);
            rte_pktmbuf_free(bufs[i]);
        }
    }
#else
pcap_loop:
    runPcapLoop(thread_id);

    if (ndpi_thread_info[thread_id].workflow->pcap_handle)
        pcap_close(ndpi_thread_info[thread_id].workflow->pcap_handle);

    ndpi_thread_info[thread_id].workflow->pcap_handle = NULL;

    if (playlist_fp[thread_id] != NULL) { /* playlist: read next file */
        char filename[256];

        if (getNextPcapFileFromPlaylist(thread_id, filename, sizeof(filename)) == 0 &&
            (ndpi_thread_info[thread_id].workflow->pcap_handle = pcap_open_offline(filename, pcap_error_buffer)) != NULL) {
            configurePcapHandle(ndpi_thread_info[thread_id].workflow->pcap_handle);
            goto pcap_loop;
        }
    }
#endif
    if (bpf_cfilter) {
        pcap_freecode(bpf_cfilter);
        bpf_cfilter = NULL;
    }
    num_reader_threads_running--;

    // flush buffered logs
    FlushThreadLogBuffer();

    return NULL;
}

// worker threads will receive packets from reader threads instead of libpcap
void* processing_thread_worker(void* _thread_id) {
#ifdef UNFINISHED_CODE_FOR_MULTI_THREADED_PROCESSING
    u_int16_t thread_id = (long int)_thread_id;

#if defined(__linux__) && defined(HAVE_PTHREAD_SETAFFINITY_NP)
    if (core_affinity[thread_id] >= 0) {
        cpu_set_t cpuset;

        CPU_ZERO(&cpuset);
        CPU_SET(core_affinity[thread_id], &cpuset);

        if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0)
            fprintf(stderr, "Error while binding thread %d to core %d\n", thread_id, core_affinity[thread_id]);
        else {
            if (!quiet_mode) printf("Running worker thread %d on core %d...\n", thread_id, core_affinity[thread_id]);
        }
    }
    else
#endif
    {
        if ((!quiet_mode)) {
            printf("Running worker thread %d...\n", thread_id);
        }
    }

    nDPI_reader_thread* my_thread = &ndpi_thread_info[thread_id];
    while (!shutdown_app && num_reader_threads_running > 0) {
        pthread_mutex_lock(&my_thread->packet_handover_lock);

        // unlock the mutex and wait until a packet arrives in our queue
        while (my_thread->packet_queue_tail == NULL && !shutdown_app && num_reader_threads_running > 0) {
            pthread_cond_wait(&my_thread->packet_available_signal, &my_thread->packet_handover_lock);
        }

        // we are requested to stop processing packets. abandon loop
        if (shutdown_app || num_reader_threads_running <= 0) {
            pthread_mutex_unlock(&my_thread->packet_handover_lock);
            break;
        }

        // pop a packet from the FIFO
        packet_buffer_store* pkt = my_thread->packet_queue_tail;
        my_thread->packet_queue_tail = pkt->prev;
        if (pkt == my_thread->packet_queue_head) {
            my_thread->packet_queue_head = NULL;
        }

        pthread_mutex_unlock(&my_thread->packet_handover_lock);

        // this is a worksplit thread. Need to parse the packet hader so we may assign it to a worker thread
        if (thread_id < num_reader_threads + num_worksplit_threads) {
            // not sure we ever use this
            pkt->processing_stage = 1;

            // implementation is missing here :
            // parse packet header to get a direction agnostic idx : ip1^ip2^port1^port2 ?
            size_t direction_agnostic_number = get_worker_thread_index_full(pkt->pcap_datalink, &pkt->header, pkt->data);
            size_t target_worker_thread_idx = direction_agnostic_number % num_worker_threads;
            // move the packet to the worker thread
            pthread_mutex_lock(&ndpi_thread_info[target_worker_thread_idx].packet_handover_lock);
            if (ndpi_thread_info[target_worker_thread_idx].packet_queue_head)
                ndpi_thread_info[target_worker_thread_idx].packet_queue_head->prev = pkt;
            if (ndpi_thread_info[target_worker_thread_idx].packet_queue_tail == NULL) {
                ndpi_thread_info[target_worker_thread_idx].packet_queue_tail = pkt;
            }
            pthread_cond_signal(&ndpi_thread_info[target_worker_thread_idx].packet_available_signal); // signal one waiting worker
            pthread_mutex_unlock(&ndpi_thread_info[target_worker_thread_idx].packet_handover_lock);
        }
        // this is a worker thread, process the packet
        else {
            // worker thread will parse whole packet + create flow + process results
            ndpi_process_packet((u_char*)&thread_id, &pkt->header, pkt->data);

            // give back the borrowed buffer to the owner reader thread
            size_t thread_id_borrowed_buffer_from = pkt->reader_thread_id;
            pthread_mutex_lock(&ndpi_thread_info[thread_id_borrowed_buffer_from].packet_handover_lock);
            struct_pool_free(ndpi_thread_info[thread_id_borrowed_buffer_from].packet_buffer_pool, pkt);
            pthread_mutex_unlock(&ndpi_thread_info[thread_id_borrowed_buffer_from].packet_handover_lock);
        }
    }

    // free up allocated packets
    while (my_thread->packet_queue_tail) {
        packet_buffer_store* pkt = my_thread->packet_queue_tail;
        my_thread->packet_queue_tail = pkt->prev;

        size_t thread_id_borrowed_buffer_from = pkt->reader_thread_id;
        pthread_mutex_lock(&ndpi_thread_info[thread_id_borrowed_buffer_from].packet_handover_lock);
        struct_pool_free(ndpi_thread_info[thread_id_borrowed_buffer_from].packet_buffer_pool, pkt);
        pthread_mutex_unlock(&ndpi_thread_info[thread_id_borrowed_buffer_from].packet_handover_lock);
    }
    my_thread->packet_queue_head = NULL; // sanity

    // flush buffered logs
    FlushThreadLogBuffer();
#endif
    return NULL;
}

/* ***************************************************** */

static u_int32_t reader_slot_malloc_bins(u_int64_t v)
{
    int i;

    /* 0-2,3-4,5-8,9-16,17-32,33-64,65-128,129-256,257-512,513-1024,1025-2048,2049-4096,4097-8192,8193- */
    for (i = 0; i < max_malloc_bins - 1; i++)
        if ((1ULL << (i + 1)) >= v)
            return i;
    return i;
}

/**
 * @brief ndpi_malloc wrapper function
 */
static void* ndpi_malloc_wrapper(size_t size) {
    current_ndpi_memory += size;

    if (current_ndpi_memory > max_ndpi_memory)
        max_ndpi_memory = current_ndpi_memory;

    if (enable_malloc_bins && malloc_size_stats)
        ndpi_inc_bin(&malloc_bins, reader_slot_malloc_bins(size), 1);

#if defined(_DEBUG) && defined(_STAT_MEM_ALLOCS)
    static size_t total_alloc_size = 0;
    static size_t total_alloc_count = 0;
    total_alloc_size += size;
    total_alloc_count++;
//    AddLogEntryB(LDF_LOCAL, LogSeverityDebug, LogSourceNDPI, "ndpi_malloc_wrapper : size %ld, total alloc count %ld, total alloc size %ld", size, total_alloc_count, total_alloc_size);
    on_memory_allocated(size);
#endif 

    return(malloc(size)); /* Don't change to ndpi_malloc !!!!! */
}

/* ***************************************************** */

/**
 * @brief free wrapper function
 */
static void free_wrapper(void* freeable) {
    free(freeable); /* Don't change to ndpi_free !!!!! */
}

static void on_protocol_discovered(struct ndpi_workflow* workflow, struct ndpi_flow_info* flow, void* userdata)
{
    (void)userdata;
    if (enable_realtime_output != 0)
        dump_realtime_protocol(workflow, flow);
}

/**
 * @brief Open a pcap file or a specified device - Always returns a valid pcap_t
 */
static pcap_t* openPcapFileOrDevice(u_int16_t thread_id, const u_char* pcap_file) {
#ifndef USE_DPDK
    u_int snaplen = LIBPCAP_SNAPLEN;
    int promisc = 1;
    char pcap_error_buffer[PCAP_ERRBUF_SIZE];
#endif
    pcap_t* pcap_handle = NULL;

    /* trying to open a live interface */
#ifdef USE_DPDK
    struct rte_mempool* mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS,
        MBUF_CACHE_SIZE, 0,
        RTE_MBUF_DEFAULT_BUF_SIZE,
        rte_socket_id());

    if (mbuf_pool == NULL)
        rte_exit(EXIT_FAILURE, "Cannot create mbuf pool: are hugepages ok?\n");

    if (dpdk_port_init(dpdk_port_id, mbuf_pool) != 0)
        rte_exit(EXIT_FAILURE, "DPDK: Cannot init port %u: please see README.dpdk\n", dpdk_port_id);
#else
  /* Trying to open the interface */
    if ((pcap_handle = pcap_open_live((char*)pcap_file, snaplen,
        promisc, 500, pcap_error_buffer)) == NULL) {
        capture_for = capture_until = 0;

        live_capture = 0;
        num_reader_threads = 1; /* Open pcap files in single threads mode */

        /* Trying to open a pcap file */
        if ((pcap_handle = pcap_open_offline((char*)pcap_file, pcap_error_buffer)) == NULL) {
            char filename[256] = { 0 };

            if (strstr((char*)pcap_file, (char*)".pcap"))
                printf("ERROR: could not open pcap file: %s\n", pcap_error_buffer);

            /* Trying to open as a playlist as last attempt */
            else if ((getNextPcapFileFromPlaylist(thread_id, filename, sizeof(filename)) != 0)
                || ((pcap_handle = pcap_open_offline(filename, pcap_error_buffer)) == NULL)) {
                /* This probably was a bad interface name, printing a generic error */
                printf("ERROR: could not open %s: %s\n", filename, pcap_error_buffer);
                exit(-1);
            }
            else {
                if (!quiet_mode)
                    printf("Reading packets from playlist %s...\n", pcap_file);
            }
        }
        else {
            if (!quiet_mode)
                printf("Reading packets from pcap file %s...\n", pcap_file);
        }
    }
    else {
        live_capture = 1;

        if (!quiet_mode) {
#ifdef USE_DPDK
            printf("Capturing from DPDK (port 0)...\n");
#else
            printf("Capturing live traffic from device %s...\n", pcap_file);
#endif
        }
    }

    configurePcapHandle(pcap_handle);
#endif /* !DPDK */

    if (capture_for > 0) {
        if (!quiet_mode)
            printf("Capturing traffic up to %u seconds\n", (unsigned int)capture_for);

#ifndef WIN32
        alarm(capture_for);
        signal(SIGALRM, sigproc);
#endif
    }

    return pcap_handle;
}
/* *********************************************** */

static void node_report_on_shutdown_walker(const void* node, ndpi_VISIT which, int depth, void* user_data) {
    struct ndpi_flow_info* flow = *(struct ndpi_flow_info**)node;
//    u_int16_t thread_id = *((u_int16_t*)user_data);

    if ((which == ndpi_preorder) || (which == ndpi_leaf)) { /* Avoid walking the same node multiple times */
        /* update stats */
        node_proto_guess_walker(node, which, depth, user_data);
        if (verbose == 3)
            port_stats_walker(node, which, depth, user_data);

        if ((flow->detected_protocol.proto.app_protocol == NDPI_PROTOCOL_UNKNOWN) && !undetected_flows_deleted)
            undetected_flows_deleted = 1;

        if (flow->detected_protocol.proto.app_protocol == NDPI_PROTOCOL_UNKNOWN) {
            AddLogEntryB(LDF_LOCAL, LogSeverityDebug, LogSourceWorkerThread, "Failed to detect protocol for flow %ld\n",
                flow->flow_id);
        }

        // should have processed this sooner. We might get flows that failed to detect proto fast enough
        if (flow->app_is_identified == 0) {
            nDPI_pkt_parser_params plugin_params;
            plugin_params.flow_to_process = flow;
            check_flow_app_assoc(&plugin_params);
        }

#ifndef _APP_MODE_BUILD_DB
        if (flow->app_is_identified == 0 && flow->app_is_identified_unsure == 0) {
            flows_uncategorized++;
            const char* protoname = ndpi_protocol_id_to_str(flow->detected_protocol.proto.app_protocol);
            AddLogEntryB(LDF_LOCAL, LogSeverityDebug, LogSourceWorkerThread, "Failed to detect app_id for flow %ld, proto %s(%d). Total unknown %ld/%ld\n",
                flow->flow_id, protoname, flow->detected_protocol.proto.app_protocol, flows_uncategorized, flows_categorized);
            print_flow_info_debug(flow);
        }
        else {
            flows_categorized++;
        }
#endif
        // should have reported as soon as possible. Maybe we should report it as not detected
        push_results_to_reporting(flow);
    }
}

static void force_report_all_remaining_flows(uint16_t thread_id) {
    struct ndpi_workflow* workflow = ndpi_thread_info[thread_id].workflow;
#ifdef USE_OLD_TSEARCH_FLOW_STORE
    // This walker will deallocate flows
    for (size_t i = 0; i < workflow->prefs.num_roots; i++) {
        ndpi_twalk(ndpi_thread_info[thread_id].workflow->ndpi_flows_root[i], node_report_on_shutdown_walker, &thread_id);
    }
#else
    ndpi_flow_info* entry = NULL, * tmp = NULL;
    HASH_ITER(hh, workflow->all_flows, entry, tmp) {
        node_report_on_shutdown_walker(&entry, ndpi_leaf, 1, &thread_id);
    }
#endif
}

/**
 * @brief End of detection and free flow
 */
static void terminateDetection(u_int16_t thread_id) {
    // report all flows. This is used when building the DB. Or when making sure there are no mem leaks
    force_report_all_remaining_flows(thread_id);
    // 
    ndpi_workflow_free(ndpi_thread_info[thread_id].workflow);
    ndpi_thread_info[thread_id].workflow = NULL;
    struct_pool_destroy(ndpi_thread_info[thread_id].packet_buffer_pool);
    ndpi_thread_info[thread_id].packet_buffer_pool = NULL;
    pthread_mutex_destroy(&ndpi_thread_info[thread_id].packet_handover_lock);
    pthread_cond_destroy(&ndpi_thread_info[thread_id].packet_available_signal);
}

/* ***************************************************** */

/**
 * @brief Setup for detection begin
 */
static void setupDetection(u_int16_t thread_id, pcap_t* pcap_handle,
    struct ndpi_global_context* g_ctx) {
    NDPI_PROTOCOL_BITMASK enabled_bitmask;
    struct ndpi_workflow_prefs prefs;
    int i, ret;
    ndpi_cfg_error rc;

    memset(&prefs, 0, sizeof(prefs));
    prefs.decode_tunnels = decode_tunnels;
    prefs.num_roots = NUM_ROOTS;
    prefs.max_ndpi_flows = MAX_NDPI_FLOWS;
    prefs.quiet_mode = quiet_mode;
    prefs.ignore_vlanid = ignore_vlanid;

    memset(&ndpi_thread_info[thread_id], 0, sizeof(ndpi_thread_info[thread_id]));
    ndpi_thread_info[thread_id].workflow = ndpi_workflow_init(&prefs, pcap_handle, 1,
        serialization_format, g_ctx);

    /* Protocols to enable/disable. Default: everything is enabled */
    NDPI_BITMASK_SET_ALL(enabled_bitmask);
    if (_disabled_protocols != NULL) {
        if (parse_proto_name_list(_disabled_protocols, &enabled_bitmask, 1))
            exit(-1);
    }

    if (_categoriesDirPath) {
        int failed_files = ndpi_load_categories_dir(ndpi_thread_info[thread_id].workflow->ndpi_struct, _categoriesDirPath);
        if (failed_files < 0) {
            fprintf(stderr, "Failed to parse all *.list files in: %s\n", _categoriesDirPath);
            exit(-1);
        }
    }

    if (_domain_suffixes)
        ndpi_load_domain_suffixes(ndpi_thread_info[thread_id].workflow->ndpi_struct, _domain_suffixes);

    if (_riskyDomainFilePath)
        ndpi_load_risk_domain_file(ndpi_thread_info[thread_id].workflow->ndpi_struct, _riskyDomainFilePath);

    if (_maliciousJA4Path)
        ndpi_load_malicious_ja4_file(ndpi_thread_info[thread_id].workflow->ndpi_struct, _maliciousJA4Path);

    if (_maliciousSHA1Path)
        ndpi_load_malicious_sha1_file(ndpi_thread_info[thread_id].workflow->ndpi_struct, _maliciousSHA1Path);

    if (_customCategoryFilePath) {
        char* label = strrchr(_customCategoryFilePath, '/');

        if (label != NULL)
            label = &label[1];
        else
            label = _customCategoryFilePath;

        int failed_lines = ndpi_load_categories_file(ndpi_thread_info[thread_id].workflow->ndpi_struct, _customCategoryFilePath, label);
        if (failed_lines < 0) {
            fprintf(stderr, "Failed to parse custom categories file: %s\n", _customCategoryFilePath);
            exit(-1);
        }
    }

    ndpi_thread_info[thread_id].workflow->g_ctx = g_ctx;

    if (enable_realtime_output != 0) {
        ndpi_workflow_set_flow_callback(ndpi_thread_info[thread_id].workflow, on_protocol_discovered, NULL);
    }

    /* Make sure to load lists before finalizing the initialization */
    ndpi_set_protocol_detection_bitmask2(ndpi_thread_info[thread_id].workflow->ndpi_struct, &enabled_bitmask);

    if (_protoFilePath != NULL)
        ndpi_load_protocols_file(ndpi_thread_info[thread_id].workflow->ndpi_struct, _protoFilePath);

    ndpi_set_config(ndpi_thread_info[thread_id].workflow->ndpi_struct, NULL, "tcp_ack_payload_heuristic", "enable");

    for (i = 0; i < num_cfgs; i++) {
        rc = ndpi_set_config(ndpi_thread_info[thread_id].workflow->ndpi_struct,
            cfgs[i].proto, cfgs[i].param, cfgs[i].value);
        if (rc != NDPI_CFG_OK) {
            fprintf(stderr, "Error setting config [%s][%s][%s]: %s (%d)\n",
                (cfgs[i].proto != NULL ? cfgs[i].proto : ""),
                cfgs[i].param, cfgs[i].value, ndpi_cfg_error2string(rc), rc);
            exit(-1);
        }
    }

    if (enable_doh_dot_detection)
        ndpi_set_config(ndpi_thread_info[thread_id].workflow->ndpi_struct, "tls", "application_blocks_tracking", "enable");

    if (addr_dump_path != NULL)
        ndpi_cache_address_restore(ndpi_thread_info[thread_id].workflow->ndpi_struct, addr_dump_path, 0);

    ret = ndpi_finalize_initialization(ndpi_thread_info[thread_id].workflow->ndpi_struct);
    if (ret != 0) {
        fprintf(stderr, "Error ndpi_finalize_initialization: %d\n", ret);
        exit(-1);
    }

    char buf[16];
    if (ndpi_get_config(ndpi_thread_info[thread_id].workflow->ndpi_struct, "stun", "monitoring", buf, sizeof(buf)) != NULL) {
        if (atoi(buf))
            monitoring_enabled = 1;
    }

    pthread_mutex_init(&ndpi_thread_info[thread_id].packet_handover_lock, NULL);
    pthread_cond_init(&ndpi_thread_info[thread_id].packet_available_signal, NULL);
    ndpi_thread_info[thread_id].packet_buffer_pool = struct_pool_create(sizeof(packet_buffer_store), 2);
}

/* *********************************************** */

/**
 * @brief Begin, process, end detection process
 */
void start_process_loops() {
    u_int64_t processing_time_usec, setup_time_usec;
#ifdef WIN64
    long long int thread_id;
#else
    long thread_id;
#endif
    struct ndpi_global_context* g_ctx;

    set_ndpi_malloc(ndpi_malloc_wrapper), set_ndpi_free(free_wrapper);
    set_ndpi_flow_malloc(NULL), set_ndpi_flow_free(NULL);

#ifndef USE_GLOBAL_CONTEXT
    /* ndpiReader works even if libnDPI has been compiled without global context support,
       but you can't configure any cache with global scope */
    g_ctx = NULL;
#else
    g_ctx = ndpi_global_init();
    if (!g_ctx) {
        fprintf(stderr, "Error ndpi_global_init\n");
        exit(-1);
    }
#endif

#ifdef DEBUG_TRACE
    if (trace) fprintf(trace, "Num threads: %d\n", num_reader_threads);
#endif

    for (thread_id = 0; thread_id < num_reader_threads; thread_id++) {
        pcap_t* cap;

#ifdef DEBUG_TRACE
        if (trace) fprintf(trace, "Opening %s\n", (const u_char*)_pcap_file[thread_id]);
#endif

        cap = openPcapFileOrDevice(thread_id, (const u_char*)_pcap_file[thread_id]);
        setupDetection(thread_id, cap, g_ctx);
    }
    for (thread_id = num_reader_threads; thread_id < num_reader_threads + num_worksplit_threads + num_worker_threads; thread_id++) {
        setupDetection(thread_id, NULL, g_ctx);
    }

    gettimeofday(&begin, NULL);

    int status;
    void* thd_res;

#if defined(_DEBUG) && defined(_STAT_MEM_ALLOCS)
    toggle_allocation_statistics(1);
#endif

    /* Running processing threads */
    for (thread_id = 0; thread_id < num_reader_threads; thread_id++) {
        status = pthread_create(&ndpi_thread_info[thread_id].pthread, NULL, processing_thread, (void*)thread_id);
        /* check pthreade_create return value */
        if (status != 0) {
#ifdef WIN64
            fprintf(stderr, "error on create %lld thread\n", thread_id);
#else
            fprintf(stderr, "error on create %ld thread\n", thread_id);
#endif
            exit(-1);
        }
    }
    for (thread_id = num_reader_threads; thread_id < num_reader_threads + num_worksplit_threads + num_worker_threads; thread_id++) {
        status = pthread_create(&ndpi_thread_info[thread_id].pthread, NULL, processing_thread_worker, (void*)thread_id);
        /* check pthreade_create return value */
        if (status != 0) {
#ifdef WIN64
            fprintf(stderr, "error on create %lld thread\n", thread_id);
#else
            fprintf(stderr, "error on create %ld thread\n", thread_id);
#endif
            exit(-1);
    }
    }
    /* Waiting for completion */
    for (thread_id = 0; thread_id < num_reader_threads + num_worksplit_threads + num_worker_threads; thread_id++) {
        // make sure worker threads are no longer waiting on reader threads
        if (thread_id >= num_reader_threads) {
            num_reader_threads_running = 0;
            pthread_mutex_lock(&ndpi_thread_info[thread_id].packet_handover_lock);
            pthread_cond_signal(&ndpi_thread_info[thread_id].packet_available_signal);
            pthread_mutex_unlock(&ndpi_thread_info[thread_id].packet_handover_lock);
        }
        status = pthread_join(ndpi_thread_info[thread_id].pthread, &thd_res);
        /* check pthreade_join return value */
        if (status != 0) {
#ifdef WIN64
            fprintf(stderr, "error on join %lld thread\n", thread_id);
#else
            fprintf(stderr, "error on join %ld thread\n", thread_id);
#endif
            exit(-1);
        }
        if (thd_res != NULL) {
#ifdef WIN64
            fprintf(stderr, "error on returned value of %lld joined thread\n", thread_id);
#else
            fprintf(stderr, "error on returned value of %ld joined thread\n", thread_id);
#endif
            exit(-1);
        }
    }

#ifdef USE_DPDK
    dpdk_port_deinit(dpdk_port_id);
#endif

    gettimeofday(&end, NULL);
    processing_time_usec = (u_int64_t)end.tv_sec * 1000000 + end.tv_usec - ((u_int64_t)begin.tv_sec * 1000000 + begin.tv_usec);
    setup_time_usec = (u_int64_t)begin.tv_sec * 1000000 + begin.tv_usec - ((u_int64_t)startup_time.tv_sec * 1000000 + startup_time.tv_usec);

    /* Printing cumulative results */
    printResults(processing_time_usec, setup_time_usec);

    for (thread_id = 0; thread_id < num_reader_threads + num_worksplit_threads + num_worker_threads; thread_id++) {
        if (ndpi_thread_info[thread_id].workflow->pcap_handle != NULL)
            pcap_close(ndpi_thread_info[thread_id].workflow->pcap_handle);

        terminateDetection(thread_id);
    }

    ndpi_global_deinit(g_ctx);
}


/* *********************************************** */

/**
   @brief MAIN FUNCTION
**/
int main(int argc, char** argv) {
    int i;
#ifdef NDPI_EXTENDED_SANITY_CHECKS
    int skip_unit_tests = 0;
#else
    int skip_unit_tests = 1;
#endif

#ifdef DEBUG_TRACE
    trace = fopen("/tmp/ndpiReader.log", "a");

    if (trace) {
        int i;

        fprintf(trace, " #### %s #### \n", __FUNCTION__);
        fprintf(trace, " #### [argc: %u] #### \n", argc);

        for (i = 0; i < argc; i++)
            fprintf(trace, " #### [%d] [%s]\n", i, argv[i]);
    }
#endif

    if (ndpi_get_api_version() != NDPI_API_VERSION) {
        printf("nDPI Library version mismatch: please make sure this code and the nDPI library are in sync\n");
        return(-1);
    }

    if (!skip_unit_tests) {
#ifndef DEBUG_TRACE
        /* Skip tests when debugging */

#ifdef HW_TEST
        hwUnitTest2();
#endif

#ifdef STRESS_TEST
        desUnitStressTest();
        exit(0);
#endif
        run_unittests();
#endif
    }

    gettimeofday(&startup_time, NULL);
    memset(ndpi_thread_info, 0, sizeof(ndpi_thread_info));

    if (getenv("AHO_DEBUG"))
        ac_automata_enable_debug(1);

    parseOptions(argc, argv);

    if (domain_to_check) {
        ndpiCheckHostStringMatch(domain_to_check);
        exit(0);
    }

    if (ip_port_to_check) {
        ndpiCheckIPMatch(ip_port_to_check);
        exit(0);
    }

    if (enable_doh_dot_detection) {
        init_doh_bins();
        /* Clusters are not really used in DoH/DoT detection, but because of how
           the code has been written, we need to enable also clustering feature */
        if (num_bin_clusters == 0)
            num_bin_clusters = 1;
    }

#ifdef CUSTOM_NDPI_PROTOCOLS
#include "../../nDPI-custom/ndpiReader_init.c"
#endif

    if (!quiet_mode) {
        printf("Using nDPI (%s) [%d thread(s)]\n", ndpi_revision(), num_reader_threads + num_worksplit_threads + num_worker_threads);

        const char* gcrypt_ver = ndpi_get_gcrypt_version();
        if (gcrypt_ver)
            printf("Using libgcrypt version %s\n", gcrypt_ver);
    }

    int ret_code = 0;
    char app_path[PATH_MAX];
    if (realpath(argv[0], app_path) == NULL) {
        perror("realpath");
        return 1;
    }
    char* dir = dirname(app_path);
    // load configs
    char ini_path[PATH_MAX];
    snprintf(ini_path, sizeof(ini_path), "%s/%s", dir, "ndpi.ini");
    printf("Loading ini from %s\n", ini_path);
    parse_ini_file(ini_path);

    // to file logger init
    init_log_manager(dir);
    if (argc == 0) {
        printf("usage: ndpiSimpleIntegration <device name>\n");
        ret_code = 1;
        goto cleanup_and_exit;
    }

    // init global pool manager
    init_pool_manager();

    // plugins to load / store data
    init_mysql_interface();

    // init all plugins based on the current usage mode
    init_plugin_globals();

    signal(SIGINT, sigproc);

    // actual processing ini and loop start
    for (i = 0; i < num_loops; i++)
        start_process_loops();

cleanup_and_exit:
    if (results_path)  ndpi_free(results_path);
    if (results_file)  fclose(results_file);
    if (extcap_dumper) pcap_dump_close(extcap_dumper);
    if (extcap_fifo_h) pcap_close(extcap_fifo_h);
    if (enable_malloc_bins) ndpi_free_bin(&malloc_bins);
    if (csv_fp)         fclose(csv_fp);
    if (fingerprint_fp) fclose(fingerprint_fp);

    ndpi_free(_disabled_protocols);

    for (i = 0; i < num_cfgs; i++) {
        ndpi_free(cfgs[i].proto);
        ndpi_free(cfgs[i].param);
        ndpi_free(cfgs[i].value);
    }

    for (i = 0; i < fargc; i++) {
        ndpi_free(fargv[i]);
    }

#ifdef CUSTOM_NDPI_PROTOCOLS
#include "../../nDPI-custom/ndpiReader_term.c"
#endif

#ifdef DEBUG_TRACE
    if (trace) fclose(trace);
#endif

    destroy_plugin_globals();

    // stats where/who/why/how much memory allocates. Expect this to return only init values
#if defined(_DEBUG) && defined(_STAT_MEM_ALLOCS)
    print_memory_allocation_stats();
#endif

    // destroy global resources
    destroy_mysql_interface();
    destroy_pool_manager();
    destroy_log_manager();
    destroy_ini_file();

    return ret_code;
}
