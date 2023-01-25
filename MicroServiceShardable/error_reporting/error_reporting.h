#ifndef ERROR_REPORTING_H
#define ERROR_REPORTING_H

#define REPORT_CONNECTION_COUNT_LARGER    100     // right now persistent connections are not used. If connection count
											      // increases strangely large, report it
#define REPORT_MEM_USAGE_COUNT_LARGER     200     // every connection uses up 1 allocation. System will only do cleanup after X allocations
#define REPORT_REQUEST_PROCESSING_TIME_MS 60*1000 // if merging scores take too much time, investigate it											

// Functions
int errorReporting_start(const char* errorDirectory);
int errorReporting_stop();

void perform_SystemSanityChecks(int activeConnectionCount, int expectedMemUsagePerConnection, int reqProcessingTime);
#endif // ERROR_REPORTING_H
