#include <mysql/mysql.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "ndpi_types.h"
#include "reader_util.h"
#include "plugin_SNI.h"
#include "log_manager.h"
#include "uthash.h"
#include "mysql_interface.h"
#include "plugin_globals.h"
#include "utils.h"
#include "ini_file_handler.h"

extern int app_id_for_pcap;
extern char* _pcap_file[MAX_NUM_READER_THREADS];

#define SNI_BYTE_COUNT 80
#define SNI_MAX_APPIDS_TRACKED	20 // could make it dynamic

typedef struct app_id_confidence {
	uint16_t application_id;
	uint16_t occurance_count;
	double confidence;
}app_id_confidence;

typedef struct plugin_DB_row {
	char host_server_name[SNI_BYTE_COUNT]; // should be in sync with flow host_server_name size
	app_id_confidence apps[SNI_MAX_APPIDS_TRACKED];
	uint16_t total_occurance_count; // number of applications are using this. Loaded from DB, not tracked while parsing pcap
#ifdef _APP_MODE_BUILD_DB
	uint8_t updated_ocurrences; // happens only when ingesting a pcap for a specific app. Will not happen for more than 1 app
	uint32_t row_id;
#endif
	UT_hash_handle hh;
}plugin_DB_row;

// make a habbit of calling this function even if it seems useless
// consider it safe coding
static void init_plugin_DB_row_SNI(plugin_DB_row* this) {
	// !! hashing might fail if you happen to use a hash key that has bytes uninitialized !!
	// !! migh happen on dynamic length strings or padded structure fields !!
	memset(this, 0, sizeof(plugin_DB_row));
}

// this is plugin specific. We will only read it when using live traffic.
static plugin_DB_row* g_SNI_db_cache = NULL;
#ifdef _APP_MODE_BUILD_DB
extern int g_app_table_updated;
#endif
static size_t g_is_plugin_disabled = 0;

static void init_plugin_mysql_table() {
	const char* create_SNI_fingerprints_table_sql =
		"CREATE TABLE IF NOT EXISTS sni_fingerprints (\n"
		"    id INT UNSIGNED NOT NULL AUTO_INCREMENT,\n"
		"    sni CHAR(80) NOT NULL,\n"
		"    occurrences INT UNSIGNED NOT NULL DEFAULT 1,\n"
		"    application_id INT UNSIGNED DEFAULT NULL,\n"
		"    sources TEXT NOT NULL,\n"
		"    PRIMARY KEY (id),\n"
		"    CONSTRAINT fk_application_SNI FOREIGN KEY (application_id)\n"
		"        REFERENCES applications(id)\n"
		"        ON DELETE SET NULL\n"
		"        ON UPDATE CASCADE\n"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;";

	if (get_mysql_conn() && mysql_query(get_mysql_conn(), create_SNI_fingerprints_table_sql)) {
		AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", 
			create_SNI_fingerprints_table_sql, mysql_error(get_mysql_conn()));
	}
}

static int get_app_id_slot_SNI(plugin_DB_row* existing_entry, uint16_t app_id) {
	if (existing_entry == NULL) {
		return 0;
	}
	for (size_t i = 0; i < SNI_MAX_APPIDS_TRACKED; i++) {
		if (existing_entry->apps[i].application_id == app_id) {
			return i;
		}
	}
	if (app_id > 0) {
		AddLogEntryB(LDF_LOCAL, LogSeverityWarn, LogSourcePluginSNI, "SNI : All app_id slots are taken. Increase SNI_MAX_APPIDS_TRACKED\n");
	}
	return -1;
}

#ifdef _APP_MODE_BUILD_DB
static void on_new_entry_found_db_gen(char* host_server_name, uint64_t sni_size) {
	// do we have this value ?
	plugin_DB_row* existing_entry;
	HASH_FIND(hh, g_SNI_db_cache, host_server_name, SNI_BYTE_COUNT, existing_entry);
	int existing_app_index = get_app_id_slot_SNI(existing_entry, app_id_for_pcap);
	// should only increase counter once per pcap file
	if (existing_entry != NULL && existing_app_index >= 0) {
		if (existing_entry->updated_ocurrences == 0 && existing_entry->row_id) {
			existing_entry->updated_ocurrences = 1;
			char update_sql[2000];
			snprintf(update_sql, sizeof(update_sql), "update sni_fingerprints set occurrences = occurrences + 1 where id = %d", existing_entry->row_id);
			if (get_mysql_conn() && mysql_query(get_mysql_conn(), update_sql)) {
				AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", update_sql, mysql_error(get_mysql_conn()));
			}
			snprintf(update_sql, sizeof(update_sql), "update sni_fingerprints set sources = CONCAT(sources, '%s,') where id = %d and sources NOT LIKE '%%%s,%%'",
				_pcap_file[0], existing_entry->row_id, _pcap_file[0]);
			if (get_mysql_conn() && mysql_query(get_mysql_conn(), update_sql)) {
				AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", update_sql, mysql_error(get_mysql_conn()));
			}
		}
	}
	plugin_DB_row* new_entry = NULL;
	// hash exists in DB, but not for this specific app
	if (existing_entry != NULL && existing_app_index == -1) {
		existing_app_index = get_app_id_slot_SNI(existing_entry, 0);
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
		init_plugin_DB_row_SNI(new_entry);
		size_t copy_len = sni_size < sizeof(new_entry->host_server_name) - 1 ? sni_size : sizeof(new_entry->host_server_name) - 1;
		memcpy(new_entry->host_server_name, host_server_name, copy_len);
		new_entry->host_server_name[copy_len] = '\0';
		new_entry->updated_ocurrences = 1;
		new_entry->apps[existing_app_index].application_id = app_id_for_pcap;
		HASH_ADD(hh, g_SNI_db_cache, host_server_name, SNI_BYTE_COUNT, new_entry);
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
		snprintf(update_sql, sizeof(update_sql), "insert into sni_fingerprints "
			"(sni,application_id,sources) values ('%s',%d,'%s,')", host_server_name, new_entry->apps[existing_app_index].application_id, _pcap_file[0]);
		if (mysql_query(get_mysql_conn(), update_sql)) {
			AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", update_sql, mysql_error(get_mysql_conn()));
		}
		else {
			new_entry->row_id = mysql_insert_id(get_mysql_conn());
		}
	}
}
#else
static void on_new_entry_found_report(nDPI_pkt_parser_params* pktparams, char* host_server_name) {
	// do we have this value ?
	plugin_DB_row* existing_entry;
	HASH_FIND(hh, g_SNI_db_cache, host_server_name, SNI_BYTE_COUNT, existing_entry);
	// should only increase counter once per pcap file
	if (existing_entry != NULL) {
		AddLogEntryB(LDF_LOCAL, LogSeverityDebug, LogSourcePluginSNI, "SNI identified flow %d as app %ld with confidence %f. SNI %s\n", 
			pktparams->flow_to_process->flow_id, existing_entry->apps[0].application_id, existing_entry->apps[0].confidence, existing_entry->host_server_name);
		for (size_t i = 0; i < _countof(existing_entry->apps); i++) {
			if (existing_entry->apps[i].application_id != 0) {
				if (existing_entry->apps[i].confidence >= g_StopSearchTreshold) {
					pktparams->flow_to_process->app_is_identified = 1;
				}
				else {
					pktparams->flow_to_process->app_is_identified_unsure = 1;
				}
				// push results so reporting can use it later on
				queue_app_detected_result(pktparams, PT_SNI, existing_entry->apps[i].application_id, existing_entry->apps[i].confidence);
			}
		}
	}
}
#endif

static void update_app_confidence_scores_SNI()
{
	plugin_DB_row *entry = NULL, *tmp= NULL;

	HASH_ITER(hh, g_SNI_db_cache, entry, tmp) {
		for (size_t i = 0; i < SNI_MAX_APPIDS_TRACKED; i++) {
			if (entry->apps[i].application_id != 0 && entry->total_occurance_count) {
				entry->apps[i].confidence = (double)entry->apps[i].occurance_count / (double)entry->total_occurance_count;
			}
		}
	}
}

static void insert_update_db_row_to_cache_SNI(const char *szid, const char *szSNI, const char *szapplication_id, const char *szoccurrences) {
	long app_id = strtol(szapplication_id, NULL, 10);
#ifndef _APP_MODE_BUILD_DB
	(void)szid;
#endif
	plugin_DB_row* existing_entry;
	char db_row_key[SNI_BYTE_COUNT];
	memset(db_row_key, 0, sizeof(db_row_key));
	memcpy(db_row_key, szSNI, min(sizeof(db_row_key) - 1, strlen(szSNI)));
	HASH_FIND(hh, g_SNI_db_cache, db_row_key, SNI_BYTE_COUNT, existing_entry);
	// should only increase counter once per pcap file
	if (existing_entry != NULL) {
#ifdef _APP_MODE_BUILD_DB
		if (app_id == app_id_for_pcap) {
			existing_entry->row_id = strtol(szid, NULL, 10);
		}
#endif
		// find the first available slot that we can store it in
		int free_slot = get_app_id_slot_SNI(existing_entry, 0);
		if (free_slot >= 0) {
			existing_entry->apps[free_slot].application_id = app_id;
			existing_entry->apps[free_slot].occurance_count = strtol(szoccurrences, NULL, 10);
			existing_entry->total_occurance_count += existing_entry->apps[free_slot].occurance_count;
		}
	}
	else {
		plugin_DB_row* new_entry = (plugin_DB_row*)malloc(sizeof(plugin_DB_row));
		init_plugin_DB_row_SNI(new_entry);
#ifdef _APP_MODE_BUILD_DB
		new_entry->row_id = strtol(szid, NULL, 10);
		new_entry->updated_ocurrences = 0;
#endif
		uint64_t SNI_size = strlen(szSNI); // should always be 32
		size_t copy_len = SNI_size < sizeof(new_entry->host_server_name) - 1 ? SNI_size : sizeof(new_entry->host_server_name) - 1;
		memcpy(new_entry->host_server_name, szSNI, copy_len);
		new_entry->host_server_name[copy_len] = '\0';
		new_entry->apps[0].application_id = app_id;
		new_entry->apps[0].occurance_count = strtol(szoccurrences, NULL, 10);
		new_entry->total_occurance_count = new_entry->apps[0].occurance_count;
#ifdef _APP_MODE_BUILD_DB
		if (app_id == app_id_for_pcap) {
			new_entry->row_id = strtol(szid, NULL, 10);
		}
#endif

		HASH_ADD(hh, g_SNI_db_cache, host_server_name, SNI_BYTE_COUNT, new_entry);
	}
}

void init_packetp_SNI() {
	g_is_plugin_disabled = get_ini_int_value("Plugins", "Disable_DNS", 0);
	if (g_is_plugin_disabled != 0) {
		return;
	}
	if (get_mysql_conn()) {
		// one time init for easy deployment
		init_plugin_mysql_table();
		// load existing values
		size_t entries_loaded = 0;
		const char* select_query = "select id,sni,application_id,occurrences from sni_fingerprints";
		if (mysql_query(get_mysql_conn(), select_query)) {
			AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n",
				select_query, mysql_error(get_mysql_conn()));
		}
		else {
			MYSQL_RES* res = mysql_store_result(get_mysql_conn());
			if (res) {
				MYSQL_ROW row;
				while ((row = mysql_fetch_row(res))) {
					insert_update_db_row_to_cache_SNI(row[0], row[1], row[2], row[3]);
					entries_loaded++;
				}
				mysql_free_result(res);
			}
		}
		AddLogEntryB(LDF_LOCAL, LogSeverityDebug, LogSourcePluginSNI, "SNI : Loaded %ld entries\n", entries_loaded);

		// calculate confidence
		update_app_confidence_scores_SNI();
	}
}

void destroy_packetp_SNI() {
	// free up cache values
	plugin_DB_row* entry = NULL, * tmp = NULL;

	HASH_ITER(hh, g_SNI_db_cache, entry, tmp) {
		HASH_DEL(g_SNI_db_cache, entry); 
		free(entry);                      
	}
}

size_t process_packet_SNI(nDPI_pkt_parser_params* pktparams) {
#define flow			   (pktparams->flow_to_process)
	// this flow has been handled and there is nothing to be done with it
	if (flow->app_is_identified != 0) {
		return RETC_NO_ERR;
	}
	if (flow->checked_plugin_SNI != 0) {
		return RETC_NO_ERR;
	}
	if (flow->host_server_name[0] != 0) {
		flow->checked_plugin_SNI = 1;
#ifdef _DEBUG
		_Static_assert(sizeof(flow->host_server_name) == SNI_BYTE_COUNT, "SNI Hash key size differs changes since it was made. fix it");
#endif
#ifdef _APP_MODE_BUILD_DB
		on_new_entry_found_db_gen(flow->host_server_name, sizeof(flow->host_server_name));
#else
		on_new_entry_found_report(pktparams, flow->host_server_name);
#endif
	}

	return RETC_NO_ERR;
}