#include <gtk/gtk.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "../common/protocol.h"
#include "../common/transport.h"
#include "../common/session.h"

#define MAX_BUFFER 2048

typedef struct {
    GtkWidget *window;
    GtkWidget *text_view;
    GtkWidget *entry;
    GtkTextBuffer *text_buffer;
    int sockfd;
    char peer_ip[INET_ADDRSTRLEN];
    unsigned short peer_port;
    ChatSession session;
} GuiContext;

// Global context for GTK-safe message updates
static GuiContext *global_ctx = NULL;

// Signal to control background thread
static volatile int keep_running = 1;

// Forward declarations
void append_text(GuiContext *ctx, const char *msg);
gboolean append_text_wrapper(gpointer data);

// Append text to the text view
void append_text(GuiContext *ctx, const char *msg) {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(ctx->text_buffer, &end);
    gtk_text_buffer_insert(ctx->text_buffer, &end, msg, -1);
    gtk_text_buffer_insert(ctx->text_buffer, &end, "\n", -1);
}

// Called from GTK thread to safely append messages
gboolean append_text_wrapper(gpointer data) {
    if (global_ctx && data) {
        append_text(global_ctx, (const char *)data);
    }
    g_free(data);
    return FALSE;
}

// Background receiver thread
void *receiver_thread(void *arg) {
    GuiContext *ctx = (GuiContext *)arg;
    char buffer[MAX_BUFFER];

    while (keep_running) {
        char sender_ip[INET_ADDRSTRLEN];
        unsigned short sender_port;

        int received = udp_receive(ctx->sockfd, buffer, sizeof(buffer), sender_ip, &sender_port);
        if (received > 0) {
            ChatMessage msg;
            if (parse_message(buffer, &msg) == 0) {
                ctx->session.bytes_received += strlen(msg.message_text);

                char line[MAX_BUFFER];
                snprintf(line, sizeof(line), "[%s@%s] %s", msg.username, msg.hostname, msg.message_text);
                g_idle_add((GSourceFunc)append_text_wrapper, g_strdup(line));
                
                char time_str[64];
                time_t chat_time = msg.chat_start_time;
                strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&chat_time));

                char time_line[MAX_BUFFER];
                snprintf(time_line, sizeof(time_line), "🕒 Chat started at: %s", time_str);
                g_idle_add((GSourceFunc)append_text_wrapper, g_strdup(time_line));
                char bytes_line[MAX_BUFFER];
                char track_line[MAX_BUFFER];
                snprintf(track_line, sizeof(track_line),"📥 Local bytes %zu bytes", ctx->session.bytes_received); 
                g_idle_add((GSourceFunc)append_text_wrapper, g_strdup(track_line));
                snprintf(bytes_line, sizeof(bytes_line), "🔢 %s has sent %zu bytes", msg.username, msg.bytes_sent);
                g_idle_add((GSourceFunc)append_text_wrapper, g_strdup(bytes_line));

                if (ctx->session.bytes_received != msg.bytes_sent) {
                    g_idle_add((GSourceFunc)append_text_wrapper, g_strdup("⚠ Byte mismatch detected!"));
                }
            }
        } else {
            usleep(1000); // Slight delay to avoid CPU spin if socket fails
        }
    }

    return NULL;
}

// Called when "Send" button is clicked
void on_send_clicked(GtkButton *button, gpointer user_data) {
    (void)button; // Suppress unused warning
    GuiContext *ctx = (GuiContext *)user_data;

    const char *text = gtk_entry_get_text(GTK_ENTRY(ctx->entry));
    if (strlen(text) == 0) return;

    ChatMessage msg = {0};
    strncpy(msg.username, ctx->session.username, MAX_USERNAME_LEN);
    strncpy(msg.hostname, ctx->session.hostname, MAX_HOSTNAME_LEN);
    msg.chat_start_time = ctx->session.start_time;
    strncpy(msg.message_text, text, MAX_MESSAGE_LEN);

    ctx->session.bytes_sent += strlen(msg.message_text);
    msg.bytes_sent = ctx->session.bytes_sent;

    char buffer[MAX_BUFFER];
    if (format_message(&msg, buffer, sizeof(buffer)) == 0) {
        udp_send(ctx->sockfd, buffer, strlen(buffer), ctx->peer_ip, ctx->peer_port);

        char line[MAX_BUFFER];
        snprintf(line, sizeof(line), "[You] %s", msg.message_text);
        append_text(ctx, line);
    }

    gtk_entry_set_text(GTK_ENTRY(ctx->entry), "");
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Usage: %s <username> <local_port> <peer_ip> <peer_port> <hostname>\n", argv[0]);
        return 1;
    }

    const char *username = argv[1];
    unsigned short local_port = (unsigned short)atoi(argv[2]);
    const char *peer_ip = argv[3];
    unsigned short peer_port = (unsigned short)atoi(argv[4]);
    const char *hostname = argv[5];

    gtk_init(&argc, &argv);

    static GuiContext ctx = {0};
    global_ctx = &ctx;

    strncpy(ctx.session.username, username, sizeof(ctx.session.username));
    strncpy(ctx.session.hostname, hostname, sizeof(ctx.session.hostname));
    ctx.session.start_time = time(NULL);
    strncpy(ctx.peer_ip, peer_ip, sizeof(ctx.peer_ip));
    ctx.peer_port = peer_port;

    ctx.sockfd = udp_init_socket(local_port);
    if (ctx.sockfd < 0) {
        fprintf(stderr, "Failed to bind socket\n");
        return 1;
    }

    ctx.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(ctx.window), "UDP Chat GUI");
    gtk_window_set_default_size(GTK_WINDOW(ctx.window), 400, 300);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(ctx.window), vbox);

    ctx.text_view = gtk_text_view_new();
    ctx.text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ctx.text_view));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(ctx.text_view), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), ctx.text_view, TRUE, TRUE, 0);

    ctx.entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), ctx.entry, FALSE, FALSE, 0);

    GtkWidget *send_button = gtk_button_new_with_label("Send");
    gtk_box_pack_start(GTK_BOX(vbox), send_button, FALSE, FALSE, 0);

    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_clicked), &ctx);
    g_signal_connect(ctx.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receiver_thread, &ctx);

    gtk_widget_show_all(ctx.window);
    gtk_main();

    // Clean shutdown
    keep_running = 0;
    pthread_join(recv_thread, NULL);
    close(ctx.sockfd);

    return 0;
}
