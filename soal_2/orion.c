#include "arena.h"

int main() {

    int shmid = shmget(SHM_KEY, sizeof(BattleArena), IPC_CREAT | 0666);
    BattleArena *arena = (BattleArena *)shmat(shmid, NULL, 0);

    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);

    memset(arena, 0, sizeof(BattleArena));
    sem_init(&arena->battle_mutex, 1, 1); 

    printf("Orion is ready (PID: %d)\n", getpid());

    struct msg_buffer message;
    int waiting_pid = 0;

    while (1) {

        msgrcv(msgid, &message, sizeof(message) - sizeof(long), 1, 0);

        if (strcmp(message.msg_text, "CANCEL") == 0) {
            printf("Prajurit %d batal antre (Timeout/Lawan Bot)\n", message.client_pid);
            if (waiting_pid == message.client_pid) {
                waiting_pid = 0; // Kosongkan antrean
                arena->is_matchmaking = 0;
            }
            continue; 
        }

        printf("Prajurit %d mencoba matchmaking...\n", message.client_pid);
        printf("Prajurit %d mencoba matchmaking...\n", message.client_pid);

        if (waiting_pid == 0) {

            waiting_pid = message.client_pid;
            arena->player1_pid = waiting_pid;
            arena->is_matchmaking = 1;
            
            message.msg_type = message.client_pid;
            strcpy(message.msg_text, "WAIT");
            msgsnd(msgid, &message, sizeof(message) - sizeof(long), 0);
        } else {
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