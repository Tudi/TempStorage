#include <mysql/mysql.h>
#include "log_manager.h"
#include "ini_file_handler.h"

static MYSQL* g_conn = NULL;

struct MYSQL* get_mysql_conn() {
	return g_conn;
}

void create_common_app_table() {
	const char* create_applications_table_sql =
		"CREATE TABLE IF NOT EXISTS applications (\n"
		"    id INT UNSIGNED NOT NULL AUTO_INCREMENT,\n"
		"    name VARCHAR(191) NOT NULL UNIQUE,\n"
		"    app_group INT UNSIGNED NOT NULL DEFAULT 0,\n"
		"    description TEXT,\n"
		"    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,\n"
		"    pcaps_parsed INT UNSIGNED NOT NULL DEFAULT 0,\n"
		"    pcap_names TEXT,\n"
		"    PRIMARY KEY (id)\n"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;";

	if (mysql_query(g_conn, create_applications_table_sql)) {
		AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", create_applications_table_sql, mysql_error(g_conn));
	}
}

// TODO : move this somewhere else. External SQL script ?
void init_app_table_content() {
	{
		const char* app_sql = "insert ignore into applications values (1, \"Airbnb\", 0, \"\", NULL, 0, '');";
		if (mysql_query(g_conn, app_sql)) {
			AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", app_sql, mysql_error(g_conn));
		}
	}
	{
		const char* app_sql = "insert ignore into applications values (2, \"American Airlines\", 0, \"\", NULL, 0, '');";
		if (mysql_query(g_conn, app_sql)) {
			AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", app_sql, mysql_error(g_conn));
		}
	}
	{
		const char* app_sql = "insert ignore into applications values (3, \"American Express\", 0, \"\", NULL, 0, '');";
		if (mysql_query(g_conn, app_sql)) {
			AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", app_sql, mysql_error(g_conn));
		}
	}
	{
		const char* app_sql = "insert ignore into applications values (4, \"Amtrak\", 0, \"\", NULL, 0, '');";
		if (mysql_query(g_conn, app_sql)) {
			AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", app_sql, mysql_error(g_conn));
		}
	}
	{
		const char* app_sql = "insert ignore into applications values (5, \"Bank of Scotland\", 0, \"\", NULL, 0, '');";
		if (mysql_query(g_conn, app_sql)) {
			AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", app_sql, mysql_error(g_conn));
		}
	}
	{
		const char* app_sql = "insert ignore into applications values (6, \"Barclays\", 0, \"\", NULL, 0, '');";
		if (mysql_query(g_conn, app_sql)) {
			AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", app_sql, mysql_error(g_conn));
		}
	}
	{
		const char* app_sql = "insert ignore into applications values (7, \"Lufthansa airlines\", 0, \"\", NULL, 0, '');";
		if (mysql_query(g_conn, app_sql)) {
			AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", app_sql, mysql_error(g_conn));
		}
	}
	{
		const char* app_sql = "insert ignore into applications values (8, \"Swiss airlines\", 0, \"\", NULL, 0, '');";
		if (mysql_query(g_conn, app_sql)) {
			AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", app_sql, mysql_error(g_conn));
		}
	}
	{
		const char* app_sql = "insert ignore into applications values (9, \"Uber\", 0, \"\", NULL, 0, '');";
		if (mysql_query(g_conn, app_sql)) {
			AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "query %s failed. Error: %s\n", app_sql, mysql_error(g_conn));
		}
	}
}

int init_mysql_interface() {
	g_conn = mysql_init(NULL);
	if (g_conn == NULL) {
		AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "mysql_init() failed");
		return EXIT_FAILURE;
	}

	const char* host = get_ini_value("MySQL", "Host");
	int port = get_ini_int_value("MySQL", "Port", 0);
	const char* user = get_ini_value("MySQL", "User");
	const char* passw = get_ini_value("MySQL", "Passw");
	const char* dbName = get_ini_value("MySQL", "DBName");

	if (mysql_real_connect(g_conn, host, user, passw, dbName, port, NULL, 0) == NULL) {
		AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceMySQL, "mysql_real_connect() failed: %s", mysql_error(g_conn));
		mysql_close(g_conn);
		g_conn = NULL;
		return EXIT_FAILURE;
	}

	// one time init. Done here so that deployment is smooth and easy. Feel free to externalize it
	create_common_app_table();
	init_app_table_content();

	return 0;
}

void destroy_mysql_interface() {
	if (g_conn) {
		mysql_close(g_conn);
		g_conn = NULL;
	}
	mysql_library_end();
}
