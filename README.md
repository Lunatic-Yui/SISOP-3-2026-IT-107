# SISOP-3-2026-IT-107

## Member

| Nama                   | NRP        |
| ---------------------- | ---------- |
| Yovi Prayudya Rizky Ramadhani | 5027251107 |

## Reporting

### Soal 1

#### Penjelasan

**buku_hutang.csv**

Untuk langkah pertama, download terlebih dahulu file `buku_hutang.csv`nya ini menggunakan command: `wget buku_hutang.csv "https://drive.google.com/uc?export=download&id=144hsFJGoAM5lbvBQjkTfZaD3GOil4yEw"`.

Selanjutnya membuat program bernama kasir_muthu menggunakan bahasa pemrograman c. File coding: [kasir_muthu.c](/soal_1/kasir_muthu.c).

Pertama-tama kita membuat setupnya terlebih dahulu

```c
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <unistd.h>

char *file = "buku_hutang.csv";
```

Ini memasukkan beberapa fungsi yang dibutuhkan oleh program yang akan kita jalankan dan deklarasi file agar memudahkan kodingannya dengan memanggil fungsi `file` saja. Selanjutnya

```c
void all_command(char *cmd, char *args[], char *error_messaging) {

    pid_t pid = fork();
    if (pid == 0) {

        execvp(cmd, args);

        perror("exec gagal");
        exit(1);

    } else if (pid > 0) {

        int status;
        waitpid(pid, &status, 0);

        if (WEXITSTATUS(status) != 0) {

            printf("[ERROR] Aiyaa! %s.\n", error_messaging);
            exit(1);
        } 
    } else {
        perror("Fork gagal brok!");
        exit(1);
    }
};
```

adalah fungsi yang dimana memanggil perintah ke dalam terminal tanpa menggunakan `system()`. Jika dibedah beberapa fungsinya

```c
void all_command(char *cmd, char *args[], char *error_messaging) {
```

fungsi yang membutuhkan command dan argumennya beserta error_messaging jika suatu kondisi tidak terpenuhi

```c
pid_t pid = fork();
```

fungsi yang membelah dirinya dari parent ke anaknya untuk mengeksekusi programnya.

```c
if (pid == 0) {

        execvp(cmd, args);

        perror("exec gagal");
        exit(1);

    }
```

ini adalah kondisi dimana ketika `pid == 0` maka program akan dieksekusi lewat `execvp` dengan execvp sendiri adalah program yg dijalankan tanpa harus sama dengan `parent program` atau program utamanya. `perror` akan muncul ketika sebuah cmd sendiri ada typonya dan muncul `perror` maupun ketika ada kekeliruan dalam argumennya juga, dia akan muncul error dan langsung keluar. jadi ketika kita mebuat sebuah command `mkdir` dalam execvp, dia akan membuang program parent atau lamanya dan diganti dengan sebuah command barunya.

```c
else if (pid > 0) {

        int status;
        waitpid(pid, &status, 0);

        if (WEXITSTATUS(status) != 0) {

            printf("[ERROR] Aiyaa! %s.\n", error_messaging);
            exit(1);
        } 
    }
};
```

Lalu ada kondisi lain yang dimana pid > 0 adalah program parentnya sendiri yang dihentikan sementara agar program `child process`nya berjalan. ketika status dari child processnya itu sendiri tidak sama dengan 0 maka program itu akan berhenti dan langsung gagal.

```c
else {
        perror("Fork gagal brok!");
        exit(1);
    }
```

ini adalah kondisi lain dimana kondisi fork gagal dieksekusi.

Lalu kita lanjut ke fungsi mainnya

```c
int main() {
    
        char *args1[] = {"mkdir", "brankas_kedai", NULL};
        all_command("mkdir", args1, "Proses bikin folder gagal.");

        char *args2[] = {"cp", file, "brankas_kedai/", NULL};
        all_command("cp", args2, "file atau folder tidak ditemukan");

        pid_t pid_grep = fork();
        if (pid_grep == 0) {

            execlp("bash", "bash", "-c", "grep 'Belum Lunas' brankas_kedai/buku_hutang.csv > brankas_kedai/daftar_penunggak.txt", NULL);

            perror("exec gagal brok");
            exit(1);
        } else if (pid_grep > 0) {

            int status_grep;
            waitpid(pid_grep, &status_grep, 0);

            if (WEXITSTATUS(status_grep) != 0) {
                printf("[ERROR] Aiyaa! Proses nagih hutang gagal.\n");
                exit(1);
            }
            
        } else {
            perror("Fork grep gagal!");
            exit(1);
        }

        char *args4[] = {        
            "zip", "-r", "rahasia_muthu.zip", "brankas_kedai", NULL
        };
        all_command("zip", args4, "Proses zipping gagal.");

        printf("[INFO!] Fuhh, selamat! Buku hutang dan daftar penagihan berhasil diamankan.");
        return 0;
}
```

dan jika dibedah masing-masing:

```c

        char *args1[] = {"mkdir", "brankas_kedai", NULL};
        all_command("mkdir", args1, "Proses bikin folder gagal.");

        char *args2[] = {"cp", file, "brankas_kedai/", NULL};
        all_command("cp", args2, "Proses pindahin filenya tidak bisa.");
```

dan

```c
char *args4[] = {        
            "zip", "-r", "rahasia_muthu.zip", "brankas_kedai", NULL
        };
        all_command("zip", args4, "Proses zipping gagal.");
```

adalah program yang dijalankan menggunakan function `all_command` dengan kriteria yang terpenuhi. program 1 adalah membuat direktori folder `brankas_kedai`. program 2 adalah mengcopy filenya ke dalam brankas, dan program terakhir adalah program untuk zip dari folder brankas tersebut.

program 3:

```c
pid_t pid_grep = fork();
        if (pid_grep == 0) {

            execlp("bash", "bash", "-c", "grep 'Belum Lunas' brankas_kedai/buku_hutang.csv > brankas_kedai/daftar_penunggak.txt", NULL);

            perror("exec gagal brok");
            exit(1);
        } else if (pid_grep > 0) {

            int status_grep;
            waitpid(pid_grep, &status_grep, 0);

            if (WEXITSTATUS(status_grep) != 0) {
                printf("[ERROR] Aiyaa! Proses nagih hutang gagal.\n");
                exit(1);
            }
            
        } else {
            perror("Fork grep gagal!");
            exit(1);
        }
```

adalah program yang hampir sama dengan function `all_command` tersebut. Yang membedakan dari kedua program ini adalah penggunaan `exec`nya. execvp ini adalah program yang ngepointing dari argumennya contohnya `char *args` sedangkan untu execlp adalah list program yang ditulis secara eksplisit satu per satu dalam fungsi. Maksudnya ada di contoh ini: `"bash", "bash"`. Persamaan dari kedua command ini adalah sama sama mencari path yang otomatis mencari file yang dijalankan.

dan terakhir adalah

```c
printf("[INFO!] Fuhh, selamat! Buku hutang dan daftar penagihan berhasil diamankan.");
        return 0;
```

ketika program tersebut sudah dijalankan dengan baik dan benar tanpa ada kendala apapun.
#### output

1. Hasil dari file yang sudah di download:

![alt_text](/assets/soal_1/download.png)

2. List awal sebelum program dijalankan

![alt_text](/assets/soal_1/list_awal.png)

3. Hasil akhir ketika program dijalankan tanpa ada kendala apapun

![alt_text](/assets/soal_1/sukses.png)

4. Error handling pertama kalau folder ada

![alt_text](/assets/soal_1/folder.png)

5. Error handling kedua ketika file tidak ada

![alt_text](/assets/soal_1/!file.png)

6. Error handling ketiga ketika folder tidak ada

![alt_text](/assets/soal_1/!folder.png)

#### Kendala

Tidak ada kendala

### Soal 2

**contract_daemon.c**

Langkah pertama adalah membuat file contract_daemon.c yang isinya adalah membuat program continue dengan penggunaan daemon. File: [contract_daemon.c](/soal_2/contract_daemon.c). Untuk awalannya sendiri yaitu:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h> 
#include <string.h>

const char *file = "contract.txt";
const char *file_log = "work.log";
char expected_content[256];
```

dengan memberikan konstan nama file contract.txt, log: work.log, dan yang terakhir adalah untuk kebutuhan dari suatu program nantinya. Setelah itu

```c
static void skeleton_daemon()
{
    pid_t pid;
    
    pid = fork();
    
    if (pid < 0)
        exit(EXIT_FAILURE);
    
    if (pid > 0)
        exit(EXIT_SUCCESS);
    
    if (setsid() < 0)
        exit(EXIT_FAILURE);
    
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    
    pid = fork();
    
    if (pid < 0)
        exit(EXIT_FAILURE);
    
    if (pid > 0)
        exit(EXIT_SUCCESS);
    
    umask(0);
    
    chdir("/home/Ly/Downloads/sisop/SISOP-2-2026-IT-107/soal_2");
    
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }
    
    openlog ("contract_daemon", LOG_PID, LOG_DAEMON);
}
```

adalah program template daemon yang saya temukan di github. Githubnya sendiri: https://github.com/pasce/daemon-skeleton-linux-c. Inti dari program ini adalah membuat sebuah daemon yang terus menerus berjalan sampai program tersebut berhenti dalam suatu keadaan

```c
void time_convert(char *buffer) {

    time_t t = time (NULL);

    struct tm *tm_info = localtime(&t);

    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
}
```

ini adalah program yang membuat sebuah timestamp. dia akan mengkonversikannya dalam bentuk hari, tahun, bulan, dan waktunya. Selanjutnya

```c
void creating() {
    char time_buf[26];
    time_convert(time_buf);
    FILE *filenya = fopen(file, "w");  //this is checking only 

    if (filenya != NULL) {
        snprintf(expected_content, sizeof(expected_content), "\"A promise to keep going, even when unseen.\"\nCreated at: %s", time_buf);
        fprintf(filenya, "%s", expected_content);
        fclose(filenya);
    }

}
```

adalah sebuah program untuk membuat file bernama `contract.txt` dengan isi suratnya tersebut adalah `"A promise to keep going, even when unseen."`. Program ini terbuat dan memberikan timestamp pada baris kedua setelah kalimat `created at: `. Selanjutnya

```c
void restoring() {
    char time_buf[26];
    time_convert(time_buf);
    FILE *filenya = fopen(file, "w");

    if (filenya != NULL) {
        snprintf(expected_content, sizeof(expected_content), "\"A promise to keep going, even when unseen.\"\nrestored at: %s", time_buf);
        fprintf(filenya, "%s", expected_content);
        fclose(filenya);
    }
}
```

adalah program yang berjalan untuk merestoring sebuah file contract.txt dengan isi filenya. File tersebut akan dicheck dan diisi ulang dengan kalimat yang sama pada baris pertama dan digantikan baris keduanya berupa `restored at:`. Selanjutnya

```c
void check() {
    FILE *filenya = fopen(file, "r");

    if (filenya == NULL) {
        restoring();

    } else {
        char current_content[256] = {0};

        fread(current_content, 1, sizeof(current_content) - 1, filenya);
        fclose(filenya);

        if (strcmp(current_content, expected_content) != 0) {
            FILE *flog = fopen(file_log, "a");
            if (flog != NULL) {
                fprintf(flog, "contract violated.\n");
                fclose(flog);
            }
            restoring(); 
        }
    }
}
```

adalah suatu program yang akan mengecheck dalam 2 kondisi. Kondisi pertama adalah ketika file `contract.txt` itu dihapus, maka dia akan membuat file baru dengan nama yang sama dan konten pada baris pertama juga sama walau dengan baris keduanya beda. Terbukti dari kondisi if pertama kalau filenya kosong, dia akan memanggil function `restoring`. Kondisi kedua adalah ketika sebuah file tadi isinya berubah. Misalkan pada baris pertama, ketika input user `rua rua` sebelum kalimat `"A promise to keep going, even when unseen."`, maka dia akan menulis ke dalam lognya `contract violated` dan dia akan memanggil function `restoring()`. Ini juga berlaku ketika baris kedua berubah, misal input user: `rua rua` sebelum kata `restored at` ataupun `created at`, program itu akan memanggil function `restoring()` dan menghilangkan kalimat yang ditambahkan user dan diganti menjadi `restored at`. Setelah itu, dia akan menuliskannya dalam log. Selanjutnya

```c
const char* randomnesss() {

    const char *listing[] = {"awake", "drifting", "numbness"};
    return listing[rand() % 3];
}
``` 

adalah program yang dibuat memanggil acak dari 3 kalimat yang sudah ada di array. Selanjutnya

```c
void sigterm_handler(int signum) {
    FILE *flog = fopen(file_log, "a");
    if (flog != NULL) {
        fprintf(flog, "We really weren't meant to be together\n");
        fclose(flog);
    }
    exit(0);
}
```

adalah program yang memanggil signal ketika sebuah pid dari daemon itu di hentikan atau di kill. Program ini akan terdeteksi dan terpanggil di main, lalu di mainnya sendiri:

```c
int main()
{
    srand(time(NULL));
    creating();
    skeleton_daemon();
    signal(SIGTERM, sigterm_handler);
    
    int hitung_waktu = 0;

    while (1)
    {

        check();
        
        if (hitung_waktu % 5 == 0) {
            FILE *flog = fopen(file_log, "a");
            if (flog != NULL) {
                fprintf(flog, "still working... [%s]\n", randomnesss());
                fclose(flog);
            }
        }
        
        sleep(1);
        hitung_waktu++; 
    }
   
    closelog();
    
    return EXIT_SUCCESS;
}
```

Dalam program main sendiri ada 4 function pertama yang dipanggil. function pertama adalah `srand(time(NULL));` yang memanggil function array dari listing tersebut dan mengambil 1 dari 3 array itu. kemudian function `creating()` yaitu function yang membuat file contract.txt pertama sebelum restore. Kemudian function `skeleton_daemon();` adalah function yang membuat daemon itu dan terus berulang dalam `while` looping. Dan terakhir function signal adalah function yang mendeteksi ketika daemon tersebut di kill menggunakan command `pkill` contohnya. Dia akan mengecek terlebih dahulu apakah program itu akan dibunuh atau tidak. Jika tidak, program itu terus berjalan. jika iya, function signal bekerja. 

#### output

1. awalan

![alt_text](/assets/soal_2/file.png)

2. Program dicompile menjadi `rua` dan dijalankan

![alt_text](/assets/soal_2/dijalankan.png)

3. Isi file

![alt_text](/assets/soal_2/isi_file.png)

4. Error handling pertama, menambahkan baris pertama:

![alt_text](/assets/soal_2/baris_pertama.png)

5. Error handling ketika baris kedua ditambahkan atau diganti:

![alt_text](/assets/soal_2/bukti.png)

![alt_text](/assets/soal_2/baris_kedua.png)

6. Error handling kedua ketika file contract.txt dihapus

![alt_text](/assets/soal_2/file_dihapus.png)

7. Program daemon dimatikan:

![alt_text](/assets/soal_2/dimatikan.png)

#### Kendala

Tidak ada kendala. Tapi dalam pengumpulan zipnya terdapat 1 function terselip yaitu function `deleteness()`. Function itu sudah dihapus dalam file contract_daemon.c disini.

### Soal 3

**angel.c**

Langkah pertama adalah menyiapkan beberapa yang dibutuhkan dalam codingan beserta konstantanya:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h>
#include <string.h>
#include <sys/prctl.h> 

const char *file_love = "LoveLetter.txt";
const char *file_log = "ethereal.log";
const char *file_pid = "maya.pid"; 
```

Lalu selanjutnya membuat function daemonnya:

```c
static void skeleton_daemon(char *cwd) {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    if (setsid() < 0) exit(EXIT_FAILURE);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) {

        FILE *fp = fopen(file_pid, "w");
        fprintf(fp, "%d\n", pid);
        fclose(fp);
        exit(EXIT_SUCCESS); 
    }

    umask(0);
    
    chdir(cwd);

    int x;
    for (x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
        close(x);
    }
    openlog("maya", LOG_PID, LOG_DAEMON);
}
```

Function ini bertujuan membuat custom daemon berdasarkan pid yang sudah ditentukan di awal yaitu maya. Untuk format dari daemon sendiri tidak berbeda jauh dengan Soal 2. Lalu

```c
void write_log(const char *proses, const char *status) {
    FILE *log_file = fopen(file_log, "a");

    if (log_file != NULL) {

        time_t t = time(NULL);
        struct tm *tm = localtime(&t);

        fprintf(log_file, "[%02d:%02d:%04d]-[%02d:%02d:%02d]_%s_%s\n",
                tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900,
                tm->tm_hour, tm->tm_min, tm->tm_sec,
                proses, status);

        fclose(log_file);
    }
}
```

function ini untuk menuliskan format waktu beserta status suksesnya entah itu berhasil ataupun gagal. inti dari function ini adalah menuliskan format waktu dan status yang sudah terbikin ke dalam log file `ethereal.log`. Setelah itu 

```c
void secret_feature() {
    write_log("secret", "RUNNING");
    
    const char *kalimat[] = {
        "aku akan fokus pada diriku sendiri",
        "aku mencintaimu dari sekarang hingga selamanya",
        "aku akan menjauh darimu, hingga takdir mempertemukan kita di versi kita yang terbaik.",
        "kalau aku dilahirkan kembali, aku tetap akan terus menyayangimu"
    };
    
    FILE *f = fopen(file_love, "w");

    if (f != NULL) {
        int acak = rand() % 4;
        fprintf(f, "%s\n", kalimat[acak]);
        fclose(f);
        write_log("secret", "SUCCESS");

    } else {

        write_log("secret", "ERROR");

    }
}
```

function ini akan mengambil 1 dari 4 kalimat yang sudah terbikin dalam array kalimat. Di awal dia akan menuliskan ke dalam log berupa `secret, running` setelah itu mengambil 1 kalimat secara acak dan melakukan check apakah dia sukses atau tidak dan dituliskan ke dalam lognya. Kemudian

```c
void surprise_feature() {
    write_log("surprise", "RUNNING");
    
    int status = system("base64 LoveLetter.txt > temp.txt && mv temp.txt LoveLetter.txt");
    
    if (status == 0) write_log("surprise", "SUCCESS");
    else write_log("surprise", "ERROR");
}
```

adalah function mendecode isi dari file LoveLetter.txtnya dan mengubahnya ke dalam bentuk base64. Kemudian, dia akan menuliskan ke dalam lognya berupa laporan apakah encodenya berhasil atau gagal. jika berhasil, akan ditulis dalam log `SUCCESS` begitupun sebaliknya. Kemudian dalam fungsi main:

```c
int main(int argc, char *argv[]) {

    if (argc != 2) {

        printf("Penggunaan:\n");

        printf("  ./angel -daemon  : jalankan sebagai daemon (nama proses: maya)\n");
        printf("  ./angel -decrypt : decrypt LoveLetter.txt\n");
        printf("  ./angel -kill    : kill proses\n");

        return EXIT_FAILURE;

    }

    if (strcmp(argv[1], "-daemon") == 0) {

        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL) return EXIT_FAILURE;

        prctl(PR_SET_NAME, "maya", 0, 0, 0);
        strcpy(argv[0], "maya");            

        srand(time(NULL));

        skeleton_daemon(cwd);

        while (1) {

            secret_feature();
            surprise_feature();
            sleep(10); 

        }
    } 
    
    else if (strcmp(argv[1], "-decrypt") == 0) {
        write_log("decrypt", "RUNNING");
        
        if (access(file_love, F_OK) != 0) {

            printf("ERROR: File LoveLetter.txt tidak ditemukan!\n");
            write_log("decrypt", "ERROR");
            return EXIT_FAILURE;

        }

        int status = system("base64 -d LoveLetter.txt > temp.txt && mv temp.txt LoveLetter.txt");
        
        if (status == 0) {
            printf("SUCCESS: File berhasil didekripsi!\n");
            write_log("decrypt", "SUCCESS");
        } else {
            printf("ERROR: Gagal mendekripsi file.\n");
            write_log("decrypt", "ERROR");
        }
    } 
    
    else if (strcmp(argv[1], "-kill") == 0) {
        write_log("kill", "RUNNING");
        
        FILE *fpid = fopen(file_pid, "r");
        if (fpid == NULL) {
            printf("ERROR: Program maya ternyata belum berjalan!\n");
            write_log("kill", "ERROR");
            return EXIT_FAILURE;
        }

        pid_t target_pid;
        fscanf(fpid, "%d", &target_pid);
        fclose(fpid);

        if (kill(target_pid, SIGTERM) == 0) {
            printf("SUCCESS: Proses maya (%d) berhasil dihentikan.\n", target_pid);
            remove(file_pid); 
            write_log("kill", "SUCCESS");
        } else {
            printf("ERROR: Gagal menghentikan proses.\n");
            write_log("kill", "ERROR");
        }
    }

    return EXIT_SUCCESS;
}
```

sendiri adalah function dari program utamanya. Terdapat 3 command yaitu `./angel -daemon` yang akan memanggil fungsi daemon dan melakukan proses kerjanya untuk menyalakan daemon dan menencode isi dari file LoveLetter.txtnya ke dalam base 64 dan menuliskan laporan akhirnya pada `ethereal.log` dalam keadaan sukses maupun gagal. Nah dalam keadaan menyala ini, program akan terus berputar dan akan mengganti isi filenya selama 10 detik dan menuliskan laporannya itu ke dalam log. lalu command kedua: `./angel -decrypt` adalah command yang mendecode isi dari file `LoveLetter.txt` ini menjadi kata aslinya. Dia akan mengecheck terlebih dahulu apakah file `LoveLetter.txt` ada atau tidak, jika tidak akan dituliskan ke dalam log dan keluar dari program. jika ada, dia akan mendecodenya isi file dan menuliskan ke dalam log dengan status sukses. Command terakhir yaitu `./angel -kill`. Command ini akan dijalankan dengan 2 kondisi. Kalau kondisi pertama program daemon tidak menyala, dia akan menuliskan ke dalam log dengan status error dan keluar. Jika daemon menyala, ia akan menyari program yang bernama `maya` ini dan meng-kill daemon tersebut dan statusnya akan dituliskan ke dalam file log dengan status sukses serta memberhentikan program daemonnya itu.

#### output

1. Awal mula

![alt_text](/assets/soal_3/begin.png)

2. Program `angel` dijalankan

![alt_text](/assets/soal_3/no_command.png)

3. ./angel -daemon jalan

![alt_text](/assets/soal_3/daemon.png)

4. ./angel -kill jalan

![alt_text](/assets/soal_3/kill.png)

5. ./angel -decrypt jalan

![alt_text](/assets/soal_3/decrypt.png)

6. Error handling 1: no file LoveLetter.txt

![alt_text](/assets/soal_3/decrypt_fail.png)

7. Error handling 2: ./angel -kill jalan ketika daemon mati

![alt_text](/assets/soal_3/kill_fail.png)

#### kendala

tidak ada kendala.

#### tambahan

Di function main, terdapat beberapa tambahan sedikit yaitu

```c
prctl(PR_SET_NAME, "maya", 0, 0, 0);

        int total_len = 0;
        for (int i = 0; i < argc; i++) {
            total_len += strlen(argv[i]) + 1; 
        }
        memset(argv[0], 0, total_len);
        
        strncpy(argv[0], "maya", total_len - 1);
```

program ini menjalankan daemon custom bernama maya dengan dibelakangnya yang awalnya "maya el daemon" menjadi "maya" saja. Itu saja tambahan dari solvingan no 3. 
