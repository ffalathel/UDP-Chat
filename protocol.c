#include "protocol.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Define the delimiter used between message fields
#define DELIM "|"

// Parses a raw delimited string into a structured ChatMessage
int parse_message(const char *raw, ChatMessage *msg) {
    // Check for null pointers
    if (!raw || !msg) return -1;

    // Copy the raw message to avoid modifying original input
    char copy[MAX_MESSAGE_LEN + 256];
    strncpy(copy, raw, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    // Use strtok to split the string based on the delimiter

    // Get the username field
    char *token = strtok(copy, DELIM);
    if (!token) return -1;
    strncpy(msg->username, token, MAX_USERNAME_LEN);

    // Get the hostname field
    token = strtok(NULL, DELIM);
    if (!token) return -1;
    strncpy(msg->hostname, token, MAX_HOSTNAME_LEN);

    // Get the chat start time (as a string and convert to time_t)
    token = strtok(NULL, DELIM);
    if (!token) return -1;
    msg->chat_start_time = (time_t)atol(token);

    // Get the bytes sent field
    token = strtok(NULL, DELIM);
    if (!token) return -1;
    msg->bytes_sent = (size_t)atol(token);

    // Get the remaining part of the message as the message text
    token = strtok(NULL, ""); // Take the rest of the string
    if (!token) return -1;
    strncpy(msg->message_text, token, MAX_MESSAGE_LEN);

    return 0; // Success
}

// Formats a ChatMessage struct into a delimited string buffer
int format_message(const ChatMessage *msg, char *buffer, size_t bufsize) {
    if (!msg || !buffer) return -1;

    // Format the message using snprintf to avoid buffer overflow
    int written = snprintf(buffer, bufsize, "%s|%s|%ld|%zu|%s",
        msg->username,               // Username field
        msg->hostname,              // Hostname field
        msg->chat_start_time,       // Chat start timestamp
        msg->bytes_sent,            // Total bytes sent
        msg->message_text           // Actual message content
    );

    // Return success if formatting succeeded and didn’t overflow buffer
    return (written > 0 && (size_t)written < bufsize) ? 0 : -1;
}
