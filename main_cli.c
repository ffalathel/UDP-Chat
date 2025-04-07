#include <stdio.h>              // Standard I/O
#include <stdlib.h>             // Memory allocation, exit()
#include <string.h>             // String manipulation
#include <unistd.h>             // For close(), read()
#include <time.h>               // For time()
#include <getopt.h>             // For parsing long command-line options
#include <limits.h>             // For HOST_NAME_MAX
#include <sys/types.h>          // For system types
#include <pwd.h>                // For user info (not used here but included)

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

#include "../common/protocol.h" // Shared protocol definition
#include "../common/transport.h"// UDP transport functions
#include "../common/session.h"  // Chat session tracking struct

#define BUFFER_SIZE 2048        // Buffer size for sending/receiving messages

// Prints usage info for command-line help
void print_usage(const char *prog) {
    printf("Usage: %s --username <name> --port <local_port> --peer-ip <ip> --peer-port <port>\n", prog);
}

// Helper to get hostname of local machine
char* get_hostname() {
    static char hostname[HOST_NAME_MAX];
    gethostname(hostname, sizeof(hostname)); // Fill hostname buffer
    return hostname;
}

int main(int argc, char *argv[]) {
    char username[MAX_USERNAME_LEN] = {0};  // To store current user's name
    unsigned short local_port = 0, peer_port = 0; // Local and peer port numbers
    char peer_ip[INET_ADDRSTRLEN] = {0};    // IP address of peer

    int opt;
    // Define long options for command-line args
    static struct option long_options[] = {
        {"username", required_argument, 0, 'u'},
        {"port", required_argument, 0, 'p'},
        {"peer-ip", required_argument, 0, 'i'},
        {"peer-port", required_argument, 0, 'r'},
        {0, 0, 0, 0}
    };

    // Parse command-line arguments
    while ((opt = getopt_long(argc, argv, "u:p:i:r:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'u': strncpy(username, optarg, MAX_USERNAME_LEN); break;
            case 'p': local_port = (unsigned short)atoi(optarg); break;
            case 'i': strncpy(peer_ip, optarg, INET_ADDRSTRLEN); break;
            case 'r': peer_port = (unsigned short)atoi(optarg); break;
            default: print_usage(argv[0]); exit(1);
        }
    }

    // Validate required inputs
    if (!*username || !*peer_ip || !local_port || !peer_port) {
        print_usage(argv[0]);
        return 1;
    }

    // Create and bind the UDP socket to local port
    int sockfd = udp_init_socket(local_port);
    if (sockfd < 0) {
        fprintf(stderr, "Failed to create socket\n");
        return 1;
    }

    // Initialize chat session
    ChatSession session = {0};
    strncpy(session.username, username, sizeof(session.username));
    strncpy(session.hostname, get_hostname(), sizeof(session.hostname));
    session.start_time = time(NULL);
    session.bytes_sent = 0;
    session.bytes_received = 0;

    printf("Chat started. Type your messages below.\n");

    fd_set fds; // File descriptor set for select()
    char send_buf[BUFFER_SIZE]; // Buffer for outgoing messages
    char recv_buf[BUFFER_SIZE]; // Buffer for incoming messages

    while (1) {
        // Prepare file descriptor set
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds); // Watch for user input
        FD_SET(sockfd, &fds);       // Watch for incoming UDP messages

        int max_fd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

        // Wait for activity on stdin or socket
        if (select(max_fd + 1, &fds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        // Incoming UDP message
        if (FD_ISSET(sockfd, &fds)) {
            char sender_ip[INET_ADDRSTRLEN] = {0};
            unsigned short sender_port = 0;

            int received = udp_receive(sockfd, recv_buf, sizeof(recv_buf), sender_ip, &sender_port);
            if (received > 0) {
                ChatMessage incoming;
                if (parse_message(recv_buf, &incoming) == 0) {
                    size_t expected = incoming.bytes_sent;
                    session.bytes_received += strlen(incoming.message_text);
                    
                    char time_str[64];
                    time_t chat_time = incoming.chat_start_time;
                    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&chat_time));
                    printf("🕒 Chat started at: %s\n", time_str);
                    

                    // Print the message with sender info
                    printf("[%s@%s] %s\n", incoming.username, incoming.hostname, incoming.message_text);
                    printf("📥 Local bytes: %zu\n", session.bytes_received);
                    printf("🔢 Bytes sent by %s: %zu\n", incoming.username, expected);
                    // Check for byte discrepancy
                    if (session.bytes_received != expected) {
                        printf("⚠ Byte mismatch! You received %zu, but remote says %zu\n", expected, session.bytes_received);
                    }
                }
            }
        }

        // Outgoing message from user input
        if (FD_ISSET(STDIN_FILENO, &fds)) {
            if (fgets(send_buf, sizeof(send_buf), stdin) == NULL) {
                printf("Exiting.\n");
                break;
            }

            // Strip newline from input
            send_buf[strcspn(send_buf, "\n")] = '\0';

            ChatMessage msg = {0};
            strncpy(msg.username, session.username, sizeof(msg.username));
            strncpy(msg.hostname, session.hostname, sizeof(msg.hostname));
            msg.chat_start_time = session.start_time;
            strncpy(msg.message_text, send_buf, sizeof(msg.message_text));

            // Update byte counters
            session.bytes_sent += strlen(msg.message_text);
            msg.bytes_sent = session.bytes_sent;

            char packet[BUFFER_SIZE];
            if (format_message(&msg, packet, sizeof(packet)) == 0) {
                udp_send(sockfd, packet, strlen(packet), peer_ip, peer_port);
            }
        }
    }

    close(sockfd); // Clean up socket
    return 0;
}
