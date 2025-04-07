#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include <time.h>

#define MAX_MESSAGE_LEN 1024
#define MAX_USERNAME_LEN 64
#define MAX_HOSTNAME_LEN 64

typedef struct {
    char username[MAX_USERNAME_LEN];
    char hostname[MAX_HOSTNAME_LEN];
    time_t chat_start_time;
    size_t bytes_sent;
    char message_text[MAX_MESSAGE_LEN];
} ChatMessage;

// Parses a delimited string into a ChatMessage struct
int parse_message(const char *raw, ChatMessage *msg);

// Formats a ChatMessage struct into a delimited string
int format_message(const ChatMessage *msg, char *buffer, size_t bufsize);

#endif // PROTOCOL_H
