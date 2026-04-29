#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "protocol.h"

int sock;
int running = 1;

void *receive_msg(void *arg) {
    char buf[1024];
    while (running) {
        memset(buf, 0, 1024);
        if (read(sock, buf, 1024) > 0) {
            printf("\r\033[K%s\n> ", buf);
            fflush(stdout);
        }
    }
    return NULL;
}

int main() {
    struct sockaddr_in server;
    char username[50], buf[1024];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(WIRED_PORT);
    inet_pton(AF_INET, WIRED_IP, &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) return 1;

    while (1) {
        printf("Masukkan nama Anda: ");
        scanf("%s", username); getchar();
        if (strcmp(username, "The_Knights") == 0) {
            printf("Masukkan Kata Sandi: ");
            char pass[50]; scanf("%s", pass); getchar();
            sprintf(buf, "ADMIN:%s", pass);
        } else {
            sprintf(buf, "LOGIN:%s", username);
        }
        send(sock, buf, strlen(buf), 0);
        read(sock, buf, 1024);
        if (strncmp(buf, "OK", 2) == 0) break;
        printf("Gagal sinkronisasi!\n");
    }

    pthread_t tid;
    pthread_create(&tid, NULL, receive_msg, NULL);

    while (running) {
        printf("> ");
        fgets(buf, 1024, stdin);
        buf[strcspn(buf, "\n")] = 0;
        if (strcmp(buf, "/exit") == 0) {
            send(sock, EXIT_CMD, 5, 0);
            running = 0;
        } else {
            send(sock, buf, strlen(buf), 0);
        }
    }

    close(sock);
    return 0;
}