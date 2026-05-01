# SISOP-3-2026-IT-107

## Member

| Nama                   | NRP        |
| ---------------------- | ---------- |
| Yovi Prayudya Rizky Ramadhani | 5027251107 |

## Reporting

### Soal 1

#### Penjelasan

** protocol.h **

Kodenya seperti ini:

```h
#ifndef PROTOCOL_H
#define PROTOCOL_H

#define WIRED_IP "127.0.0.1"
#define WIRED_PORT 8080

#define EXIT_CMD "/exit"
#define ADMIN_PASS "protokol7"

#endif
```

Ini adalah setupnya untuk pertama dengan mendefinisikan file protocol H, dan port wirednya beserta admin pass dan command untuk exitnya. Nah dalam kode ini itu mendefinisikan dan menghubungkan file navi dengan ip yang sudah ditentukan beserta portnya. 

** wired.c **

Selanjutkan setup terlebih dahulu wired.cnya. Untuk awalnya seperti ini:

```c
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

```

Nah dalam kode ini menginclude semua library yang dibutuhkan dan file protocol.hnya. Selanjutnya mendefinisikan maksimum client yaitu 100. Terus membuat struct untuk membuat datanya dan adminnya. Kemudian ada fungsi `start_time` untuk mencatat kapan server ini dimulai. Selanjutnya

```c
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
        if (clients[i].socket != 0 && clients[i].socket != sender_sd && !clients[i].is_admin) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
}
```

adalah fungsi yang mencatat dan fungsi yang memunculkan pesan kepada layar lain. Fungsi mencatat sendiri ada di fungsi `write_log` dengan mode `a` (append) yang dimana dia akan menulisnya terus tanpa menghapus isi kontent yang sudah ditulis serta melihat filenya. Lalu juga dalam `write_log` juga ada fungsi format waktu yaitu mengambil `localtime` yang ada di device pengguna, memformatnya dalam bentuk tahun, bulan, hari, jam, menit, detik dan menulisnya ke dalam file. Untuk fungsi broadcastnya sendiri yaitu fungsi yang dimana ketika ada 2 user mengirim pesan dan terhubung, maka pesan ini akan muncul di layar masing-masing misalnya user 1 ngirim pesan ke user 2 maka pesan dari user 1 ini muncul ke layar user 2. Begitupun sebaliknya. Selanjutnya fungsi mainnya: 

```c
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
```

nah dalam fungsi yg main awal ini adalah membangun servernya. Ketika server dipanggil, array awalnya akan dikosongkan lalu membuat socket utamanya `server_fd` ini. Kemudian ada `setsockopt` itu untuk mencegah terjadinya nyangkut ketika servernya itu crash. Lalu ada `bind` dan `listen` untuk menempelkan ke port 8080 dan dengerin apakah ada client yang masuk atau tidak. Selanjutnya:

```c
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
```

Kode ini untuk skalabilitas. Ini buat menghemat cpu tanpa harus mencari satu-satu dan mendengarkan kalau misal ada yang memanggilnya. Kemudian

```c
if (FD_ISSET(server_fd, &readfds)) {
            int new_socket = accept(server_fd, NULL, NULL);
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket == 0) {
                    clients[i].socket = new_socket;
                    break;
                }
            }
        }
```

adalah lanjutan dari kode sebelumnya. Dalam kode ini ketika ada gerbang utama `server_fd`nya, dia akan menerima dan mencari array yang kosong dalam clientnya itu. Selanjutnya

```c
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
                    }
```

adalah kode yang mengatur socketnya. Kalau ada socket lain yang masuk, ia akan dicheck terlebih dahulu apakah namanya sama atau tidak (error handling). Jika sama, dia menolaknya. Jika tidak, maka dia akan menerimanya dan mendengarkan socket yang lain itu. Ketika ada command `/exit` maka dia akan terputus dan arraynya pun dikosongkan. Selanjutnya:

```c
else if (strncmp(buffer, "ADMIN:", 6) == 0) {

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
                    }
```

Dalam kode ini dikhususkan buat admin yang bernama `The Knights`. Nah dalam kode ini ketika username yang diinput adalah the knights, dia akan meminta password dan bikin `is_admin` yang awalnya adalah 0 menjadi 1. Nah disini juga ada fungsi remote procedure call (RPC) khusus admin yang bisa memantau user, uptime, dan mematikan server. Dan terakhir

```c
else {
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
```

Adalah kode kalau misal bukan admin, dia hanya mengirim chat biasa tersebut.

Untuk error handling sendiri:

```c
FILE *f = fopen("history.log", "a");
if (!f) return;
``` 

Ketika file tersebut gagal dibuka, dia akan mengembalikan hasilnya null tanpa bikin `segmentation fault`.

```c
if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Bind failed"); return 1;
}
if (listen(server_fd, 10) < 0) {
    perror("Listen failed"); return 1;
}
```

Ketika port 8080 lagi tidak bisa dibuka (misal yang lama masih tidak dihapus) maka dia akan mengirim pesan errornya dan menutup servernya

```c
valread = read(sd, buffer, 1024);
if (valread == 0 || strcmp(buffer, EXIT_CMD) == 0) {
    close(sd);
    clients[i].socket = 0;
}
```

akan menghandle ketika client ini memutuskan diri dari koneksi server (contohnya: ctrl + c). Nah dia akan mengosongkan socketnya dan menutupnya.

```c
if (dup) send(sd, "ERR_DUP", 7, 0);
```

Ini akan mengecheck apakah namanya sama atau tidak. Jika sama akan mengirim errornya.

```c
buffer[strcspn(buffer, "\r\n")] = 0;
```

Ini hanya membersihkan carriage returnnya saja. Next filenya:

** navi.c **

Pertama-tama untuk setupnya seperti ini:

```
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
```

Dengan mengimport library yang dibutuhkan dan mendefinisikan running = 1 dan admin = 0. Lalu selanjutnya

```c
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
```

adalah fungsi yang menerima pesan secara asinkron pada thread tersebut. Lalu pada fungsi main:

```c
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
```

yaitu menjalankan fungsi setup koneksinya. Nah disini terdapat `inet_pton` yang mengubah string '127.0.0.1' menjadi angka binary yang dipahami oleh sistem dan menghubungkan dengan fungsi `connect` menuju alamt yang udah ditentukan pada protocol.h. Selanjutnya:

```c
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
```

nah pada program ini akan menjalankan identitas dari usernya. Ketika user identitasnya itu bukan `the knights` yang mana merupakan sebuah admin dalam soal ini, dia akan menjalankan socket sebagai user biasa. Tapi kalau sebagai `the knights` maka password akan muncul yang meminta player memasukkan passwordnya dan mendapatkan akses admin dengan identitas `the knights`. Lalu selanjutnya:

```c
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
```

akan dijalankan ketika di awal tadi pada saat fase identitas sudah selesai dijalankan maka akan menjalankan fungsi `pthread_create` yang membuat sebuah thread untuk dijalankan 2 jalur yaitu jalur untuk membaca pesannya dan jalur untuk membaca input dari user. Nah untuk kasus ini adalah kasus user biasa atau bisa dibilang bukan `the knights` itu sendiri. Untuk kasus admin atau `the knights` sendiri maka `is_admin` yang awalnya 0 menjadi 1 dan menjalankan konsol untuk muncul di permukaannya. Nah terdapat 4 pilihan dan 4 pilihan ini akan dimapping menjadi kode khusus seperti `CMD:USERS`nya itu. Nah disini ada fungsi `usleep` yang dimana mencegah race condition untuk mencegah menu uinya tidak tercetak sebelum balasan dari perintah si admin ini di eksekusi. 

** error handling pada navi.c **

Terdapat error handling di file ini yaitu: 

```c
sock = socket(AF_INET, SOCK_STREAM, 0);
if (sock < 0) {
    printf("Error pembuatan socket.\n"); 
    return 1; 
}
```

nah error handling pertama ini mengecheck apakah sistem operasinya ini bisa mengalokasikan memori yang sudah ditentukan atau tidak. Jika tidak, program tersebut akan diterminasi dengan cepat. Selanjutnya

```c
if (inet_pton(AF_INET, WIRED_IP, &server.sin_addr) <= 0) {
    printf("IP dari protocol.h tidak valid\n"); 
    return 1; 
}
```

adalah kode untuk mengecheck apakah IP yang diubah dari string menjadi biner dan di terjemahkan kembali menjadi string ini valid atau tidak. jika tidak, program akan diterminasi. Selanjutnya

```c
if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
    printf("[Sistem] Koneksi gagal! Pastikan server The Wired sudah menyala.\n"); 
    return 1; 
}
```

Nah dalam kode ini akan menjalankan pengechekan apakah program klien ini menjalankan deadlock atau tidak ketika terhubung ke sebuah port yang sudah ditentukan. Jika iya dan port itu belum jalan, program akan dimatikan agar tidak hang saat dijalankan. Lalu

```c
int bytes_read = read(sock, buf, 1024);
if (bytes_read <= 0) {
    printf("\n[Sistem] Gagal sinkronisasi, tidak ada respon dari server.\n");
    return 1; 
}
```

akan mengecheck apakah pada saat mengisi identitas ada respond dari host atau tidak. jika tidak, fungsi read akan menangkap sinyal itu dan program akan dimatikan. Selanjutnya

```c
if (strncmp(buf, "ERR_DUP", 7) == 0) {
    printf("[Sistem] Identitas '%s' sudah disinkronkan di The Wired.\n\n", username);
```

dan

```c
} else {
    printf("\n[Sistem] Autentikasi gagal. Password salah.\n\n");
```

adalah fungsi yang mengecheck apakah terdapat username yang sama atau tidak. Jika sama, akan meminta username yang baru dan ketika password. Dan terakhir:

```c
} else {

    printf("\r\033[K\n[Sistem] Koneksi ke The Wired terputus.\n");
    running = 0;  utama
    exit(0);
```

adlaah fungsi yang dimana dia akan memutus koneksi ketika server tidak terhubung dan akan mengeluarkan dari thread. 
#### output

#### Kendala

Tidak ada kendala

### Soal 2 

#### output

#### Kendala

Tidak ada kendala