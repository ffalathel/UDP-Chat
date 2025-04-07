#ifndef SESSION_H
#define SESSION_H

#include <time.h>
#include <stddef.h>

typedef struct {
    char username[64];
    char hostname[64];
    time_t start_time;
    size_t bytes_sent;
    size_t bytes_received;
} ChatSession;

#endif // SESSION_H
