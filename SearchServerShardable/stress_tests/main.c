#include <config_reader.h>
#include <app_errors.h>
#include <worker_threads.h>
#include <signal.h>
#include <logger.h>

static void signalHandler(int sigNum)
{
	stopWorkerThreads();
}

int main()
{
	// get config settings
	if (parseConfigFile("stresser.conf") != ERR_NO_ERROR)
	{
		freeAppConf();
		return 1;
	}

	// replace ID with empty space so we can overwrite it by worker threads
	prepareEntryContent();

	// make sure we can shut down the program
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	// alloc memory and stuff for threads
	prepareThreadParams();

	// start it before worker threads
	startLoggerThread();

	LOG_MESSAGE(INFO_LOG_MSG, "Starting worker threads. If server IP is bad, there should be a 2 second freez.");
	startWorkerThreads();

	// cleanup
	freeAppConf();
	return 0;
}