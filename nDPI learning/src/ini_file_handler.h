#ifndef _INI_FILE_HANDLER_H_
#define _INI_FILE_HANDLER_H_

/* 
* There was no need for a more complicated ini handler. In case there is a need, should be swapped out to a 3rd party library
* Right now config values are used for initializing the application. No actual settings are fetched after app startup
* Ideas for later on :
*   - dynamic memory allocations
*   - also sore parsed values : int, double
*   - hashmap for key lookups
*/


#define MAX_SECTIONS 100
#define MAX_KEYS 100
#define MAX_NAME_LEN 64
#define MAX_VALUE_LEN 256

typedef struct {
    char key[MAX_NAME_LEN];
    char value[MAX_VALUE_LEN];
} IniKeyValue;

typedef struct {
    char name[MAX_NAME_LEN];
    IniKeyValue pairs[MAX_KEYS];
    int pairCount;
} IniSection;

typedef struct {
    IniSection sections[MAX_SECTIONS];
    int sectionCount;
} IniFile;

int parse_ini_file(const char* filename);
void destroy_ini_file();
int write_ini_file(const char* filename);
const char* get_ini_value(const char* section, const char* key);
int64_t get_ini_int_value(const char* section, const char* key, int64_t default_val);
int64_t get_ini_bool_value(const char* section, const char* key, int64_t default_val);
double get_ini_double_value(const char* section, const char* key, double default_val);

#endif
