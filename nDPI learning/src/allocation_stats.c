#define _GNU_SOURCE
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log_manager.h"
#include <dlfcn.h>
#include "uthash.h"

typedef struct allocation_stat_store {
    char* callstack;
    int64_t callcount;
    int64_t totalallocated;
}allocation_stat_store;

#define ALLOC_STATS_GROW_STEP 2000
#define MAX_CALLSTACK_DEPTH 15
#define SKIP_FIRST_N_CALLS  2
#define SKIP_REPORT_IF_COUNT_SMALLER 10

static struct allocation_stat_store* allocation_stats = NULL;
static int allocation_stats_count = 0;
static int allocation_stats_capacity = 0;
static int allocation_enable_stats = 0; // only start statistics once we calm down with the init phase

#define MAX_SYMBOL_LINE_LEN 1024

void toggle_allocation_statistics(int enable) {
    allocation_enable_stats = enable;
}

static int deep_symbol_resolution_enabled = 1;
void enable_deep_symbol_resolution(int enable) {
    deep_symbol_resolution_enabled = enable;
}

typedef struct addr2line_cache_entry {
    void* addr;         // key
    char* resolved;     // function info
    UT_hash_handle hh;
} addr2line_cache_entry;

static addr2line_cache_entry* addr2line_cache = NULL;

static const char* resolve_with_addr2line(void* addr) {
    addr2line_cache_entry* entry = NULL;
    HASH_FIND_PTR(addr2line_cache, &addr, entry);
    if (entry) return entry->resolved;

    Dl_info info;
    char* result = NULL;

    if (dladdr(addr, &info) && info.dli_fname) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "addr2line -f -C -e %s %p", info.dli_fname, addr);

        FILE* fp = popen(cmd, "r");
        if (fp) {
            char func[256] = "", fileline[256] = "";
            if (fgets(func, sizeof(func), fp) && fgets(fileline, sizeof(fileline), fp)) {
                func[strcspn(func, "\n")] = 0;
                fileline[strcspn(fileline, "\n")] = 0;

                result = (char*)malloc(strlen(func) + strlen(fileline) + 8);
                if (result) {
                    snprintf(result, MAX_SYMBOL_LINE_LEN, "%s at %s", func, fileline);
                }
            }
            pclose(fp);
        }
    }

    if (!result) {
        result = (char*)malloc(64);
        snprintf(result, 64, "?? (%p)", addr);
    }

    entry = malloc(sizeof(addr2line_cache_entry));
    entry->addr = addr;
    entry->resolved = result;
    HASH_ADD_PTR(addr2line_cache, addr, entry);
    return result;
}

void free_addr2line_cache() {
    addr2line_cache_entry* cur, * tmp;
    HASH_ITER(hh, addr2line_cache, cur, tmp) {
        HASH_DEL(addr2line_cache, cur);
        free(cur->resolved);
        free(cur);
    }
}

char* get_callstack_as_string(int max_frames, int skip_first_n) {
    void* callstack[max_frames];
    int frames = backtrace(callstack, max_frames);
    if (frames <= skip_first_n) return NULL;

    size_t buffer_size = frames * MAX_SYMBOL_LINE_LEN;
    char* result = (char*)malloc(buffer_size);
    if (!result) return NULL;

    result[0] = '\0';

    for (int i = skip_first_n; i < frames; ++i) {
        char line[MAX_SYMBOL_LINE_LEN];
        Dl_info info;
        if (dladdr(callstack[i], &info) && info.dli_sname) {
            snprintf(line, sizeof(line), "%2d: %s + %td (%p) [%s]\n",
                i - skip_first_n,
                info.dli_sname,
                (char*)callstack[i] - (char*)info.dli_saddr,
                callstack[i],
                info.dli_fname ? info.dli_fname : "??");
        }
        else if (deep_symbol_resolution_enabled) {
            const char* resolved = resolve_with_addr2line(callstack[i]);
            snprintf(line, sizeof(line), "%2d: %s\n", i - skip_first_n, resolved);
        }
        else {
            snprintf(line, sizeof(line), "%2d: ?? (%p)\n", i - skip_first_n, callstack[i]);
        }

        strncat(result, line, buffer_size - strlen(result) - 1);
    }

    return result;
}

char* get_callstack_as_string_(int max_frames, int skip_first_n) {
    void* callstack[max_frames];
    int frames = backtrace(callstack, max_frames);
    char** symbols = backtrace_symbols(callstack, frames);

    if (!symbols) return NULL;

    size_t total_len = 0;
    for (int i = skip_first_n; i < frames; ++i) {
        total_len += strlen(symbols[i]) + 1; // +1 for newline
    }

    char* result = (char*)malloc(total_len + 1); // +1 for null terminator
    if (!result) {
        free(symbols);
        return NULL;
    }

    result[0] = '\0';
    for (int i = skip_first_n; i < frames; ++i) {
        strcat(result, symbols[i]);
        strcat(result, "\n");
    }

    free(symbols);
    return result;
}

void on_memory_allocated(int64_t size) {

    // only start profiling once we are done with the init phase
    if (allocation_enable_stats == 0) {
        return;
    }

    char* callstack = get_callstack_as_string(MAX_CALLSTACK_DEPTH, SKIP_FIRST_N_CALLS);
    if (!callstack) {
        return;
    }

    // Search for existing callstack
    for (int i = 0; i < allocation_stats_count; ++i) {
        if (strcmp(allocation_stats[i].callstack, callstack) == 0) {
            allocation_stats[i].callcount += 1;
            allocation_stats[i].totalallocated += size;
            free(callstack);
            return;
        }
    }

    // Grow array if needed
    if (allocation_stats_count == allocation_stats_capacity) {
        int new_capacity = allocation_stats_capacity + ALLOC_STATS_GROW_STEP;
        struct allocation_stat_store* new_stats = realloc(allocation_stats, new_capacity * sizeof(struct allocation_stat_store));
        if (!new_stats) {
            AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceAllocStats, "Failed to grow allocation stats buffer\n");
            free(callstack);
            return;
        }
        allocation_stats = new_stats;
        allocation_stats_capacity = new_capacity;
    }

    // Add new entry
    allocation_stats[allocation_stats_count].callstack = callstack;
    allocation_stats[allocation_stats_count].callcount = 1;
    allocation_stats[allocation_stats_count].totalallocated = size;
    allocation_stats_count++;
}

// used by qsort
static int compare_by_count(const void* a, const void* b) {
    const struct allocation_stat_store* sa = (const struct allocation_stat_store*)a;
    const struct allocation_stat_store* sb = (const struct allocation_stat_store*)b;
    return (sa->callcount > sb->callcount) - (sa->callcount < sb->callcount);
}

void print_memory_allocation_stats() {
    qsort(allocation_stats, allocation_stats_count, sizeof(struct allocation_stat_store), compare_by_count);

    AddLogEntryB(LDF_LOCAL, LogSeverityDebug, LogSourceAllocStats, "=== Memory Allocation Stats ===\n");
    for (int i = 0; i < allocation_stats_count; ++i) {
        if (allocation_stats[i].callcount > SKIP_REPORT_IF_COUNT_SMALLER) {
            AddLogEntryB(LDF_LOCAL, LogSeverityDebug, LogSourceAllocStats,
                "Allocated %ld times, total %ld bytes\nStacktrace:\n%s\n",
                allocation_stats[i].callcount,
                allocation_stats[i].totalallocated,
                allocation_stats[i].callstack);
        }
        free(allocation_stats[i].callstack);
    }
    free(allocation_stats);
    allocation_stats = NULL;
    allocation_stats_count = 0;
    allocation_stats_capacity = 0;
}

void* stat_tracked_malloc(size_t size) {
    void* ptr = malloc(size);
    on_memory_allocated(size);
    return ptr;
}
