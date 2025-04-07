#include "transport.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

// Initializes a UDP socket and binds it to the given port
int udp_init_socket(unsigned short port) {
    // Create a UDP socket using IPv4
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket"); // Print error if socket creation fails
        return -1;
    }

    // Prepare the sockaddr_in structure to bind to
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr)); // Zero out the structure

    addr.sin_family = AF_INET;             // IPv4
    addr.sin_addr.s_addr = INADDR_ANY;     // Accept messages from any IP
    addr.sin_port = htons(port);           // Host to network byte order

    // Bind the socket to the address and port
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); // Print error if binding fails
        close(sockfd);  // Close the socket before exiting
        return -1;
    }

    return sockfd; // Return the socket file descriptor
}

// Sends a UDP datagram to a specific IP and port
int udp_send(int sockfd, const char *message, size_t message_len, const char *ip, unsigned short port) {
    // Set up destination address structure
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));

    dest.sin_family = AF_INET;        // IPv4
    dest.sin_port = htons(port);      // Set destination port

    // Convert IP address string to binary form
    if (inet_pton(AF_INET, ip, &dest.sin_addr) <= 0) {
        perror("inet_pton"); // Error if IP conversion fails
        return -1;
    }

    // Send the UDP message using sendto()
    ssize_t sent = sendto(sockfd, message, message_len, 0, (struct sockaddr*)&dest, sizeof(dest));
    if (sent < 0) {
        perror("sendto"); // Error if sending fails
        return -1;
    }

    return 0; // Success
}

// Receives a UDP datagram (blocking call), optionally returns sender info
int udp_receive(int sockfd, char *buffer, size_t buffer_len, char *sender_ip, unsigned short *sender_port) {
    struct sockaddr_in sender_addr;          // To store sender's info
    socklen_t addrlen = sizeof(sender_addr); // Length of the address struct

    // Receive the datagram
    ssize_t received = recvfrom(sockfd, buffer, buffer_len - 1, 0,
                                (struct sockaddr*)&sender_addr, &addrlen);
    if (received < 0) {
        perror("recvfrom"); // Error if receiving fails
        return -1;
    }

    buffer[received] = '\0'; // Null-terminate the buffer safely

    // Convert sender address to string if requested
    if (sender_ip)
        inet_ntop(AF_INET, &sender_addr.sin_addr, sender_ip, INET_ADDRSTRLEN);

    // Extract port number if requested
    if (sender_port)
        *sender_port = ntohs(sender_addr.sin_port);

    return (int)received; // Return number of bytes received
}
