#include <mysql/mysql.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "ndpi_types.h"
#include "reader_util.h"
#include "plugin_serv_ip.h"
#include "log_manager.h"
#include "uthash.h"
#include "mysql_interface.h"
#include <arpa/inet.h>
#include "plugin_globals.h"
#include "utils.h"
#include "ini_file_handler.h"

extern int app_id_for_pcap;
extern char* _pcap_file[MAX_NUM_READER_THREADS];

#define S_IP_BYTE_COUNT sizeof(db_row_key)
#define S_IP_MAX_APPIDS_TRACKED	20 // could make it dynamic

typedef struct app_id_confidence {
	uint16_t application_id;
	uint16_t occurance_count;
	double confidence;
}app_id_confidence;

typedef struct db_row_key {
	union {
		uint32_t v4;
		struct ndpi_in6_addr v6;
	} server_ip;
}db_row_key;

typedef struct plugin_DB_row {
	db_row_key row_key; // binary hash. Put it at the start of the struct to ensure allignment
	app_id_confidence apps[S_IP_MAX_APPIDS_TRACKED];
	uint16_t total_occurance_count; // number of applications are using this. Loaded from DB, not tracked while parsing pcap
#ifdef _APP_MODE_BUILD_DB
	uint8_t updated_ocurrences; // happens only when ingesting a pcap for a specific app. Will not happen for more than 1 app
	uint32_t row_id;
#endif
	UT_hash_handle hh;
}plugin_DB_row;

// make a habbit of calling this function even if it seems useless
// consider it safe coding
static void init_plugin_DB_row_S_IP(plugin_DB_row* this) {
	// !! hashing might fail if you happen to use a hash key that has bytes uninitialized !!
	// !! migh happen on dynamic length strings or padded structure fields !!
	memset(this, 0, sizeof(plugin_DB_row));
}

// this is plugin specific. We will only read it when using live traffic.
static plugin_DB_row* g_S_IP_db_cache = NULL;
#ifdef _APP_MODE_BUILD_DB
extern int g_app_table_updated;
#endif
static size_t g_is_plugin_disabled = 0;

static void init_plugin_mysql_table() {
	const char* create_S_IP_fingerprints_table_sql =
		"CREATE TABLE IF NOT EXISTS serv_ip_fingerprints (\n"
		"    id INT UNSIGNED NOT NULL AUTO_INCREMENT,\n"
		"    serv_ip CHAR(49) NOT NULL,\n"
		"    occurrences INT UNSIGNED NOT NULL DEFAULT 1,\n"
		"    application_id INT UNSIGNED DEFAULT NULL,\n"
		"    sources TEXT NOT NULL,\n"
		"    PRIMARY KEY (id),\n"
		"    CONSTRAINT fk_application_S_IP FOREIGN KEY (application_id)\n"
		"        REFERENCES applications(id)\n"
		"        ON DELETE SET NULL\n"
		"        ON UPDATE CASCADE\n"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;";

	if (get_mysql_conn() && mysql_query(get_mysql_conn(), create_S_IP_fingerprints_table_sql)) {
		AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n",
			create_S_IP_fingerprints_table_sql, mysql_error(get_mysql_conn()));
	}
}

static int get_app_id_slot_S_IP(plugin_DB_row* existing_entry, uint16_t app_id) {
	if (existing_entry == NULL) {
		return 0;
	}
	for (size_t i = 0; i < S_IP_MAX_APPIDS_TRACKED; i++) {
		if (existing_entry->apps[i].application_id == app_id) {
			return i;
		}
	}
	if (app_id > 0) {
		AddLogEntryB(LDF_LOCAL, LogSeverityWarn, LogSourcePluginS_IP, "S_IP : All app_id slots are taken. Increase S_IP_MAX_APPIDS_TRACKED\n");
	}
	return -1;
}

#ifdef _APP_MODE_BUILD_DB
static void on_new_entry_found_db_gen(const uint32_t ip_version, const db_row_key* row_key) {
	// do we have this value ?
	plugin_DB_row* existing_entry;
	HASH_FIND(hh, g_S_IP_db_cache, row_key, S_IP_BYTE_COUNT, existing_entry);
	int existing_app_index = get_app_id_slot_S_IP(existing_entry, app_id_for_pcap);
	// should only increase counter once per pcap file
	if (existing_entry != NULL && existing_app_index >= 0) {
		if (existing_entry->updated_ocurrences == 0 && existing_entry->row_id) {
			existing_entry->updated_ocurrences = 1;
			char update_sql[2000];
			snprintf(update_sql, sizeof(update_sql), "update serv_ip_fingerprints set occurrences = occurrences + 1 where id = %d", existing_entry->row_id);
			if (get_mysql_conn() && mysql_query(get_mysql_conn(), update_sql)) {
				AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", update_sql, mysql_error(get_mysql_conn()));
			}
			snprintf(update_sql, sizeof(update_sql), "update serv_ip_fingerprints set sources = CONCAT(sources, '%s,') where id = %d and sources NOT LIKE '%%%s,%%'",
				_pcap_file[0], existing_entry->row_id, _pcap_file[0]);
			if (get_mysql_conn() && mysql_query(get_mysql_conn(), update_sql)) {
				AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", update_sql, mysql_error(get_mysql_conn()));
			}
		}
	}
	plugin_DB_row* new_entry = NULL;
	// hash exists in DB, but not for this specific app
	if (existing_entry != NULL && existing_app_index == -1) {
		existing_app_index = get_app_id_slot_S_IP(existing_entry, 0);
		if (existing_app_index >= 0) {
			existing_entry->apps[existing_app_index].application_id = app_id_for_pcap;
			existing_entry->apps[existing_app_index].occurance_count = 1;
			existing_entry->total_occurance_count += 1;
			existing_entry->updated_ocurrences = 1;
		}
		new_entry = existing_entry;
	}
	// hash does not exist in DB for any apps
	if (existing_entry == NULL) {
		existing_app_index = 0;
		new_entry = (plugin_DB_row*)malloc(sizeof(plugin_DB_row));
		init_plugin_DB_row_S_IP(new_entry);
		memcpy(&new_entry->row_key, row_key, S_IP_BYTE_COUNT);
		new_entry->updated_ocurrences = 1;
		new_entry->apps[existing_app_index].application_id = app_id_for_pcap;
		HASH_ADD(hh, g_S_IP_db_cache, row_key, S_IP_BYTE_COUNT, new_entry);
	}
	// if the entry is new, should save it to DB
	if (new_entry != NULL && get_mysql_conn()) {
		char update_sql[2000];
		if (g_app_table_updated == 0) {
			g_app_table_updated = 1;
			snprintf(update_sql, sizeof(update_sql), "update applications set pcaps_parsed = pcaps_parsed + 1, pcap_names = CONCAT(pcap_names, '%s,') where id=%d",
				_pcap_file[0], new_entry->apps[existing_app_index].application_id);
			if (mysql_query(get_mysql_conn(), update_sql)) {
				AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", update_sql, mysql_error(get_mysql_conn()));
			}
		}
		// convert binary IP to string IP to be saved to DB
		char strIP[INET6_ADDRSTRLEN];
		strIP[0] = '\0';
		if (ip_version == 4) {
			inet_ntop(AF_INET, &row_key->server_ip.v4, strIP, sizeof(strIP));
		}
		else if (ip_version == 6) {
			inet_ntop(AF_INET6, &row_key->server_ip.v6, strIP, sizeof(strIP));
		}
		snprintf(update_sql, sizeof(update_sql), "insert into serv_ip_fingerprints "
			"(serv_ip,application_id,sources) values ('%s',%d,'%s,')", strIP, new_entry->apps[existing_app_index].application_id, _pcap_file[0]);
		if (mysql_query(get_mysql_conn(), update_sql)) {
			AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", update_sql, mysql_error(get_mysql_conn()));
		}
		else {
			new_entry->row_id = mysql_insert_id(get_mysql_conn());
		}
	}
}
#else
static void on_new_entry_found_report(nDPI_pkt_parser_params* pktparams, const db_row_key *row_key) {
	// do we have this value ?
	plugin_DB_row* existing_entry;
	HASH_FIND(hh, g_S_IP_db_cache, row_key, S_IP_BYTE_COUNT, existing_entry);
	// should only increase counter once per pcap file
	if (existing_entry != NULL) {
#ifdef _DEBUG
		char dst_ip_str[INET6_ADDRSTRLEN];
		if (row_key->server_ip.v6.u6_addr.u6_addr64[1] == 0) {
			inet_ntop(AF_INET, &row_key->server_ip.v4, dst_ip_str, sizeof(dst_ip_str));
		}
		else {
			inet_ntop(AF_INET6, &row_key->server_ip.v6, dst_ip_str, sizeof(dst_ip_str));
		}
		AddLogEntryB(LDF_LOCAL, LogSeverityDebug, LogSourcePluginS_IP, "S_IP identified flow %d as app %ld with confidence %f. IP: %s\n",
			pktparams->flow_to_process->flow_id, existing_entry->apps[0].application_id, existing_entry->apps[0].confidence, dst_ip_str);
#else
		AddLogEntryB(LDF_LOCAL, LogSeverityDebug, LogSourcePluginS_IP, "S_IP identified flow %d as app %ld with confidence %f\n",
			pktparams->flow_to_process->flow_id, existing_entry->apps[0].application_id, existing_entry->apps[0].confidence);
#endif
		for (size_t i = 0; i < _countof(existing_entry->apps); i++) {
			if (existing_entry->apps[i].application_id != 0) {
				if (existing_entry->apps[i].confidence >= g_StopSearchTreshold) {
					pktparams->flow_to_process->app_is_identified = 1;
				}
				else {
					pktparams->flow_to_process->app_is_identified_unsure = 1;
				}
				// push results so reporting can use it later on
				queue_app_detected_result(pktparams, PT_IP_SERVER, existing_entry->apps[i].application_id, existing_entry->apps[i].confidence);
			}
		}
	}
}
#endif

static void update_app_confidence_scores_S_IP()
{
	plugin_DB_row* entry = NULL, * tmp = NULL;
	HASH_ITER(hh, g_S_IP_db_cache, entry, tmp) {
		for (size_t i = 0; i < S_IP_MAX_APPIDS_TRACKED; i++) {
			if (entry->apps[i].application_id != 0 && entry->total_occurance_count) {
				entry->apps[i].confidence = (double)entry->apps[i].occurance_count / (double)entry->total_occurance_count;
			}
		}
	}
}

static void insert_update_db_row_to_cache_S_IP(const char* szid, const char* szIP, const char* szapplication_id, const char* szoccurrences) {
	long app_id = strtol(szapplication_id, NULL, 10);
#ifndef _APP_MODE_BUILD_DB
	(void)szid;
#endif
	// convert string IP into a binary representation
	db_row_key row_key = { 0 };
	if (strchr(szIP,'.')) {
		if (inet_pton(AF_INET, szIP, &row_key.server_ip.v4) != 1) {
			AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "Failed to convert ipv4 str %s to bin\n", szIP);
			return; // conversion failed
		}
	}
	else {
		if (inet_pton(AF_INET6, szIP, &row_key.server_ip.v6) != 1) {
			AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "Failed to convert ipv6 str %s to bin\n", szIP);
			return;
		}
	}

	plugin_DB_row* existing_entry;
	HASH_FIND(hh, g_S_IP_db_cache, &row_key, S_IP_BYTE_COUNT, existing_entry);
	// should only increase counter once per pcap file
	if (existing_entry != NULL) {
#ifdef _APP_MODE_BUILD_DB
		if (app_id == app_id_for_pcap) {
			existing_entry->row_id = strtol(szid, NULL, 10);
		}
#endif
		// find the first available slot that we can store it in
		int free_slot = get_app_id_slot_S_IP(existing_entry, 0);
		if (free_slot >= 0) {
			existing_entry->apps[free_slot].application_id = app_id;
			existing_entry->apps[free_slot].occurance_count = strtol(szoccurrences, NULL, 10);
			existing_entry->total_occurance_count += existing_entry->apps[free_slot].occurance_count;
		}
	}
	else {
		plugin_DB_row* new_entry = (plugin_DB_row*)malloc(sizeof(plugin_DB_row));
		init_plugin_DB_row_S_IP(new_entry);
#ifdef _APP_MODE_BUILD_DB
		new_entry->row_id = strtol(szid, NULL, 10);
		new_entry->updated_ocurrences = 0;
#endif
		memcpy(&new_entry->row_key, &row_key, S_IP_BYTE_COUNT);
		new_entry->apps[0].application_id = app_id;
		new_entry->apps[0].occurance_count = strtol(szoccurrences, NULL, 10);
		new_entry->total_occurance_count = new_entry->apps[0].occurance_count;
#ifdef _APP_MODE_BUILD_DB
		if (app_id == app_id_for_pcap) {
			new_entry->row_id = strtol(szid, NULL, 10);
		}
#endif

		HASH_ADD(hh, g_S_IP_db_cache, row_key, S_IP_BYTE_COUNT, new_entry);
	}
}

void init_packetp_SERVER_IP() {
	g_is_plugin_disabled = get_ini_int_value("Plugins", "Disable_IP", 0);
	if (g_is_plugin_disabled != 0) {
		return;
	}
	if (get_mysql_conn()) {
		// one time init for easy deployment
		init_plugin_mysql_table();
		// load existing values
		size_t entries_loaded = 0;
		const char* select_query = "select id, serv_ip, application_id,occurrences from serv_ip_fingerprints";
		if (mysql_query(get_mysql_conn(), select_query)) {
			AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n",
				select_query, mysql_error(get_mysql_conn()));
		}
		else {
			MYSQL_RES* res = mysql_store_result(get_mysql_conn());
			if (res) {
				MYSQL_ROW row;
				while ((row = mysql_fetch_row(res))) {
					insert_update_db_row_to_cache_S_IP(row[0], row[1], row[2], row[3]);
					entries_loaded++;
				}
				mysql_free_result(res);
			}
		}
		AddLogEntryB(LDF_LOCAL, LogSeverityDebug, LogSourcePluginS_IP, "S_IP : Loaded %ld entries\n", entries_loaded);

		// calculate confidence
		update_app_confidence_scores_S_IP();
	}
}

void destroy_packetp_SERVER_IP() {
	// free up cache values
	plugin_DB_row* entry = NULL, * tmp = NULL;

	HASH_ITER(hh, g_S_IP_db_cache, entry, tmp) {
		HASH_DEL(g_S_IP_db_cache, entry);
		free(entry);
	}
}

size_t process_packet_SERVER_IP(nDPI_pkt_parser_params* pktparams) {
#define flow_user			   (pktparams->flow_to_process)

	// this flow has been handled and there is nothing to be done with it
	if (flow_user->app_is_identified != 0) {
		return RETC_NO_ERR;
	}
	if (flow_user->checked_plugin_S_IP != 0) {
		return RETC_NO_ERR;
	}
	if (g_is_plugin_disabled != 0) {
		return RETC_NO_ERR;
	}
	if (flow_user->ip_version) {
		flow_user->checked_plugin_S_IP = 1;

		db_row_key row_key = { 0 };
		if (flow_user->ip_version == 4) {
			row_key.server_ip.v4 = flow_user->hash_key_full_val.dst_addr.dst_ip_v4;
		}
		else {
			memcpy(&row_key.server_ip.v6, &flow_user->hash_key_full_val.dst_addr.dst_ip_v6, sizeof(struct ndpi_in6_addr));
		}
#ifdef _APP_MODE_BUILD_DB
		on_new_entry_found_db_gen(flow_user->ip_version, &row_key);
#else
		on_new_entry_found_report(pktparams, &row_key);
#endif
		
		// in some ocasions source and destination is mixed up, need to search both to make sure we search the right server IP
		if (flow_user->ip_version == 4) {
			row_key.server_ip.v4 = flow_user->hash_key_full_val.src_addr.src_ip_v4;
		}
		else {
			memcpy(&row_key.server_ip.v6, &flow_user->hash_key_full_val.src_addr.src_ip_v6, sizeof(struct ndpi_in6_addr));
		}
#ifdef _APP_MODE_BUILD_DB
		on_new_entry_found_db_gen(flow_user->ip_version, &row_key);
#else
		on_new_entry_found_report(pktparams, &row_key);
#endif
	}

	return RETC_NO_ERR;
}