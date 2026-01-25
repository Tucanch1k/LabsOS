#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define KEY 1234

typedef struct {
    char message[256];
    int sender_pid;
    time_t timestamp;
} shared_data_t;

int shmid, semid;
shared_data_t *shared_data;

void sem_wait(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

void sem_signal(int semid) {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

void cleanup(int sig) {
    printf("\nЗавершение приёмника...\n");
    shmdt(shared_data);
    exit(0);
}

int main() {
    shmid = shmget(KEY, sizeof(shared_data_t), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        printf("Сначала запустите sender\n");
        exit(EXIT_FAILURE);
    }

    shared_data = (shared_data_t*)shmat(shmid, NULL, 0);
    if (shared_data == (void*)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    semid = semget(KEY, 1, 0666);
    if (semid == -1) {
        perror("semget failed");
        shmdt(shared_data);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, cleanup);

    printf("=== ПРИНИМАЮЩИЙ ПРОЦЕСС ===\n");
    printf("PID процесса: %d\n", getpid());
    printf("Ожидание сообщений...\n");
    printf("Для остановки нажмите Ctrl+C\n\n");

    while (1) {
        sem_wait(semid);

        if (shared_data->timestamp != 0) {
            time_t now = time(NULL);

            printf("┌─────────────────────────────────────┐\n");
            printf("│ Время получения: %s", asctime(localtime(&now)));
            printf("│ PID получателя:  %d\n", getpid());
            printf("│ PID отправителя: %d\n", shared_data->sender_pid);
            printf("│ Время отправки:  %s", asctime(localtime(&shared_data->timestamp)));
            printf("│ Сообщение:       %s\n", shared_data->message);
            printf("└─────────────────────────────────────┘\n\n");

            shared_data->timestamp = 0;
        }

        sem_signal(semid);
        usleep(100000);
    }

    return 0;
}
