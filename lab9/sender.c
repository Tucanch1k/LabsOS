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
    printf("\nЗавершение передатчика. Очистка ресурсов...\n");
    shmdt(shared_data);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    exit(0);
}

int main() {
    shmid = shmget(KEY, sizeof(shared_data_t), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    shared_data = (shared_data_t*)shmat(shmid, NULL, 0);
    if (shared_data == (void*)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    semid = semget(KEY, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget failed");
        shmdt(shared_data);
        shmctl(shmid, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    semctl(semid, 0, SETVAL, 1);

    signal(SIGINT, cleanup);

    printf("=== ПЕРЕДАЮЩИЙ ПРОЦЕСС ===\n");
    printf("PID процесса: %d\n", getpid());
    printf("Передача данных каждые 3 секунды\n");
    printf("Для остановки нажмите Ctrl+C\n\n");

    int counter = 1;

    while (1) {
        sleep(3);

        sem_wait(semid);

        time_t now = time(NULL);

        snprintf(shared_data->message,
                 sizeof(shared_data->message),
                 "Сообщение #%d от передатчика",
                 counter++);

        shared_data->sender_pid = getpid();
        shared_data->timestamp = now;

        printf("[Отправлено] Время: %sPID отправителя: %d\nСообщение: %s\n\n",
               asctime(localtime(&now)),
               getpid(),
               shared_data->message);

        sem_signal(semid);
    }

    return 0;
}
