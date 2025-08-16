#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <stdint.h>
#include "ini_file_handler.h"

static IniFile g_IniFile;

static void trim_whitespace(char* str) {
    char* end;
    while (isspace((unsigned char)*str)) str++;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) *end-- = '\0';
}

int parse_ini_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return 0;

    char line[512];
    char currentSection[MAX_NAME_LEN] = "";
    g_IniFile.sectionCount = 0;

    while (fgets(line, sizeof(line), file)) {
        trim_whitespace(line);
        if (line[0] == '\0' || line[0] == '#') continue;

        if (line[0] == '[' && line[strlen(line) - 1] == ']') {
            strncpy(currentSection, line + 1, strlen(line) - 2);
            currentSection[strlen(line) - 2] = '\0';

            IniSection* section = &g_IniFile.sections[g_IniFile.sectionCount++];
            strncpy(section->name, currentSection, MAX_NAME_LEN);
            section->pairCount = 0;
        } else {
            char* equals = strchr(line, '=');
            if (!equals) continue;

            *equals = '\0';
            char* key = line;
            char* value = equals + 1;
            trim_whitespace(key);
            trim_whitespace(value);

            for (int i = 0; i < g_IniFile.sectionCount; ++i) {
                if (strcmp(g_IniFile.sections[i].name, currentSection) == 0) {
                    IniSection* section = &g_IniFile.sections[i];
                    if (section->pairCount < MAX_KEYS) {
                        strncpy(section->pairs[section->pairCount].key, key, MAX_NAME_LEN - 1);
                        section->pairs[section->pairCount].key[MAX_NAME_LEN - 1] = '\0';
                        strncpy(section->pairs[section->pairCount].value, value, MAX_NAME_LEN - 1);
                        section->pairs[section->pairCount].value[MAX_VALUE_LEN - 1] = '\0';
                        section->pairCount++;
                    }
                    break;
                }
            }
        }
    }

    fclose(file);
    return 1;
}

void destroy_ini_file() {
    g_IniFile.sectionCount = 0;
    memset(&g_IniFile, 0, sizeof(g_IniFile));
}

int write_ini_file(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return 0;

    for (int i = 0; i < g_IniFile.sectionCount; ++i) {
        fprintf(file, "[%s]\n", g_IniFile.sections[i].name);
        for (int j = 0; j < g_IniFile.sections[i].pairCount; ++j) {
            fprintf(file, "%s=%s\n", g_IniFile.sections[i].pairs[j].key, g_IniFile.sections[i].pairs[j].value);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    return 1;
}

const char* get_ini_value(const char* section, const char* key) {
    for (int i = 0; i < g_IniFile.sectionCount; ++i) {
        if (strcmp(g_IniFile.sections[i].name, section) == 0) {
            for (int j = 0; j < g_IniFile.sections[i].pairCount; ++j) {
                if (strcmp(g_IniFile.sections[i].pairs[j].key, key) == 0) {
                    return g_IniFile.sections[i].pairs[j].value;
                }
            }
        }
    }
    return NULL;
}

int64_t get_ini_int_value(const char* section, const char* key, int64_t default_val) {
    const char* value = get_ini_value(section, key);
    if (!value) {
        return default_val;
    }

    // try to parse as integer
    errno = 0;
    char* endptr;
    int64_t result = strtol(value, &endptr, 10);

    if (errno != 0 || *endptr != '\0') {
        return default_val;  // invalid or partial conversion
    }

    return result;
}

int64_t get_ini_bool_value(const char* section, const char* key, int64_t default_val) {
    const char* value = get_ini_value(section, key);
    if (!value) {
        return default_val;
    }

    // Case-insensitive checks for common boolean true/false values
    if (strcasecmp(value, "true") == 0 ||
        strcasecmp(value, "yes") == 0 ||
        strcasecmp(value, "1") == 0 ||
        strcasecmp(value, "on") == 0)
        return 1;

    if (strcasecmp(value, "false") == 0 ||
        strcasecmp(value, "no") == 0 ||
        strcasecmp(value, "0") == 0 ||
        strcasecmp(value, "off") == 0)
        return 0;

    return default_val;
}

double get_ini_double_value(const char* section, const char* key, double default_val) {
    const char* value = get_ini_value(section, key);
    if (!value) {
        return default_val;
    }

    // Try to parse as double
    errno = 0;
    char* endptr;
    double result = strtod(value, &endptr);

    if (errno != 0 || *endptr != '\0') {
        return default_val;  // invalid or partial conversion
    }

    return result;
}