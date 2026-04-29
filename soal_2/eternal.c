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

int is_bot_active = 0;
int player_hp_local, bot_hp_local;
char bot_combat_logs[LOG_MAX][100];
int bot_log_idx = 0;

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