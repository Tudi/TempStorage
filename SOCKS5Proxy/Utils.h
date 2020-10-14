#pragma once

extern HANDLE lock;
/*
 * Error handling.
 */
static void message(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    WaitForSingleObject(lock, INFINITE);
    vfprintf(stderr, msg, args);
    putc('\n', stderr);
    ReleaseMutex(lock);
    va_end(args);
}
#define error(msg, ...)                         \
    do {                                        \
        message("error: " msg, ## __VA_ARGS__); \
        exit(EXIT_FAILURE);                     \
    } while (FALSE)
#define warning(msg, ...)                       \
    message("warning: " msg, ## __VA_ARGS__)
