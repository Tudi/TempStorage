#ifndef _MYSQL_INTERFACE_H_
#define _MYSQL_INTERFACE_H_

/*
* SQL is only used for centralized static data storage
* Since there is no need for performance, a single connection will be used
* App detector plugins will init/store/load their specific data
* All data will be in memory cached for fast access
*/
int init_mysql_interface();
void destroy_mysql_interface();
// right now there is a single connection. See no point to make it dynamic or pooled
struct MYSQL* get_mysql_conn();

#endif