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