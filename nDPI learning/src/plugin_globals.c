#include <stdint.h>
#include "ini_file_handler.h"
#include "ndpi_types.h"
#include "reader_util.h"
#include "ndpiReader.h"
#include "log_manager.h"
#include "plugin_globals.h"
#include "buffer_pool.h"
#include "plugin_ja3.h"
#include "plugin_ja4.h"
#include "plugin_SHA1.h"
#include "plugin_serv_ip.h"
#include "plugin_SNI.h"

double g_StopSearchTreshold = 1.0;

PluginName plugin_table[PT_MAX_VAUE+1] = {
	{ PT_INVALID,   "invalid" },
	{ PT_JA3S, "JA3S" },
	{ PT_JA4,  "JA4" },
	{ PT_SHA1_CERT,  "SHA1_certificates" },
	{ PT_IP_SERVER,  "IP_server" },
	{ PT_SNI,  "Server_name_identifier" },
	{ PT_MAX_VAUE, NULL }
};

#define MAX_EXPECTED_APP_ID_RESULTS 20
typedef struct app_detection_result_store {
	PluginTypes plugin_type;
	uint16_t app_id;
	double confidence;
}app_detection_result_store;
typedef struct fsa_plugin_results {
	uint32_t size_used;
	uint32_t size_allocated;
	app_detection_result_store results[0];
}fsa_plugin_results;

void init_plugin_globals()
{
	g_StopSearchTreshold = get_ini_double_value("Plugins", "ConfidenceTreshold", 1.0);
	if (g_StopSearchTreshold < 0) {
		g_StopSearchTreshold = 0.0;
	}
	if (g_StopSearchTreshold > 1) {
		g_StopSearchTreshold = 1.0;
	}

	init_packetp_JA3();
	init_packetp_JA4();
	init_packetp_SHA1();
	init_packetp_SERVER_IP();
	init_packetp_SNI();
}

void destroy_plugin_globals()
{
	destroy_packetp_JA3();
	destroy_packetp_JA4();
	destroy_packetp_SHA1();
	destroy_packetp_SERVER_IP();
	destroy_packetp_SNI();
}

void check_flow_app_assoc(nDPI_pkt_parser_params* plugin_params)
{
	// already processed. Careless call :P
	if (plugin_params->flow_to_process == NULL || plugin_params->flow_to_process->app_is_identified != 0) {
		return;
	}

	// list of plugins that can identify a flow
	process_packet_JA3(plugin_params);
	process_packet_JA4(plugin_params);
	process_packet_SHA1(plugin_params);
	process_packet_SERVER_IP(plugin_params);
	process_packet_SNI(plugin_params);

	// identification status changed. Report as soon as possible ?
	// Todo : periodically re report it ? Ex : bytes used 
	if (plugin_params->flow_to_process->app_is_identified == 1) {
		// should have reported as soon as possible
		push_results_to_reporting(plugin_params->flow_to_process);
	}
}

void queue_app_detected_result(const nDPI_pkt_parser_params* p_pktparams, const PluginTypes plugin_type, const uint16_t application_id, const double confidence) {
	// first time we detect results for this flow ?
	if (p_pktparams->flow_to_process->buff_pool == NULL) {
		p_pktparams->flow_to_process->buff_pool = pool_manager_pop_st(p_pktparams->flow_to_process->workflow->buff_pool_mgr);
	}
	// unexpected
	if (p_pktparams->flow_to_process->buff_pool == NULL) {
		AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourcePluginGlobal, "Failed to create buffer pool. Out of memory ?");
		return;
	}
	// first time push results for the flow ?
	if (p_pktparams->flow_to_process->detection_results == NULL) {
		uint32_t bytes_to_allocate = sizeof(fsa_plugin_results) + sizeof(app_detection_result_store) * MAX_EXPECTED_APP_ID_RESULTS;
		p_pktparams->flow_to_process->detection_results = alloc_pooled(p_pktparams->flow_to_process->buff_pool, bytes_to_allocate);
		if (p_pktparams->flow_to_process->detection_results == NULL) {
			AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourcePluginGlobal, "Failed to create buffer pool. Out of memory ?");
			return;
		}
		p_pktparams->flow_to_process->detection_results->size_allocated = MAX_EXPECTED_APP_ID_RESULTS;
		p_pktparams->flow_to_process->detection_results->size_used = 0;
	}
	// full of results, need more slots to store additional results
	if (p_pktparams->flow_to_process->detection_results->size_used == p_pktparams->flow_to_process->detection_results->size_allocated) {
		uint32_t bytes_to_allocate = sizeof(fsa_plugin_results) + sizeof(app_detection_result_store) * (p_pktparams->flow_to_process->detection_results->size_used + MAX_EXPECTED_APP_ID_RESULTS);
		struct fsa_plugin_results *new_store = alloc_pooled(p_pktparams->flow_to_process->buff_pool, bytes_to_allocate);
		if (new_store == NULL) {
			AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourcePluginGlobal, "Failed to create buffer pool. Out of memory ?");
			return;
		}
		uint32_t bytes_before = sizeof(fsa_plugin_results) + sizeof(app_detection_result_store) * p_pktparams->flow_to_process->detection_results->size_used;
		memcpy(new_store, p_pktparams->flow_to_process->detection_results, bytes_before);
		// buffer pooled allocations do not need to be deallocated 1 by 1, at the end of the flow they get thrown out all
		p_pktparams->flow_to_process->detection_results = new_store;
		p_pktparams->flow_to_process->detection_results->size_allocated += MAX_EXPECTED_APP_ID_RESULTS;
	}
	uint32_t store_index = p_pktparams->flow_to_process->detection_results->size_used;
	p_pktparams->flow_to_process->detection_results->size_used++;
	p_pktparams->flow_to_process->detection_results->results[store_index].plugin_type = plugin_type;
	p_pktparams->flow_to_process->detection_results->results[store_index].app_id = application_id;
	p_pktparams->flow_to_process->detection_results->results[store_index].confidence = confidence;
}

void push_results_to_reporting(struct ndpi_flow_info* flow) {
	// should not happen ?
	if (flow->app_reported) {
		return;
	}
	// mark it as reported so we do not report it a second time
	flow->app_reported = 1;
	if (flow->detection_results == NULL) {
		AddLogEntryB(LDF_LOCAL, LogSeverityDebug, LogSourcePluginGlobal, "There are no results to be reported for flow\n");
		return;
	}
	// should get buffer from reporting module so we can push our results there : flow data + results data
//	printf("Flow %d has %d app results\n", flow->flow_id, flow->detection_results->size_used);
	for (size_t i = 0; i < flow->detection_results->size_used; i++) {
		printf("Flow %d has been identified by pt=%s as app %d with confidence=%f\n", 
			flow->flow_id, plugin_table[flow->detection_results->results[i].plugin_type].name, flow->detection_results->results[i].app_id, flow->detection_results->results[i].confidence);
	}
}