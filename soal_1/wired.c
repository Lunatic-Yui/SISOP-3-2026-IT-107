#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>
#include <time.h>
#include "protocol.h"

#define MAX_CLIENTS 100

typedef struct {
    int socket;
    char username[50];
    int is_admin;
} Client;

Client clients[MAX_CLIENTS];
int server_fd;
time_t start_time;

void write_log(const char* role, const char* status_or_msg) {
    FILE *f = fopen("history.log", "a");
    if (!f) return;
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char time_str[30];
    strftime(time_str, 30, "%Y-%m-%d %H:%M:%S", tm_info);
    fprintf(f, "[%s] [%s] [%s]\n", time_str, role, status_or_msg);
    fclose(f);
}

void broadcast(const char *message, int sender_sd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        // Jangan kirim ke diri sendiri, dan jangan kirim ke Admin
        if (clients[i].socket != 0 && clients[i].socket != sender_sd && !clients[i].is_admin) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
}

int main() {
    int activity, i, valread, sd, max_sd;
    struct sockaddr_in address;
    fd_set readfds;
    char buffer[1024];

    start_time = time(NULL);

    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = 0;
        clients[i].is_admin = 0;
        memset(clients[i].username, 0, 50);
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(WIRED_PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed"); return 1;
    }
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed"); return 1;
    }

    printf("[Sistem] The Wired aktif di %s:%d\n", WIRED_IP, WIRED_PORT);
    write_log("System", "SERVER ONLINE");

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = clients[i].socket;
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_sd) max_sd = sd;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(server_fd, &readfds)) {
            int new_socket = accept(server_fd, NULL, NULL);
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket == 0) {
                    clients[i].socket = new_socket;
                    break;
                }
            }
        }

        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = clients[i].socket;
            if (FD_ISSET(sd, &readfds)) {
                memset(buffer, 0, 1024);
                valread = read(sd, buffer, 1024);
                
                if (valread == 0 || strcmp(buffer, EXIT_CMD) == 0) {
                    char log_msg[100];
                    if (clients[i].is_admin) {
                        snprintf(log_msg, sizeof(log_msg), "User 'The Knights' disconnected");
                    } else {
                        snprintf(log_msg, sizeof(log_msg), "User '%s' disconnected", clients[i].username);
                    }
                    if(strlen(clients[i].username) > 0) write_log("System", log_msg);
                    
                    close(sd);
                    clients[i].socket = 0;
                    clients[i].is_admin = 0;
                    memset(clients[i].username, 0, 50);
                } else {
                    buffer[strcspn(buffer, "\r\n")] = 0; 
                    
                    if (strncmp(buffer, "LOGIN:", 6) == 0) {
                        char *name = buffer + 6;
                        int dup = 0;
                        for(int j=0; j<MAX_CLIENTS; j++) {
                            if(clients[j].socket != 0 && strcmp(clients[j].username, name) == 0) dup=1;
                        }
                        
                        if (dup) send(sd, "ERR_DUP", 7, 0);
                        else {
                            strcpy(clients[i].username, name);
                            send(sd, "OK", 2, 0);
                            
                            char log_msg[100];
                            snprintf(log_msg, sizeof(log_msg), "User '%s' connected", name);
                            write_log("System", log_msg);
                        }
                    } else if (strncmp(buffer, "ADMIN:", 6) == 0) {

                        strcpy(clients[i].username, "The Knights");
                        clients[i].is_admin = 1;
                        send(sd, "OK_ADMIN", 8, 0);
                        write_log("System", "User 'The Knights' connected");
                    } else if (clients[i].is_admin) {
                        if (strcmp(buffer, "CMD:USERS") == 0) {
                            int count = 0;
                            for (int j = 0; j < MAX_CLIENTS; j++) {
                                if (clients[j].socket != 0 && clients[j].is_admin == 0 && strlen(clients[j].username) > 0) count++;
                            }
                            char resp[100];
                            snprintf(resp, sizeof(resp), "[Sistem] Total Entitas Aktif: %d", count);
                            send(sd, resp, strlen(resp), 0);
                            write_log("Admin", "RPC_GET_USERS");
                        } else if (strcmp(buffer, "CMD:UPTIME") == 0) {
                            double seconds = difftime(time(NULL), start_time);
                            char resp[100];
                            snprintf(resp, sizeof(resp), "[Sistem] Waktu Aktif Server: %.0f detik", seconds);
                            send(sd, resp, strlen(resp), 0);
                            write_log("Admin", "RPC_GET_UPTIME");
                        } else if (strcmp(buffer, "CMD:SHUTDOWN") == 0) {
                            write_log("Admin", "RPC_SHUTDOWN");
                            write_log("System", "EMERGENCY SHUTDOWN INITIATED");
                            broadcast("\n[Sistem] Server dimatikan secara darurat oleh The Knights.", sd);
                            exit(0);
                        }
                    } else {
                        char msg[1100];
                        sprintf(msg, "[%s]: %s", clients[i].username, buffer);
                        broadcast(msg, sd);
                        
                        char log_msg[1100];
                        snprintf(log_msg, sizeof(log_msg), "[%s]: %s", clients[i].username, buffer);
                        write_log("User", log_msg);
                    }
                }
            }
        }
    }
    return 0;
}