#ifndef _PLUGIN_GLOBALS_H_
#define _PLUGIN_GLOBALS_H_

extern double g_StopSearchTreshold;

typedef enum {
    PT_INVALID,
    PT_JA3S,
    PT_JA4,
    PT_SHA1_CERT,
    PT_IP_SERVER,
    PT_SNI,
    PT_MAX_VAUE
} PluginTypes;

typedef struct PluginName {
    PluginTypes value;
    const char* name;
} PluginName;

extern PluginName plugin_table[PT_MAX_VAUE+1];

typedef struct nDPI_pkt_parser_params nDPI_pkt_parser_params;

/*
* Init global variables used by most plugins
*/
void init_plugin_globals();
/*
* Deallocate memory used for global settings
*/
void destroy_plugin_globals();
/*
* When a flow should be investigated to detect / report 
*/
void check_flow_app_assoc(nDPI_pkt_parser_params *params);
/*
* When a plugin detects a flow belonging to a specific application the plugin will queue it's results to the flow
* Once app detection cycle is over, flow can queue the results to reporting
*/
void queue_app_detected_result(const nDPI_pkt_parser_params *p_pktparams, const PluginTypes plugin_type, const uint16_t application_id, const double confidence);
/*
* Will be called once the flow expires or when a strong enough confidence detection happened
*/
void push_results_to_reporting(struct ndpi_flow_info* flow);
#endif