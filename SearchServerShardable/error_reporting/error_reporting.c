#include <error_reporting.h>
#include <sentry.h>
#include <stdio.h>

int errorReporting_start(const char *errorDirectory)
{
    // By default, Sentry will read the environment variables: SENTRY_DSN, SENTRY_RELEASE, SENTRY_ENVIRONMENT.
    sentry_options_t *options = sentry_options_new();
    sentry_options_set_database_path(options, errorDirectory);
    sentry_options_set_debug(options, 1);
    int ret = sentry_init(options);
    if (ret == 0)
    {
        sentry_flush(10 * 1000); // Attempt to flush all pending events within 10 seconds.
    }
    return ret;
}

int errorReporting_stop()
{
    return sentry_close();
}
