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

```c
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

1. Program wired jalan
![alt_text](/assets/soal_1/wired.png)

2. Membuat akun untuk user 1
![alt_text](/assets/soal_1/user_1.png)

3. Membuat akun untuk user 2 dengan username sama
![alt_text](/assets/soal_1/handling_1.png)

4. Membuat akun untuk user 2
![alt_text](/assets/soal_1/user_2.png)

5. Ngobrol
![alt_text](/assets/soal_1/ngobrol.png)

6. Disconnect user 1
![alt_text](/assets/soal_1/dc.png)

7. Admin masuk
![alt_text](/assets/soal_1/admin.png)

8. Command 1 dan 2
![alt_text](/assets/soal_1/command_1.png)

9. Command 3 dan hasil dari user
![alt_text](/assets/soal_1/command_2.png)

![alt_text](/assets/soal_1/pemadaman.png)

10. Mencoba menjalankan navi ketika server mati

![alt_text](/assets/soal_1/server_mati.png)

11. Result history.lognya

![alt_text](/assets/soal_1/result.png)

12. Menjalankan wired ketika wired yang lain aktif

![alt_text](/assets/soal_1/handling_3.png)

#### Kendala

Tidak ada kendala

### Soal 2 

** Makefile **

Diberikan file `Makefile` yang mengcompile file eternal dan orion secara langsung.

Selanjutnya

** arena.h **

Untuk kodingannya seperti ini:

```c
#ifndef ARENA_H
#define ARENA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>
#include <termios.h>

#define SHM_KEY 0x1234
#define MSG_KEY 0x5678

#define MAX_USERS 100
#define LOG_MAX 5

typedef struct {
    char username[50];
    char password[50];
    int gold;
    int lvl;
    int xp;
    int bonus_dmg_weapon;
    char weapon_name[50];
} Player;

struct msg_buffer {
    long msg_type;
    char msg_text[100];
    int client_pid;
};

typedef struct {
    int is_matchmaking;
    int player1_pid;
    int player2_pid;
    
    int is_battle_active;
    Player p1_stats;
    Player p2_stats;
    int p1_current_hp;
    int p2_current_hp;
    
    char combat_logs[LOG_MAX][100];
    int log_index;

    sem_t battle_mutex;
} BattleArena;


#define BASE_DMG 10
#define BASE_HP 100

typedef struct {
    char name[20];
    int price;
    int dmg;
} Weapon;

Weapon armory[5] = {
    {"Wood Sword", 100, 5},
    {"Iron Sword", 300, 15},
    {"Steel Axe", 600, 30},
    {"Demon Blade", 1500, 60},
    {"God Slayer", 5000, 150}
};

#endif
```

Nah dalam kode ini ada beberapa fungsi seperti fungsi untuk mendefinisikan alamat memorinya yang berbasis hexadecimal lalu juga mendefinisikan maksimum dari user dan lognya. Kemudian menyediakan sebuah data dari player untuk menyimpan statnya. Lalu juga ada fungsi `msg_buffer` yang dimana sebagai tempat komunikasi untuk antreannya. Lalu sinkronisasi data pada bagian matchmakingnya. Nah dalam kasus ini menggunakan memori bersama dan melacak masing-masing pid dari kedua player. Dan terakhir data persenjataan yang mana menyimpan beberapa stat seperti nama senjata, harga, dan dmg dari senjata tersebut. Selanjutnya

** orion.c **

nah pada file ini adalah sebagai tempat server untuk menghubungkan header file arena dengan file eternal. Pertama-pertama 

```c
#include "arena.h"
```

untuk mengimport fungsi yang dibutuhkan ke dalam file orion.c. Selanjutnya

```c
int main() {

int shmid = shmget(SHM_KEY, sizeof(BattleArena), IPC_CREAT | 0666);
    BattleArena *arena = (BattleArena *)shmat(shmid, NULL, 0);

    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);

    memset(arena, 0, sizeof(BattleArena));
    sem_init(&arena->battle_mutex, 1, 1); 
```

Nah pada kode ini membuat arena virtual dengan cara meminjam memori. Nah dari ini memorinya tersebut menjadi memori bersama. Lalu ada fungsi msgget untuk bisa mendaftar player. Kemudian fungsi memset ini membersihkan memori yang sudah dipakai (sebelumnya) untuk bisa dipakai kembali. Dan juga kode sem_init ini untuk mengendalikan dari pergerakan player agar tidak terjadi race condition. Ini juga mencegah data tidak corrupt. Selanjutnya

```c
printf("Orion is ready (PID: %d)\n", getpid());

    struct msg_buffer message;
    int waiting_pid = 0;

while (1) {

        msgrcv(msgid, &message, sizeof(message) - sizeof(long), 1, 0);
```

adalah kode yang menjalankan untuk pengiriman pesan matchmaking dan menunggu player lain untuk bisa bermain. Selanjutnya

```c
if (strcmp(message.msg_text, "CANCEL") == 0) {
            printf("Prajurit %d batal antre (Timeout/Lawan Bot)\n", message.client_pid);
            if (waiting_pid == message.client_pid) {
                waiting_pid = 0; 
                arena->is_matchmaking = 0;
            }
            continue; 
        }
```

kode ini menjalankan ketika dari player cancel untuk matchmakingnya atau udh melewati timeoutnya maka dia akan melawan bot dan is_waitingnya menjadi 0. Selanjutnya

```c
if (waiting_pid == 0) {

            waiting_pid = message.client_pid;
            arena->player1_pid = waiting_pid;
            arena->is_matchmaking = 1;
            
            message.msg_type = message.client_pid;
            strcpy(message.msg_text, "WAIT");
            msgsnd(msgid, &message, sizeof(message) - sizeof(long), 0);
        }
```

nah dalam kode ini ketika ada player memulai matchmaking maka is waitnya jadi 1 dan menyuruh playernya menunggu. Lalu

```c
else {
            arena->player2_pid = message.client_pid;
            arena->is_matchmaking = 0;
            arena->is_battle_active = 1;

            message.msg_type = waiting_pid;
            strcpy(message.msg_text, "START");
            msgsnd(msgid, &message, sizeof(message) - sizeof(long), 0);

            message.msg_type = message.client_pid;
            strcpy(message.msg_text, "START");
            msgsnd(msgid, &message, sizeof(message) - sizeof(long), 0);

            printf("Pertempuran dimulai: %d VS %d\n", waiting_pid, message.client_pid);
            waiting_pid = 0; 
        }
    }

    shmdt(arena);
    return 0;
}
```

adalah koding yang menjalankan ketika sudah ada player 1 menunggu dan ada player lain memulai matchmaking maka dia menjadi player 2. Nah karena udh ada player maka battle dimulai dan is_waitingnya menjadi 0

Error handlingnya:

```c
if (strcmp(message.msg_text, "CANCEL") == 0) {
            printf("Prajurit %d batal antre (Timeout/Lawan Bot)\n", message.client_pid);
            if (waiting_pid == message.client_pid) {
                waiting_pid = 0; 
                arena->is_matchmaking = 0;
            }
            continue; 
        }
```

Nah disini error handling ketika player tidak menemukan lawan ataupun kena timeout, maka waiting_pidnya menjadi 0 bukan 1 untuk menghindari error. Selanjutnya file

** eternal.c **

```c
#include "arena.h"
#include <time.h>
#include <stdlib.h> 

Player currentPlayer;
BattleArena *arena;
int msgid;

typedef struct {
    char username[50];
    char time_str[10];
    char opponent[50];
    char result[10];
    int xp;
} MatchRecord;

void clear_screen() { printf("\e[1;1H\e[2J"); }

void print_banner() {
    clear_screen();
    printf(" |__ ) /\\|__ _|_ _|| |____| / _\\| __|\n");
    printf(" | _ \\/ _ \\| | | | | |__ | | | || |__\n");
    printf(" |___/_/ \\_\\_| |_| |____|___|\\___/|___|\n\n");
}
```

Untuk setupun awal pada file eternal.cnya. Disini terdapat data filenya lalu ada ascii text. Selanjutnya

```c
void save_player() {
    FILE *f = fopen("prajurit.bin", "rb+");
    if (!f) return;
    Player temp;
    while (fread(&temp, sizeof(Player), 1, f)) {
        if (strcmp(temp.username, currentPlayer.username) == 0) {
            fseek(f, -sizeof(Player), SEEK_CUR);
            fwrite(&currentPlayer, sizeof(Player), 1, f);
            break;
        }
    }
    fclose(f);
}
```

adalah kodingan yang menyimpan stat player yang sudah dibikin dan disimpan kepada file `prajurit.bin`. Lalu ada juga

```c
void save_history(const char* opp, const char* res, int xp) {
    FILE *f = fopen("history.bin", "ab");
    if (!f) return;
    
    MatchRecord rec;
    strcpy(rec.username, currentPlayer.username);
    strcpy(rec.opponent, opp);
    strcpy(rec.result, res);
    rec.xp = xp;

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(rec.time_str, sizeof(rec.time_str), "%02d:%02d", tm.tm_hour, tm.tm_min);

    fwrite(&rec, sizeof(MatchRecord), 1, f);
    fclose(f);
}
```

adalah menyimpan history file player dengan mencatat exp, musuh, dan hasilnya serta waktunya ke dalam file `history.bin`. Selanjutnya

```c
void do_armory() {
    int prices[] = {0, 100, 300, 600, 1500, 5000};
    int dmgs[] = {0, 5, 15, 30, 60, 150};
    char* names[] = {"", "Wood Sword", "Iron Sword", "Steel Axe", "Demon Blade", "God Slayer"};

    while (1) {
        clear_screen();
        printf("══ ARMORY ══\n");
        printf("Gold: %d\n", currentPlayer.gold);
        printf("1. Wood Sword   | 100 G  | +5   Dmg\n");
        printf("2. Iron Sword   | 300 G  | +15  Dmg\n");
        printf("3. Steel Axe    | 600 G  | +30  Dmg\n");
        printf("4. Demon Blade  | 1500 G | +60  Dmg\n");
        printf("5. God Slayer   | 5000 G | +150 Dmg\n");
        printf("0. Back | Choice: ");
        
        int c;
        if (scanf("%d", &c) != 1) { 
            while(getchar() != '\n'); 
            continue; 
        }

        if (c == 0) break;
        
        if (c >= 1 && c <= 5) {
            if (currentPlayer.gold >= prices[c]) {
                currentPlayer.gold -= prices[c];

                if (dmgs[c] > currentPlayer.bonus_dmg_weapon) {
                    currentPlayer.bonus_dmg_weapon = dmgs[c];
                    strcpy(currentPlayer.weapon_name, names[c]);
                }
                save_player();
                printf("\033[0;32mWeapon purchased!\033[0m\n");
            } else {
                printf("\033[0;31mNot enough gold!\033[0m\n");
            }
            sleep(1);
        }
    }
}
```

adalah tempat beli armory yaitu harga, dmg, nama weaponnya dan mengecheck apakah player yang mempunyai gold yang cukup atau tidak untuk membeli armory yang sudah tersedia. Selanjutnya

```c
void do_history() {
    clear_screen();
    printf("┌───────────── MATCH HISTORY ─────────────┐\n");
    printf("│ Time  | Opponent        | Res  | XP     │\n");
    printf("├─────────────────────────────────────────┤\n");
    
    FILE *f = fopen("history.bin", "rb");
    if (f) {
        MatchRecord rec;
        while (fread(&rec, sizeof(MatchRecord), 1, f)) {
            if (strcmp(rec.username, currentPlayer.username) == 0) {
                char res_color[30];
                if (strcmp(rec.result, "WIN") == 0) strcpy(res_color, "\033[0;32mWIN \033[0m");
                else strcpy(res_color, "\033[0;31mLOSS\033[0m");

                printf("│ %-5s | %-15s | %s | +%-2d XP │\n", rec.time_str, rec.opponent, res_color, rec.xp);
            }
        }
        fclose(f);
    }
    printf("└─────────────────────────────────────────┘\n");
    
    int c; while ((c = getchar()) != '\n' && c != EOF);
    printf("Press [ENTER] to return..."); 
    getchar();
}
```

nah dalam kodingan ini itu memberikan fungsi untuk melihat hasil pertandingan. Kalau sebelumnya membuat hasil pertandingan dari player nah kalau ini menunjukkan hasil history dari player itu sendiri. Selanjutnya

```c
void* display_handler(void* arg) {
    while (arena->is_battle_active) {
        clear_screen();
        char* opp_name = (getpid() == arena->player1_pid) ? arena->p2_stats.username : arena->p1_stats.username;
        char* opp_weap = (getpid() == arena->player1_pid) ? arena->p2_stats.weapon_name : arena->p1_stats.weapon_name;

        printf("========== ARENA ==========\n");
        printf("%s | Weapon: %s\n", opp_name, opp_weap);
        printf("HP: [%d/100]\n", (getpid() == arena->player1_pid) ? arena->p2_current_hp : arena->p1_current_hp);
        printf("---------------------------\n");
        printf("VS\n");
        printf("---------------------------\n");
        printf("%s | Weapon: %s\n", currentPlayer.username, currentPlayer.weapon_name);
        printf("HP: [%d/100]\n", (getpid() == arena->player1_pid) ? arena->p1_current_hp : arena->p2_current_hp);
        printf("\nCombat Log:\n");
        for (int i = 0; i < LOG_MAX; i++) printf("> %s\n", arena->combat_logs[i]);
        
        printf("\n>>> AUTO-BATTLE MODE ACTIVE <<<\n");
        usleep(200000); 
    }
    return NULL;
}
```

adalah kodingan yang menjalankan thread untuk menampilkan pertandingannya. Nah kodingannya ini bakal selalu update setiap ada informasi update yang dijalankan seperti darah berkurang, dmg ke player atau bot, dll. Selanjutnya

```c
void do_battle() {
    sem_wait(&arena->battle_mutex);
    if (getpid() == arena->player1_pid) {
        arena->p1_stats = currentPlayer;
        arena->p1_current_hp = 100 + (currentPlayer.xp / 10);
    } else {
        arena->p2_stats = currentPlayer;
        arena->p2_current_hp = 100 + (currentPlayer.xp / 10);
    }
    sem_post(&arena->battle_mutex);

    pthread_t tid;
    pthread_create(&tid, NULL, display_handler, NULL);

    while (arena->is_battle_active) {
        sleep(1); 
        if (!arena->is_battle_active) break; 

        int total_dmg = BASE_DMG + (currentPlayer.xp / 50) + currentPlayer.bonus_dmg_weapon;
        int is_ult = 0;
        
        if (currentPlayer.bonus_dmg_weapon > 0 && (rand() % 100 < 25)) {
            total_dmg *= 3; 
            is_ult = 1;
        }
        
        sem_wait(&arena->battle_mutex);
        
        int my_hp = (getpid() == arena->player1_pid) ? arena->p1_current_hp : arena->p2_current_hp;
        
        if (my_hp > 0 && arena->is_battle_active) {
            if (getpid() == arena->player1_pid) arena->p2_current_hp -= total_dmg;
            else arena->p1_current_hp -= total_dmg;
            
            char move_name[15];
            strcpy(move_name, is_ult ? "Ultimates" : "hits");
            snprintf(arena->combat_logs[arena->log_index % LOG_MAX], 100, "%s %s for %d!", currentPlayer.username, move_name, total_dmg);
            arena->log_index++;
        }
        sem_post(&arena->battle_mutex);
        
        if (arena->p1_current_hp <= 0 || arena->p2_current_hp <= 0) {
            arena->is_battle_active = 0;
        }
    }
    pthread_join(tid, NULL);
    
    char opp_name[50];
    strcpy(opp_name, (getpid() == arena->player1_pid) ? arena->p2_stats.username : arena->p1_stats.username);

    int win = 0;
    if (getpid() == arena->player1_pid && arena->p1_current_hp > 0) win = 1;
    if (getpid() == arena->player2_pid && arena->p2_current_hp > 0) win = 1;

    if (win) {
        printf("\n\033[0;32m== VICTORY ==\033[0m\nBattle ended. Gold +120, XP +50\n");
        currentPlayer.gold += 120;
        currentPlayer.xp += 50;
        save_history(opp_name, "WIN", 50);
    } else {
        printf("\n\033[0;31m== DEFEAT ==\033[0m\nBattle ended. Gold +30, XP +15\n");
        currentPlayer.gold += 30;
        currentPlayer.xp += 15;
        save_history(opp_name, "LOSS", 15);
    }
    currentPlayer.lvl = 1 + (currentPlayer.xp / 100);
    save_player();

    int c; while ((c = getchar()) != '\n' && c != EOF);
    printf("Press [ENTER] to continue..."); getchar();
}
```

Nah dalam kodingan ini adalah program yang menjalankan auto battle dari player. Nah player 1 dan 2 akan terus bergantian menyerang dan melakukan update informasi dari darah masing-masing player dan memberikan dmg kepada masing-masing player. Nah jika salah satu player menang akan diberi gold tambahan lebih namun jika salah satu player kalah goldnya lebih dikit daripada yang menang. Nah dari hasil battle ini disimpan ke `history.bin`. Selanjutnya

```c
int is_bot_active = 0;
int player_hp_local, bot_hp_local;
char bot_combat_logs[LOG_MAX][100];
int bot_log_idx = 0;
```

adalah program yang menyimpan untuk stat bot dengan

```c
void* bot_display_handler(void* arg) {
    while (is_bot_active) {
        clear_screen();
        printf("========== ARENA ==========\n");
        printf("Wild Beast | Weapon: None\n");
        printf("HP: [%d/100]\n", bot_hp_local);
        printf("---------------------------\n");
        printf("VS\n");
        printf("---------------------------\n");
        printf("%s | Weapon: %s\n", currentPlayer.username, currentPlayer.weapon_name);
        printf("HP: [%d/%d]\n", player_hp_local, 100 + (currentPlayer.xp / 10));
        printf("\nCombat Log:\n");
        for (int i = 0; i < LOG_MAX; i++) printf("> %s\n", bot_combat_logs[i]);
        
        printf("\n>>> AUTO-BATTLE MODE ACTIVE <<<\n");
        usleep(200000); 
    }
    return NULL;
}
```

untuk memunculkan stat dari bot yang sudah dibikin dari darah, weapon, dan lognya. Lalu

```c
void* bot_logic_handler(void* arg) {
    while (is_bot_active && bot_hp_local > 0 && player_hp_local > 0) {
        sleep(2); 
        if (!is_bot_active) break;

        int bot_dmg = 15; 
        player_hp_local -= bot_dmg;

        snprintf(bot_combat_logs[bot_log_idx % LOG_MAX], 100, "Wild Beast hits you for %d!", bot_dmg);
        bot_log_idx++;

        if (player_hp_local <= 0) is_bot_active = 0;
    }
    return NULL;
}
```

untuk handling dari logic bot sendiri. Kemudian

```c
void do_bot_battle() {
    is_bot_active = 1;
    player_hp_local = 100 + (currentPlayer.xp / 10);
    bot_hp_local = 100;
    bot_log_idx = 0;
    memset(bot_combat_logs, 0, sizeof(bot_combat_logs));

    pthread_t disp_tid, bot_tid;
    pthread_create(&disp_tid, NULL, bot_display_handler, NULL);
    pthread_create(&bot_tid, NULL, bot_logic_handler, NULL); 

    while (is_bot_active) {
        sleep(1); 
        if (!is_bot_active) break;

        int total_dmg = BASE_DMG + (currentPlayer.xp / 50) + currentPlayer.bonus_dmg_weapon;
        int is_ult = 0;

        if (currentPlayer.bonus_dmg_weapon > 0 && (rand() % 100 < 25)) {
            total_dmg *= 3;
            is_ult = 1;
        }

        if (player_hp_local > 0) { 
            bot_hp_local -= total_dmg;
            char move_name[15];
            strcpy(move_name, is_ult ? "Ultimates" : "hits");
            snprintf(bot_combat_logs[bot_log_idx % LOG_MAX], 100, "%s %s Wild Beast for %d!", currentPlayer.username, move_name, total_dmg);
            bot_log_idx++;
        }

        if (bot_hp_local <= 0 || player_hp_local <= 0) is_bot_active = 0;
    }

    pthread_join(disp_tid, NULL);
    pthread_join(bot_tid, NULL);

    if (player_hp_local > 0) {
        printf("\n\033[0;32m== VICTORY ==\033[0m\nBattle ended. Gold +50, XP +20\n"); 
        currentPlayer.gold += 50;
        currentPlayer.xp += 20;
        save_history("Wild Beast", "WIN", 20);
    } else {
        printf("\n\033[0;31m== DEFEAT ==\033[0m\nBattle ended. Gold +10, XP +5\n");
        currentPlayer.gold += 10;
        currentPlayer.xp += 5;
        save_history("Wild Beast", "LOSS", 5);
    }
    currentPlayer.lvl = 1 + (currentPlayer.xp / 100);
    save_player();

    int c; while ((c = getchar()) != '\n' && c != EOF);
    printf("Press [ENTER] to continue..."); getchar();
}
```

adalah handling yang melakukan perlawanan dengan bot. Disini juga botnya melakukan secara automatic dan ada sistem win - lossnya juga dan hasilnya akan disimpan ke file `history.bin`.

```c
int main() {
    srand(time(NULL)); 

    int shmid = shmget(SHM_KEY, sizeof(BattleArena), 0666);
    arena = (BattleArena *)shmat(shmid, NULL, 0);
    msgid = msgget(MSG_KEY, 0666);

    int is_running = 1;

    while (is_running) {
        int is_logged_in = 0;

        while (!is_logged_in && is_running) {
            print_banner();
            printf("1. Register\n2. Login\n3. Exit\nChoice: ");
            
            int init_choice; 
            if (scanf("%d", &init_choice) != 1) { 
                while(getchar() != '\n'); 
                continue; 
            }

            if (init_choice == 1) {
                Player newP;
                printf("\nCREATE ACCOUNT\nUsername: "); scanf("%s", newP.username);
                printf("Password: "); scanf("%s", newP.password);
                
                int exists = 0;
                FILE *f = fopen("prajurit.bin", "rb");
                if (f) {
                    Player temp;
                    while (fread(&temp, sizeof(Player), 1, f)) {
                        if (strcmp(temp.username, newP.username) == 0) exists = 1;
                    }
                    fclose(f);
                }

                if (exists) {
                    printf("Username sudah terpakai!\n"); sleep(2);
                } else {
                    newP.gold = 150; newP.lvl = 1; newP.xp = 0;
                    newP.bonus_dmg_weapon = 0; strcpy(newP.weapon_name, "None");
                    f = fopen("prajurit.bin", "ab");
                    fwrite(&newP, sizeof(Player), 1, f);
                    fclose(f);
                    printf("\033[0;32mAccount created!\033[0m\n"); sleep(2);
                }
            } 
            else if (init_choice == 2) {
                char input_user[50], input_pass[50];
                printf("\nLOGIN\nUsername: "); scanf("%s", input_user);
                printf("Password: "); scanf("%s", input_pass);
                
                int found = 0;
                FILE *f = fopen("prajurit.bin", "rb");
                if (f) {
                    Player temp;
                    while (fread(&temp, sizeof(Player), 1, f)) {
                        if (strcmp(temp.username, input_user) == 0 && strcmp(temp.password, input_pass) == 0) {
                            currentPlayer = temp; 
                            found = 1; break;
                        }
                    }
                    fclose(f);
                }

                if (found) {
                    printf("\033[0;32mWelcome!\033[0m\n");
                    is_logged_in = 1; sleep(2);
                } else {
                    printf("Username atau Password salah!\n"); sleep(2);
                }
            } 
            else if (init_choice == 3) {
                is_running = 0; 
            }
        }

        while (is_logged_in) {
            clear_screen();
            printf("--- PROFILE ---\n");
            printf("Name : %-10s Lvl : %d\n", currentPlayer.username, currentPlayer.lvl);
            printf("Gold : %-10d XP  : %d\n", currentPlayer.gold, currentPlayer.xp);
            printf("---------------\n\n");
            
            printf("1. Battle\n2. Armory\n3. History\n4. Logout\nChoice: ");
            int choice; 
            if (scanf("%d", &choice) != 1) { 
                while(getchar() != '\n'); 
                continue; 
            }

            if (choice == 1) {
                struct msg_buffer msg;
                msg.msg_type = 1; msg.client_pid = getpid();
                strcpy(msg.msg_text, "MATCH"); 
                msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0);

                int time_elapsed = 0, match_found = 0;

                while (time_elapsed <= 35) {
                    if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), getpid(), IPC_NOWAIT) != -1) {
                        if (strcmp(msg.msg_text, "START") == 0) {
                            match_found = 1; break;
                        }
                    }
                    printf("\rSearching for an opponent... [%d/35 s]", time_elapsed);
                    fflush(stdout); sleep(1); time_elapsed++;
                }

                if (match_found) {
                    printf("\nOpponent found!\n"); sleep(1);
                    do_battle(); 
                } else {
                    printf("\nTimeout! Melawan Bot...\n"); sleep(1);
                    msg.msg_type = 1; msg.client_pid = getpid();
                    strcpy(msg.msg_text, "CANCEL");
                    msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0);
                    do_bot_battle(); 
                }
            } 
            else if (choice == 2) {
                do_armory();
            }
            else if (choice == 3) {
                do_history();
            }
            else if (choice == 4) {
                is_logged_in = 0; 
            }
        }
    }

    shmdt(arena);
    return 0;
}
```

Nah dalam kodingan main ini memberikan tampilan ui yaitu battel pada no 1, armory pada no 2, history pada no 3, dan logout dari menu pada no 4. 

Error handling:

```c
if (scanf("%d", &choice) != 1) { 
    while(getchar() != '\n'); 
    continue; 
}
```

adalah input sanitize agar user menginput angka bukan huruf. Selanjutnya

```c
FILE *f = fopen("prajurit.bin", "rb+");
if (!f) return;
```

adalah error handling kalau filenya tidak berhasil dibuka. Selanjutnya

```c
if (exists) {
                    printf("Username sudah terpakai!\n"); sleep(2);
                }
```

adalah mengecheck apakah usernamenya sudah dipakai atau belum. Selanjutnya

```c
if (currentPlayer.gold >= prices[c]) { ... } 
else { printf("\033[0;31mNot enough gold!\033[0m\n"); }
```

untuk mengecheck apakah goldnya cukup atau tidak. Jika tidak cukup maka barang tersebut tidak bisa dibeli. Selanjutnya

```c
else {
                    printf("\nTimeout! Melawan Bot...\n"); sleep(1);
                    msg.msg_type = 1; msg.client_pid = getpid();
                    strcpy(msg.msg_text, "CANCEL");
                    msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0);
                    do_bot_battle(); 
                }
```

Fungsi handler untuk mengcancel kalau sudah melewait timeoutnya yaitu 35 detik. Nah fungsi ini melawan bot. 

#### output

#### Kendala

Tidak ada kendala