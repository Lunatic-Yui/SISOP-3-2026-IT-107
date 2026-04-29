#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "protocol.h"

int sock;
int running = 1;
int is_admin = 0; 

void *receive_msg(void *arg) {
    char buf[1024];
    while (running) {
        memset(buf, 0, 1024);
        if (read(sock, buf, 1024) > 0) {
            if (is_admin) {
                printf("\r\033[K%s\nPerintah >> ", buf);
            } else {
                printf("\r\033[K%s\n> ", buf);
            }
            fflush(stdout);
        } else {
            printf("\r\033[K\n[Sistem] Koneksi ke The Wired terputus.\n");
            running = 0;
            exit(0);
        }
    }
    return NULL;
}

int main() {
    struct sockaddr_in server;
    char username[50], buf[1024];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Error pembuatan socket.\n"); return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(WIRED_PORT);
    if (inet_pton(AF_INET, WIRED_IP, &server.sin_addr) <= 0) {
        printf("IP dari protocol.h tidak valid\n"); return 1;
    }

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("[Sistem] Koneksi gagal! Pastikan server The Wired sudah menyala.\n"); 
        return 1;
    }

    while (1) {
        printf("Masukkan nama Anda: "); 
        
        fgets(username, 50, stdin);
        username[strcspn(username, "\r\n")] = 0; 
        
        if (strcmp(username, "The Knights") == 0) {
            printf("Masukkan Kata Sandi: ");
            char pass[50];
            fgets(pass, 50, stdin);
            pass[strcspn(pass, "\r\n")] = 0;
            
            sprintf(buf, "ADMIN:%s", pass);
        } else {
            sprintf(buf, "LOGIN:%s", username);
        }
        
        send(sock, buf, strlen(buf), 0);
        memset(buf, 0, 1024);
        
        int bytes_read = read(sock, buf, 1024);
        if (bytes_read <= 0) {
            printf("\n[Sistem] Gagal sinkronisasi, tidak ada respon dari server.\n");
            return 1;
        }
        
        if (strncmp(buf, "ERR_DUP", 7) == 0) {
            printf("[Sistem] Identitas '%s' sudah disinkronkan di The Wired.\n\n", username);
        } else if (strncmp(buf, "OK_ADMIN", 8) == 0) {
            printf("\n[Sistem] Otentikasi Berhasil. Hak akses Admin diberikan.\n");
            is_admin = 1;
            break;
        } else if (strncmp(buf, "OK", 2) == 0) {
            printf("\n--- Selamat datang di The Wired, %s\n ---", username);
            is_admin = 0;
            break;
        } else {
            printf("\n[Sistem] Autentikasi gagal. Password salah.\n\n");
        }
    }

    pthread_t tid;
    pthread_create(&tid, NULL, receive_msg, NULL);

    while (running) {
        if (is_admin) {
            printf("\n=== KONSOL THE KNIGHTS ===\n");
            printf("1. Periksa Entitas Aktif (Pengguna)\n");
            printf("2. Periksa Waktu Aktif Server\n");
            printf("3. Lakukan Pemutusan Darurat\n");
            printf("4. Putuskan sambungan\n");
            printf("Perintah >> ");
            
            char choice_str[10];
            fgets(choice_str, 10, stdin);
            int choice = atoi(choice_str);
            
            if (choice == 1) send(sock, "CMD:USERS", 9, 0);
            else if (choice == 2) send(sock, "CMD:UPTIME", 10, 0);
            else if (choice == 3) {
                send(sock, "CMD:SHUTDOWN", 12, 0);
                running = 0;
            }
            else if (choice == 4) {
                printf("[Sistem] Memutuskan koneksi dari The Wired...\n");
                send(sock, EXIT_CMD, strlen(EXIT_CMD), 0);
                running = 0;
            }
            usleep(500000); 
        } else {
            printf("> ");
            fgets(buf, 1024, stdin);
            buf[strcspn(buf, "\r\n")] = 0; 
            
            if (strlen(buf) > 0) {
                if (strcmp(buf, "/KELUAR") == 0 || strcmp(buf, "/exit") == 0) {
                    printf("[Sistem] Memutuskan koneksi dari The Wired...\n");
                    send(sock, EXIT_CMD, strlen(EXIT_CMD), 0);
                    running = 0;
                } else {
                    send(sock, buf, strlen(buf), 0);
                }
            }
        }
    }

    close(sock);
    return 0;
}