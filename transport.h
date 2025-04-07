#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <stddef.h>
#include <netinet/in.h>

// Initializes a UDP socket and binds to the given port (0 for dynamic)
// Returns socket fd, or -1 on error
int udp_init_socket(unsigned short port);

// Sends a message to the given destination
int udp_send(int sockfd, const char *message, size_t message_len, const char *ip, unsigned short port);

// Receives a message (blocking call), populates sender info
int udp_receive(int sockfd, char *buffer, size_t buffer_len, char *sender_ip, unsigned short *sender_port);

#endif // TRANSPORT_H
