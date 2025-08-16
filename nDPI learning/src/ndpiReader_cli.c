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

#ifdef __linux__
char* bind_mask = NULL;
#endif

void parseOptions(int argc, char** argv);
void ndpiCheckIPMatch(char* testChar);

u_int8_t human_readeable_string_len = 5;
bool do_load_lists = false;

/* ********************************** */

struct ndpi_proto_sorter {
    int id;
    char name[32];
};

/* ********************************** */

int cmpProto(const void* _a, const void* _b) {
    struct ndpi_proto_sorter* a = (struct ndpi_proto_sorter*)_a;
    struct ndpi_proto_sorter* b = (struct ndpi_proto_sorter*)_b;

    return(strcmp(a->name, b->name));
}

/* ********************************** */

void extcap_interfaces() {
    printf("extcap {version=%s}{help=https://github.com/ntop/nDPI/tree/dev/wireshark}\n", ndpi_revision());
    printf("interface {value=ndpi}{display=nDPI interface}\n");

    extcap_exit = 1;
}

/* ********************************** */

void extcap_dlts() {
    u_int dlts_number = DLT_EN10MB;

    printf("dlt {number=%u}{name=%s}{display=%s}\n", dlts_number, "ndpi", "nDPI Interface");
    extcap_exit = 1;
}

/* *********************************************** */

void ndpiCheckHostStringMatch(char* testChar) {
    ndpi_protocol_match_result match = { NDPI_PROTOCOL_UNKNOWN,
                         NDPI_PROTOCOL_CATEGORY_UNSPECIFIED, NDPI_PROTOCOL_UNRATED };
    int  testRes;
    char appBufStr[64];
    ndpi_protocol detected_protocol;
    struct ndpi_detection_module_struct* ndpi_str;
    NDPI_PROTOCOL_BITMASK all;

    if (!testChar)
        return;

    ndpi_str = ndpi_init_detection_module(NULL);
    NDPI_BITMASK_SET_ALL(all);
    ndpi_set_protocol_detection_bitmask2(ndpi_str, &all);
    ndpi_finalize_initialization(ndpi_str);

    testRes = ndpi_match_string_subprotocol(ndpi_str,
        testChar, strlen(testChar), &match);

    if (testRes) {
        memset(&detected_protocol, 0, sizeof(ndpi_protocol));

        detected_protocol.proto.app_protocol = match.protocol_id;
        detected_protocol.proto.master_protocol = 0;
        detected_protocol.category = match.protocol_category;

        ndpi_protocol2name(ndpi_str, detected_protocol, appBufStr,
            sizeof(appBufStr));

        printf("Match Found for string [%s] -> P(%d) B(%d) C(%d) => %s %s %s\n",
            testChar, match.protocol_id, match.protocol_breed,
            match.protocol_category,
            appBufStr,
            ndpi_get_proto_breed_name(match.protocol_breed),
            ndpi_category_get_name(ndpi_str, match.protocol_category));
    }
    else
        printf("Match NOT Found for string: %s\n\n", testChar);

    ndpi_exit_detection_module(ndpi_str);
}

/* *********************************************** */

char const* ndpi_cfg_error2string(ndpi_cfg_error const err)
{
    switch (err)
    {
    case NDPI_CFG_INVALID_CONTEXT:
        return "Invalid context";
    case NDPI_CFG_NOT_FOUND:
        return "Configuration not found";
    case NDPI_CFG_INVALID_PARAM:
        return "Invalid configuration parameter";
    case NDPI_CFG_CONTEXT_ALREADY_INITIALIZED:
        return "Configuration context already initialized";
    case NDPI_CFG_CALLBACK_ERROR:
        return "Configuration callback error";
    case NDPI_CFG_OK:
        return "Success";
    }

    return "Unknown";
}

void ndpiCheckIPMatch(char* testChar) {
    struct ndpi_detection_module_struct* ndpi_str;
    u_int16_t ret = NDPI_PROTOCOL_UNKNOWN;
    u_int16_t port = 0;
    char* saveptr, * ip_str, * port_str;
    struct in_addr addr;
    char appBufStr[64];
    ndpi_protocol detected_protocol;
    int i;
    ndpi_cfg_error rc;
    NDPI_PROTOCOL_BITMASK all;

    if (!testChar)
        return;

    ndpi_str = ndpi_init_detection_module(NULL);
    NDPI_BITMASK_SET_ALL(all);
    ndpi_set_protocol_detection_bitmask2(ndpi_str, &all);

    if (_protoFilePath != NULL)
        ndpi_load_protocols_file(ndpi_str, _protoFilePath);

    for (i = 0; i < num_cfgs; i++) {
        rc = ndpi_set_config(ndpi_str, cfgs[i].proto, cfgs[i].param, cfgs[i].value);

        if (rc != NDPI_CFG_OK) {
            fprintf(stderr, "Error setting config [%s][%s][%s]: %s (%d)\n",
                (cfgs[i].proto != NULL ? cfgs[i].proto : ""),
                cfgs[i].param, cfgs[i].value, ndpi_cfg_error2string(rc), rc);
            exit(-1);
        }
    }

    ndpi_finalize_initialization(ndpi_str);

    ip_str = strtok_r(testChar, ":", &saveptr);
    if (!ip_str)
        return;

    addr.s_addr = inet_addr(ip_str);
    port_str = strtok_r(NULL, "\n", &saveptr);
    if (port_str)
        port = atoi(port_str);
    ret = ndpi_network_port_ptree_match(ndpi_str, &addr, htons(port));

    if (ret != NDPI_PROTOCOL_UNKNOWN) {
        memset(&detected_protocol, 0, sizeof(ndpi_protocol));
        detected_protocol.proto.app_protocol = ndpi_map_ndpi_id_to_user_proto_id(ndpi_str, ret);

        ndpi_protocol2name(ndpi_str, detected_protocol, appBufStr,
            sizeof(appBufStr));

        printf("Match Found for IP %s, port %d -> %s (%d)\n",
            ip_str, port, appBufStr, detected_protocol.proto.app_protocol);
    }
    else {
        printf("Match NOT Found for IP: %s\n", testChar);
    }

    ndpi_exit_detection_module(ndpi_str);
}

/**
 * @brief Print help instructions
 */
static void help(u_int long_help) {
    printf("Welcome to nDPI %s\n\n", ndpi_revision());

    printf("ndpiReader "
#ifndef USE_DPDK
        "-i <file|device> "
#endif
        "[-f <filter>][-s <duration>][-m <duration>][-b <num bin clusters>]\n"
        "          [-p <protos>][-l <loops> [-q][-d][-h][-H][-D][-e <len>][-E <path>][-t][-v <level>]\n"
        "          [-n <threads>][-N <path>][-w <file>][-c <file>][-C <file>][-j <file>][-x <file>]\n"
        "          [-r <file>][-R][-j <file>][-S <file>][-T <num>][-U <num>] [-x <domain>]\n"
        "          [-a <mode>][-B proto_list][-L <domain suffixes>][-j <app id>]\n\n"
        "Usage:\n"
        "  -i <file.pcap|device>     | Specify a pcap file/playlist to read packets from or a\n"
        "                            | device for live capture (comma-separated list)\n"
        "  -f <BPF filter>           | Specify a BPF filter for filtering selected traffic\n"
        "  -s <duration>             | Maximum capture duration in seconds (live traffic capture only)\n"
        "  -m <duration>             | Split analysis duration in <duration> max seconds\n"
        "  -p <file>.protos          | Specify a protocol file (eg. protos.txt)\n"
        "  -l <num loops>            | Number of detection loops (test only)\n"
        "  -L <domain suffixes>      | Domain suffixes (e.g. ../lists/public_suffix_list.dat)\n"
        "  -n <num threads>          | Number of threads. Default: number of interfaces in -i.\n"
        "                            | Ignored with pcap files.\n"
        "  -N <path>                 | Address cache dump/restore pathxo.\n"
        "  -b <num bin clusters>     | Number of bin clusters\n"
        "  -k <file>                 | Specify a file to write serialized detection results\n"
        "  -K <format>               | Specify the serialization format for `-k'\n"
        "                            | Valid formats are tlv, csv or json (default)\n"
#ifdef __linux__
        "  -g <id:id...>             | Thread affinity mask (one core id per thread)\n"
#endif
        "  -a <mode>                 | Generates option values for GUIs\n"
        "                            | 0 - List known protocols\n"
        "                            | 1 - List known categories\n"
        "                            | 2 - List known risks\n"
        "  -d                        | Disable protocol guess (by ip and by port) and use only DPI.\n"
        "                            | It is a shortcut to --cfg=dpi.guess_on_giveup,0\n"
        "  -e <len>                  | Min human readeable string match len. Default %u\n"
        "  -q                        | Quiet mode\n"
        "  -F                        | Enable flow stats\n"
        "  -t                        | Dissect GTP/TZSP tunnels\n"
        "  -P <a>:<b>:<c>:<d>:<e>    | Enable payload analysis:\n"
        "                            | <a> = min pattern len to search\n"
        "                            | <b> = max pattern len to search\n"
        "                            | <c> = max num packets per flow\n"
        "                            | <d> = max packet payload dissection\n"
        "                            | <e> = max num reported payloads\n"
        "                            | Default: %u:%u:%u:%u:%u\n"
        "  -c <path>                 | Load custom categories from the specified file\n"
        "  -C <path>                 | Write output in CSV format on the specified file\n"
        "  -E <path>                 | Write flow fingerprints on the specified file\n"
        "  -r <path>                 | Load risky domain file\n"
        "  -R                        | Print detected realtime protocols\n"
        "  -j <path>                 | Load malicious JA4 fingeprints\n"
        "  -S <path>                 | Load malicious SSL certificate SHA1 fingerprints\n"
        "  -G <dir>                  | Bind domain names to categories loading files from <dir>\n"
        "  -w <path>                 | Write test output on the specified file. This is useful for\n"
        "                            | testing purposes in order to compare results across runs\n"
        "  -h                        | This help\n"
        "  -H                        | This help plus some information about supported protocols/risks\n"
        "  -v <1|2|3|4>              | Verbose 'unknown protocol' packet print.\n"
        "                            | 1 = verbose\n"
        "                            | 2 = very verbose\n"
        "                            | 3 = port stats\n"
        "                            | 4 = hash stats\n"
        "  -V <0-4>                  | nDPI logging level\n"
        "                            | 0 - error, 1 - trace, 2 - debug, 3 - extra debug\n"
        "                            | >3 - extra debug + log enabled for all protocols (i.e. '-u all')\n"
        "  -u all|proto|num[,...]    | Enable logging only for such protocol(s)\n"
        "                            | If this flag is present multiple times (directly, or via '-V'),\n"
        "                            | only the last instance will be considered\n"
        "  -B all|proto|num[,...]    | Disable such protocol(s). By defaul all protocols are enabled\n"
        "  -T <num>                  | Max number of TCP processed packets before giving up [default: %u]\n"
        "  -U <num>                  | Max number of UDP processed packets before giving up [default: %u]\n"
        "  -D                        | Enable DoH traffic analysis based on content (no DPI)\n"
        "  -x <domain>               | Check domain name [Test only]\n"
        "  -I                        | Ignore VLAN id for flow hash calculation\n"
        "  -A                        | Dump internal statistics (LRU caches / Patricia trees / Ahocarasick automas / ...\n"
        "  -M                        | Memory allocation stats on data-path (only by the library).\n"
        "                            | It works only on single-thread configuration\n"
        "  --openvp_heuristics       | Enable OpenVPN heuristics.\n"
        "                            | It is a shortcut to --cfg=openvpn,dpi.heuristics,0x01\n"
        "  --tls_heuristics          | Enable TLS heuristics.\n"
        "                            | It is a shortcut to --cfg=tls,dpi.heuristics,0x07\n"
        "  --cfg=proto,param,value   | Configure the specific attribute of this protocol\n"
        "  --dump-fpc-stats          | Print FPC statistics\n"
        "  -J                        | AppId to use for DB content generation\n"
        ,
        human_readeable_string_len,
        min_pattern_len, max_pattern_len, max_num_packets_per_flow, max_packet_payload_dissection,
        max_num_reported_top_payloads, max_num_tcp_dissected_pkts, max_num_udp_dissected_pkts);

    NDPI_PROTOCOL_BITMASK all;
    struct ndpi_detection_module_struct* ndpi_str = ndpi_init_detection_module(NULL);

    NDPI_BITMASK_SET_ALL(all);
    ndpi_set_protocol_detection_bitmask2(ndpi_str, &all);

    if (_protoFilePath != NULL)
        ndpi_load_protocols_file(ndpi_str, _protoFilePath);

    ndpi_finalize_initialization(ndpi_str);

    printf("\nProtocols configuration parameters:\n");
    ndpi_dump_config(ndpi_str, stdout);

#ifndef WIN32
    printf("\nExcap (wireshark) options:\n"
        "  --extcap-interfaces\n"
        "  --extcap-version\n"
        "  --extcap-dlts\n"
        "  --extcap-interface <name>\n"
        "  --extcap-config\n"
        "  --capture\n"
        "  --extcap-capture-filter <filter>\n"
        "  --fifo <path to file or pipe>\n"
        "  --ndpi-proto-filter <protocol>\n"
    );
#endif

    if (long_help) {
        printf("\n\n"
            "Size of nDPI Flow structure:      %u\n"
            "Size of nDPI Flow protocol union: %zu\n",
            ndpi_detection_get_sizeof_ndpi_flow_struct(),
            sizeof(((struct ndpi_flow_struct*)0)->protos));

        printf("\n\nnDPI supported protocols:\n");
        printf("%3s %8s %-22s %-10s %-8s %-12s %-18s %-31s %-31s \n",
            "Id", "Userd-id", "Protocol", "Layer_4", "Nw_Proto", "Breed", "Category", "Def UDP Port/s", "Def TCP Port/s");
        num_reader_threads = 1;

        ndpi_dump_protocols(ndpi_str, stdout);

        printf("\n\nnDPI supported risks:\n");
        ndpi_dump_risks_score(stdout);
    }

    ndpi_exit_detection_module(ndpi_str);

    exit(!long_help);
}


#define OPTLONG_VALUE_CFG		3000
#define OPTLONG_VALUE_OPENVPN_HEURISTICS	3001
#define OPTLONG_VALUE_TLS_HEURISTICS		3002
#define OPTLONG_VALUE_CONF                      3003
#define OPTLONG_VALUE_FPC_STATS                 3004

static struct option longopts[] = {
    /* mandatory extcap options */
    { "extcap-interfaces", no_argument, NULL, '0'},
    { "extcap-version", optional_argument, NULL, '1'},
    { "extcap-dlts", no_argument, NULL, '2'},
    { "extcap-interface", required_argument, NULL, '3'},
    { "extcap-config", no_argument, NULL, '4'},
    { "capture", no_argument, NULL, '5'},
    { "extcap-capture-filter", required_argument, NULL, '6'},
    { "fifo", required_argument, NULL, '7'},
    { "ndpi-proto-filter", required_argument, NULL, '9'},

    /* ndpiReader options */
    { "enable-protocol-guess", no_argument, NULL, 'd'},
    { "categories", required_argument, NULL, 'c'},
    { "csv-dump", required_argument, NULL, 'C'},
    { "interface", required_argument, NULL, 'i'},
    { "filter", required_argument, NULL, 'f'},
    { "flow-stats", required_argument, NULL, 'F'},
    { "cpu-bind", required_argument, NULL, 'g'},
    { "load-categories", required_argument, NULL, 'G'},
    { "loops", required_argument, NULL, 'l'},
    { "domain-suffixes", required_argument, NULL, 'L'},
    { "num-threads", required_argument, NULL, 'n'},
    { "address-cache-dump", required_argument, NULL, 'N'},
    { "ignore-vlanid", no_argument, NULL, 'I'},

    { "protos", required_argument, NULL, 'p'},
    { "capture-duration", required_argument, NULL, 's'},
    { "decode-tunnels", no_argument, NULL, 't'},
    { "revision", no_argument, NULL, 'r'},
    { "verbose", required_argument, NULL, 'v'},
    { "version", no_argument, NULL, 'r'},
    { "ndpi-log-level", required_argument, NULL, 'V'},
    { "dbg-proto", required_argument, NULL, 'u'},
    { "help", no_argument, NULL, 'h'},
    { "long-help", no_argument, NULL, 'H'},
    { "serialization-outfile", required_argument, NULL, 'k'},
    { "serialization-format", required_argument, NULL, 'K'},
    { "payload-analysis", required_argument, NULL, 'P'},
    { "result-path", required_argument, NULL, 'w'},
    { "quiet", no_argument, NULL, 'q'},

    { "cfg", required_argument, NULL, OPTLONG_VALUE_CFG},
    { "openvpn_heuristics", no_argument, NULL, OPTLONG_VALUE_OPENVPN_HEURISTICS},
    { "tls_heuristics", no_argument, NULL, OPTLONG_VALUE_TLS_HEURISTICS},
    { "conf", required_argument, NULL, OPTLONG_VALUE_CONF},
    { "dump-fpc-stats", no_argument, NULL, OPTLONG_VALUE_FPC_STATS},

    {0, 0, 0, 0}
};

static const char* longopts_short = "a:Ab:B:e:E:c:C:dDFf:g:G:i:Ij:J:k:K:S:hHp:pP:l:L:r:Rs:tu:v:V:n:rp:x:X:w:q0123:456:7:89:m:MN:T:U:";

/* ********************************** */

void extcap_config() {
    int argidx = 0;

    struct ndpi_proto_sorter* protos;
    u_int ndpi_num_supported_protocols;
    int i;
    ndpi_proto_defaults_t* proto_defaults;
    NDPI_PROTOCOL_BITMASK all;
    struct ndpi_detection_module_struct* ndpi_str = ndpi_init_detection_module(NULL);

    if (!ndpi_str) exit(0);

    NDPI_BITMASK_SET_ALL(all);
    ndpi_set_protocol_detection_bitmask2(ndpi_str, &all);

    ndpi_finalize_initialization(ndpi_str);

    ndpi_num_supported_protocols = ndpi_get_ndpi_num_supported_protocols(ndpi_str);
    proto_defaults = ndpi_get_proto_defaults(ndpi_str);

    /* -i <interface> */
    printf("arg {number=%d}{call=-i}{display=Capture Interface}{type=string}{group=Live Capture}"
        "{tooltip=The interface name}\n", argidx++);

    printf("arg {number=%d}{call=-i}{display=Pcap File to Analyze}{type=fileselect}{mustexist=true}{group=Pcap}"
        "{tooltip=The pcap file to analyze (if the interface is unspecified)}\n", argidx++);


    protos = (struct ndpi_proto_sorter*)ndpi_malloc(sizeof(struct ndpi_proto_sorter) * ndpi_num_supported_protocols);
    if (!protos) exit(0);

    printf("arg {number=%d}{call=--ndpi-proto-filter}{display=nDPI Protocol Filter}{type=selector}{group=Options}"
        "{tooltip=nDPI Protocol to be filtered}\n", argidx);

    printf("value {arg=%d}{value=%d}{display=%s}{default=true}\n", argidx, (u_int32_t)-1, "No nDPI filtering");

    for (i = 0; i < (int)ndpi_num_supported_protocols; i++) {
        protos[i].id = i;
        ndpi_snprintf(protos[i].name, sizeof(protos[i].name), "%s", proto_defaults[i].protoName);
    }

    qsort(protos, ndpi_num_supported_protocols, sizeof(struct ndpi_proto_sorter), cmpProto);

    for (i = 0; i < (int)ndpi_num_supported_protocols; i++)
        printf("value {arg=%d}{value=%d}{display=%s (%d)}{default=false}{enabled=true}\n", argidx, protos[i].id,
            protos[i].name, protos[i].id);

    ndpi_free(protos);
    argidx++;

    printf("arg {number=%d}{call=--openvpn_heuristics}{display=Enable Obfuscated OpenVPN heuristics}"
        "{tooltip=Enable Obfuscated OpenVPN heuristics}{type=boolflag}{group=Options}\n", argidx++);
    printf("arg {number=%d}{call=--tls_heuristics}{display=Enable Obfuscated TLS heuristics}"
        "{tooltip=Enable Obfuscated TLS heuristics}{type=boolflag}{group=Options}\n", argidx++);

    ndpi_exit_detection_module(ndpi_str);

    extcap_exit = 1;
}

/* ********************************** */

void printCSVHeader() {
    if (!csv_fp) return;

    fprintf(csv_fp, "#flow_id|protocol|first_seen|last_seen|duration|src_ip|src_port|dst_ip|dst_port|ndpi_proto_num|ndpi_proto|proto_by_ip|server_name_sni|");
    fprintf(csv_fp, "c_to_s_pkts|c_to_s_bytes|c_to_s_goodput_bytes|s_to_c_pkts|s_to_c_bytes|s_to_c_goodput_bytes|");
    fprintf(csv_fp, "data_ratio|str_data_ratio|c_to_s_goodput_ratio|s_to_c_goodput_ratio|");

    /* IAT (Inter Arrival Time) */
    fprintf(csv_fp, "iat_flow_min|iat_flow_avg|iat_flow_max|iat_flow_stddev|");
    fprintf(csv_fp, "iat_c_to_s_min|iat_c_to_s_avg|iat_c_to_s_max|iat_c_to_s_stddev|");
    fprintf(csv_fp, "iat_s_to_c_min|iat_s_to_c_avg|iat_s_to_c_max|iat_s_to_c_stddev|");

    /* Packet Length */
    fprintf(csv_fp, "pktlen_c_to_s_min|pktlen_c_to_s_avg|pktlen_c_to_s_max|pktlen_c_to_s_stddev|");
    fprintf(csv_fp, "pktlen_s_to_c_min|pktlen_s_to_c_avg|pktlen_s_to_c_max|pktlen_s_to_c_stddev|");

#ifdef TCP_FLAG_STATISTICS
    /* TCP flags */
    fprintf(csv_fp, "cwr|ece|urg|ack|psh|rst|syn|fin|");

    fprintf(csv_fp, "c_to_s_cwr|c_to_s_ece|c_to_s_urg|c_to_s_ack|c_to_s_psh|c_to_s_rst|c_to_s_syn|c_to_s_fin|");

    fprintf(csv_fp, "s_to_c_cwr|s_to_c_ece|s_to_c_urg|s_to_c_ack|s_to_c_psh|s_to_c_rst|s_to_c_syn|s_to_c_fin|");

    /* TCP window */
    fprintf(csv_fp, "c_to_s_init_win|s_to_c_init_win|");
#endif

    /* Flow info */
    fprintf(csv_fp, "server_info|");
    fprintf(csv_fp, "tls_version|quic_version|");
    fprintf(csv_fp, "ja3s|");
    fprintf(csv_fp, "advertised_alpns|negotiated_alpn|tls_supported_versions|");
#if 0
    fprintf(csv_fp, "tls_issuerDN|tls_subjectDN|");
#endif
    fprintf(csv_fp, "ssh_client_hassh|ssh_server_hassh|flow_info|plen_bins|http_user_agent");

    if (enable_flow_stats) {
        fprintf(csv_fp, "|byte_dist_mean|byte_dist_std|entropy|total_entropy");
    }

    fprintf(csv_fp, "\n");
}

static int parse_three_strings(char* param, char** s1, char** s2, char** s3)
{
    char* saveptr, * tmp_str, * s1_str, * s2_str = NULL, * s3_str;
    int num_commas;
    unsigned int i;

    tmp_str = ndpi_strdup(param);
    if (tmp_str) {

        /* First parameter might be missing */
        num_commas = 0;
        for (i = 0; i < strlen(tmp_str); i++) {
            if (tmp_str[i] == ',')
                num_commas++;
        }

        if (num_commas == 1) {
            s1_str = NULL;
            s2_str = strtok_r(tmp_str, ",", &saveptr);
        }
        else if (num_commas == 2) {
            s1_str = strtok_r(tmp_str, ",", &saveptr);
            if (s1_str) {
                s2_str = strtok_r(NULL, ",", &saveptr);
            }
        }
        else {
            ndpi_free(tmp_str);
            return -1;
        }

        if (s2_str) {
            s3_str = strtok_r(NULL, ",", &saveptr);
            if (s3_str) {
                *s1 = ndpi_strdup(s1_str);
                *s2 = ndpi_strdup(s2_str);
                *s3 = ndpi_strdup(s3_str);
                ndpi_free(tmp_str);
                if (!s1 || !s2 || !s3) {
                    ndpi_free(s1);
                    ndpi_free(s2);
                    ndpi_free(s3);
                    return -1;
                }
                return 0;
            }
        }
    }
    ndpi_free(tmp_str);
    return -1;
}


int reader_add_cfg(char* proto, char* param, char* value, int dup)
{
    if (num_cfgs >= MAX_NUM_CFGS) {
        printf("Too many parameter! [num:%d/%d]\n", num_cfgs, MAX_NUM_CFGS);
        return -1;
    }
    cfgs[num_cfgs].proto = dup ? ndpi_strdup(proto) : proto;
    cfgs[num_cfgs].param = dup ? ndpi_strdup(param) : param;
    cfgs[num_cfgs].value = dup ? ndpi_strdup(value) : value;
    num_cfgs++;
    return 0;
}

/* ********************************** */


static void parse_parameters(int argc, char** argv)
{
    int option_idx = 0;
    int opt;
    char* s1, * s2, * s3;

    while ((opt = getopt_long(argc, argv, longopts_short, longopts, &option_idx)) != EOF) {
#ifdef DEBUG_TRACE
        if (trace) fprintf(trace, " #### Handling option -%c [%s] #### \n", opt, optarg ? optarg : "");
#endif

        switch (opt) {
        case 'a':
            ndpi_generate_options(atoi(optarg), stdout);
            exit(0);

        case 'A':
            dump_internal_stats = 1;
            break;

        case 'b':
            if ((num_bin_clusters = atoi(optarg)) > 32)
                num_bin_clusters = 32;
            break;

        case 'd':
            if (reader_add_cfg(NULL, "dpi.guess_on_giveup", "0", 1) == 1) {
                printf("Invalid parameter [%s] [num:%d/%d]\n", optarg, num_cfgs, MAX_NUM_CFGS);
                exit(1);
            }
            break;

        case 'D':
            enable_doh_dot_detection = 1;
            break;

        case 'e':
            human_readeable_string_len = atoi(optarg);
            break;

        case 'E':
            errno = 0;
            if ((fingerprint_fp = fopen(optarg, "w")) == NULL) {
                printf("Unable to write on fingerprint file %s: %s\n", optarg, strerror(errno));
                exit(1);
            }

            if (reader_add_cfg("tls", "metadata.ja4r_fingerprint", "1", 1) == -1) {
                printf("Unable to enable JA4r fingerprints\n");
                exit(1);
            }

            do_load_lists = true;
            break;

        case 'i':
        case '3':
            _pcap_file[0] = optarg;
            break;

        case 'I':
            ignore_vlanid = 1;
            break;

        case 'j':
            _maliciousJA4Path = optarg;
            break;

        case 'S':
            _maliciousSHA1Path = optarg;
            break;

        case 'm':
            pcap_analysis_duration = atol(optarg);
            break;

        case 'f':
        case '6':
            bpfFilter = optarg;
            break;

#ifndef USE_DPDK
#ifdef __linux__
        case 'g':
            bind_mask = optarg;
            break;
#endif
#endif

        case 'G':
            _categoriesDirPath = optarg;
            break;

        case 'l':
            num_loops = atoi(optarg);
            break;

        case 'L':
            _domain_suffixes = optarg;
            break;

        case 'n':
            num_worker_threads = num_reader_threads = atoi(optarg);
#ifdef _APP_MODE_BUILD_DB
            num_reader_threads = 1;
#endif
            break;

        case 'N':
            addr_dump_path = optarg;
            break;

        case 'p':
            _protoFilePath = optarg;
            break;

        case 'c':
            _customCategoryFilePath = optarg;
            break;

        case 'C':
            errno = 0;
            if ((csv_fp = fopen(optarg, "w")) == NULL)
            {
                printf("Unable to write on CSV file %s: %s\n", optarg, strerror(errno));
                exit(1);
            }
            break;

        case 'r':
            _riskyDomainFilePath = optarg;
            break;

        case 'R':
            enable_realtime_output = 1;
            break;

        case 's':
            capture_for = atoi(optarg);
            capture_until = capture_for + time(NULL);
            break;

        case 't':
            decode_tunnels = 1;
            break;

        case 'v':
            verbose = atoi(optarg);
            break;

        case 'V':
        {
            char buf[12];
            int log_level;
            const char* errstrp;

            /* (Internals) log levels are 0-3, but ndpiReader allows 0-4, where with 4
               we also enable all protocols */
            log_level = ndpi_strtonum(optarg, NDPI_LOG_ERROR, NDPI_LOG_DEBUG_EXTRA + 1, &errstrp, 10);
            if (errstrp != NULL) {
                printf("Invalid log level %s: %s\n", optarg, errstrp);
                exit(1);
            }
            if (log_level > NDPI_LOG_DEBUG_EXTRA) {
                log_level = NDPI_LOG_DEBUG_EXTRA;
                if (reader_add_cfg("all", "log", "enable", 1) == 1) {
                    printf("Invalid cfg [num:%d/%d]\n", num_cfgs, MAX_NUM_CFGS);
                    exit(1);
                }
            }
            snprintf(buf, sizeof(buf), "%d", log_level);
            if (reader_add_cfg(NULL, "log.level", buf, 1) == 1) {
                printf("Invalid log level [%s] [num:%d/%d]\n", buf, num_cfgs, MAX_NUM_CFGS);
                exit(1);
            }
            reader_log_level = log_level;
            break;
        }

        case 'u':
        {
            char* n;
            char* str = ndpi_strdup(optarg);
            int inverted_logic;

            /* Reset any previous call to this knob */
            if (reader_add_cfg("all", "log", "disable", 1) == 1) {
                printf("Invalid cfg [num:%d/%d]\n", num_cfgs, MAX_NUM_CFGS);
                exit(1);
            }

            for (n = strtok(str, ","); n && *n; n = strtok(NULL, ",")) {
                inverted_logic = 0;
                if (*n == '-') {
                    inverted_logic = 1;
                    n++;
                }
                if (reader_add_cfg(n, "log", inverted_logic ? "disable" : "enable", 1) == 1) {
                    printf("Invalid parameter [%s] [num:%d/%d]\n", n, num_cfgs, MAX_NUM_CFGS);
                    exit(1);
                }
            }
            ndpi_free(str);
            break;
        }

        case 'B':
            ndpi_free(_disabled_protocols);
            _disabled_protocols = ndpi_strdup(optarg);
            break;

        case 'h':
            help(0);
            break;

        case 'H':
            help(1);
            break;

        case 'F':
            enable_flow_stats = 1;
            break;

        case 'P':
        {
            int _min_pattern_len, _max_pattern_len,
                _max_num_packets_per_flow, _max_packet_payload_dissection,
                _max_num_reported_top_payloads;

            enable_payload_analyzer = 1;
            if (sscanf(optarg, "%d:%d:%d:%d:%d", &_min_pattern_len, &_max_pattern_len,
                &_max_num_packets_per_flow,
                &_max_packet_payload_dissection,
                &_max_num_reported_top_payloads) == 5) {
                min_pattern_len = _min_pattern_len, max_pattern_len = _max_pattern_len;
                max_num_packets_per_flow = _max_num_packets_per_flow, max_packet_payload_dissection = _max_packet_payload_dissection;
                max_num_reported_top_payloads = _max_num_reported_top_payloads;
                if (min_pattern_len > max_pattern_len) min_pattern_len = max_pattern_len;
                if (min_pattern_len < 2)               min_pattern_len = 2;
                if (max_pattern_len > 16)              max_pattern_len = 16;
                if (max_num_packets_per_flow == 0)     max_num_packets_per_flow = 1;
                if (max_packet_payload_dissection < 4) max_packet_payload_dissection = 4;
                if (max_num_reported_top_payloads == 0) max_num_reported_top_payloads = 1;
            }
            else {
                printf("Invalid -P format. Ignored\n");
                help(0);
            }
        }
        break;

        case 'M':
            enable_malloc_bins = 1;
            ndpi_init_bin(&malloc_bins, ndpi_bin_family64, max_malloc_bins);
            break;

        case 'k':
            errno = 0;
            if ((serialization_fp = fopen(optarg, "w")) == NULL)
            {
                printf("Unable to write on serialization file %s: %s\n", optarg, strerror(errno));
                exit(1);
            }
            break;

        case 'K':
            if (strcasecmp(optarg, "tlv") == 0 && strlen(optarg) == 3)
            {
                serialization_format = ndpi_serialization_format_tlv;
            }
            else if (strcasecmp(optarg, "csv") == 0 && strlen(optarg) == 3)
            {
                serialization_format = ndpi_serialization_format_csv;
            }
            else if (strcasecmp(optarg, "json") == 0 && strlen(optarg) == 4)
            {
                serialization_format = ndpi_serialization_format_json;
            }
            else {
                printf("Unknown serialization format. Valid values are: tlv,csv,json\n");
                exit(1);
            }
            break;

        case 'w':
            results_path = ndpi_strdup(optarg);
            if ((results_file = fopen(results_path, "w")) == NULL) {
                printf("Unable to write in file %s: quitting\n", results_path);
                exit(1);
            }
            break;

        case 'q':
            quiet_mode = 1;
            if (reader_add_cfg(NULL, "log.level", "0", 1) == 1) {
                printf("Invalid cfg [num:%d/%d]\n", num_cfgs, MAX_NUM_CFGS);
                exit(1);
            }
            reader_log_level = 0;
            break;

        case OPTLONG_VALUE_OPENVPN_HEURISTICS:
            if (reader_add_cfg("openvpn", "dpi.heuristics", "0x01", 1) == 1) {
                printf("Invalid cfg [num:%d/%d]\n", num_cfgs, MAX_NUM_CFGS);
                exit(1);
            }
            break;

        case OPTLONG_VALUE_TLS_HEURISTICS:
            if (reader_add_cfg("tls", "dpi.heuristics", "0x07", 1) == 1) {
                printf("Invalid cfg [num:%d/%d]\n", num_cfgs, MAX_NUM_CFGS);
                exit(1);
            }
            break;

        case OPTLONG_VALUE_FPC_STATS:
            dump_fpc_stats = 1;
            break;

        case OPTLONG_VALUE_CONF:
        {
            FILE* fd;
            char buffer[512], * line, * saveptr;
            int len, saved_optind, initial_fargc;

            fd = fopen(optarg, "r");
            if (fd == NULL) {
                printf("Error opening: %s\n", optarg);
                exit(1);
            }

            if (fargc == 0) {
                fargv[0] = ndpi_strdup(argv[0]);
                fargc = 1;
            }
            initial_fargc = fargc;

            while (1) {
                line = fgets(buffer, sizeof(buffer), fd);

                if (line == NULL)
                    break;

                len = strlen(line);

                if ((len <= 1) || (line[0] == '#'))
                    continue;

                line[len - 1] = '\0';

                fargv[fargc] = ndpi_strdup(strtok_r(line, " \t", &saveptr));
                while (fargc < MAX_FARGS && fargv[fargc] != NULL) {
                    fargc++;
                    fargv[fargc] = ndpi_strdup(strtok_r(NULL, " \t", &saveptr));
                }
                if (fargc == MAX_FARGS) {
                    printf("Too many arguments\n");
                    exit(1);
                }
            }

            /* Recursive call to getopt_long() */
            saved_optind = optind;
            optind = initial_fargc;
            parse_parameters(fargc, fargv);
            optind = saved_optind;

            fclose(fd);
        }
        break;

        /* Extcap */
        case '0':
            extcap_interfaces();
            break;

        case '1':
            printf("extcap {version=%s}\n", ndpi_revision());
            break;

        case '2':
            extcap_dlts();
            break;

        case '4':
            extcap_config();
            break;

#ifndef USE_DPDK
        case '5':
            do_extcap_capture = 1;
            break;
#endif

        case '7':
            extcap_capture_fifo = ndpi_strdup(optarg);
            break;

        case '9':
        {
            struct ndpi_detection_module_struct* ndpi_str = ndpi_init_detection_module(NULL);
            NDPI_PROTOCOL_BITMASK all;

            NDPI_BITMASK_SET_ALL(all);
            ndpi_set_protocol_detection_bitmask2(ndpi_str, &all);
            ndpi_finalize_initialization(ndpi_str);

            extcap_packet_filter = ndpi_get_proto_by_name(ndpi_str, optarg);
            if (extcap_packet_filter == NDPI_PROTOCOL_UNKNOWN) extcap_packet_filter = atoi(optarg);

            ndpi_exit_detection_module(ndpi_str);
            break;
        }

        case 'T':
            max_num_tcp_dissected_pkts = atoi(optarg);
            /* If we enable that, allow at least 3WHS + 1 "real" packet */
            if (max_num_tcp_dissected_pkts != 0 && max_num_tcp_dissected_pkts < 4) max_num_tcp_dissected_pkts = 4;
            break;

        case 'x':
            domain_to_check = optarg;
            break;

        case 'X':
            ip_port_to_check = optarg;
            break;

        case 'U':
            max_num_udp_dissected_pkts = atoi(optarg);
            break;

        case OPTLONG_VALUE_CFG:
            if (parse_three_strings(optarg, &s1, &s2, &s3) == -1 ||
                reader_add_cfg(s1, s2, s3, 0) == -1) {
                printf("Invalid parameter [%s] [num:%d/%d]\n", optarg, num_cfgs, MAX_NUM_CFGS);
                exit(1);
            }
            break;

        case 'J':
            app_id_for_pcap = atoi(optarg);
            break;

        default:
#ifdef DEBUG_TRACE
            if (trace) fprintf(trace, " #### Unknown option -%c: skipping it #### \n", opt);
#endif

            help(0);
            break;
        }
    }
}

/**
 * @brief Option parser
 */
void parseOptions(int argc, char** argv) {
#ifndef USE_DPDK
    char* __pcap_file = NULL;
    int thread_id;
#ifdef __linux__
    u_int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
#endif
#endif

#ifdef USE_DPDK
    {
        int ret = rte_eal_init(argc, argv);

        if (ret < 0)
            rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

        argc -= ret, argv += ret;
    }
#endif

    parse_parameters(argc, argv);

    if (serialization_fp == NULL && serialization_format != ndpi_serialization_format_unknown)
    {
        printf("Serializing detection results to a file requires command line arguments `-k'\n");
        exit(1);
    }
    if (serialization_fp != NULL && serialization_format == ndpi_serialization_format_unknown)
    {
        serialization_format = ndpi_serialization_format_json;
    }

    if (extcap_exit)
        exit(0);

    printCSVHeader();

#ifndef USE_DPDK
    if (do_extcap_capture) {
        quiet_mode = 1;
    }

    if (!domain_to_check && !ip_port_to_check) {
        if (_pcap_file[0] == NULL)
            help(0);

        if (strchr(_pcap_file[0], ',')) { /* multiple ingress interfaces */
            num_reader_threads = 0;               /* setting number of threads = number of interfaces */
            __pcap_file = strtok(_pcap_file[0], ",");
            while (__pcap_file != NULL && num_reader_threads < MAX_NUM_READER_THREADS) {
                _pcap_file[num_reader_threads++] = __pcap_file;
                __pcap_file = strtok(NULL, ",");
            }
        }
        else {
            if (num_reader_threads > MAX_NUM_READER_THREADS) num_reader_threads = MAX_NUM_READER_THREADS;
            for (thread_id = 1; thread_id < num_reader_threads; thread_id++)
                _pcap_file[thread_id] = _pcap_file[0];
        }

        if (num_reader_threads > 1 && enable_malloc_bins == 1)
        {
            printf("Memory profiling ('-M') is incompatible with multi-thread enviroment");
            exit(1);
        }
    }

#ifdef __linux__
#ifndef USE_DPDK
    if (num_worker_threads > MAX_NUM_WORKER_THREADS) {
        num_worker_threads = MAX_NUM_WORKER_THREADS;
    }
    printf("Using libpcap as capturer. Using %d reader threads and %d processing threads\n", num_reader_threads, num_worker_threads);
    for (thread_id = 0; thread_id < num_reader_threads + num_worker_threads; thread_id++)
        core_affinity[thread_id] = -1;

    if (num_cores > 1 && bind_mask != NULL) {
        char* core_id = strtok(bind_mask, ":");
        thread_id = 0;

        while (core_id != NULL && thread_id < num_reader_threads + num_worker_threads) {
            core_affinity[thread_id++] = atoi(core_id) % num_cores;
            core_id = strtok(NULL, ":");
        }
    }
#endif
#endif
#endif
}

/* *********************************************** */

/**
 * @brief Initialize port array
 */

void bpf_filter_port_array_init(int array[], int size) {
    int i;
    for (i = 0; i < size; i++)
        array[i] = INIT_VAL;
}

/* *********************************************** */
/**
 * @brief Initialize host array
 */

void bpf_filter_host_array_init(const char* array[48], int size) {
    int i;
    for (i = 0; i < size; i++)
        array[i] = NULL;
}

/* *********************************************** */

/**
 * @brief Add host to host filter array
 */

void bpf_filter_host_array_add(const char* filter_array[48], int size, const char* host) {
    int i;
    int r;
    for (i = 0; i < size; i++) {
        if ((filter_array[i] != NULL) && (r = strcmp(filter_array[i], host)) == 0)
            return;
        if (filter_array[i] == NULL) {
            filter_array[i] = host;
            return;
        }
    }
    fprintf(stderr, "bpf_filter_host_array_add: max array size is reached!\n");
    exit(-1);
}


/* *********************************************** */

/**
 * @brief Add port to port filter array
 */

void bpf_filter_port_array_add(int filter_array[], int size, int port) {
    int i;
    for (i = 0; i < size; i++) {
        if (filter_array[i] == port)
            return;
        if (filter_array[i] == INIT_VAL) {
            filter_array[i] = port;
            return;
        }
    }
    fprintf(stderr, "bpf_filter_port_array_add: max array size is reached!\n");
    exit(-1);
}

#ifdef _MSC_BUILD
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONIN$", "r", stdin);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }

    return main(__argc, __argv);
}
#endif

#if defined(WIN32) && !defined(_MSC_BUILD)
#ifndef __GNUC__
#define EPOCHFILETIME (116444736000000000i64)
#else
#define EPOCHFILETIME (116444736000000000LL)
#endif

/**
   @brief Timezone
**/
#ifndef __GNUC__
struct timezone {
    int tz_minuteswest; /* minutes W of Greenwich */
    int tz_dsttime;     /* type of dst correction */
};
#endif

/**
   @brief Set time
**/
int gettimeofday(struct timeval* tv, struct timezone* tz) {
    FILETIME        ft;
    LARGE_INTEGER   li;
    __int64         t;
    static int      tzflag;

    if (tv) {
        GetSystemTimeAsFileTime(&ft);
        li.LowPart = ft.dwLowDateTime;
        li.HighPart = ft.dwHighDateTime;
        t = li.QuadPart;       /* In 100-nanosecond intervals */
        t -= EPOCHFILETIME;     /* Offset to the Epoch time */
        t /= 10;                /* In microseconds */
        tv->tv_sec = (long)(t / 1000000);
        tv->tv_usec = (long)(t % 1000000);
    }

    if (tz) {
        if (!tzflag) {
            _tzset();
            tzflag++;
        }

        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }

    return 0;
}
#endif /* WIN32 */
