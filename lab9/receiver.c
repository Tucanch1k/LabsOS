#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_SIZE 1024
#define KEY 1234

typedef struct {
    char message[256];
    int sender_pid;
    time_t timestamp;
} shared_data_t;

void sem_wait(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

void sem_signal(int semid) {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

int main() {
    int shmid, semid;
    shared_data_t *shared_data;
    
    shmid = shmget(KEY, sizeof(shared_data_t), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        printf("Сначала запустите передающий процесс!\n");
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
    
    printf("=== ПРИНИМАЮЩИЙ ПРОЦЕСС ===\n");
    printf("PID процесса: %d\n", getpid());
    printf("Ключ SHM/SEM: %d\n", KEY);
    printf("Ожидание данных...\n");
    printf("Для остановки нажмите Ctrl+C\n\n");
    
    while (1) {
        sem_wait(semid);
        
        if (shared_data->timestamp != 0) {
            time_t receiver_time = time(NULL);
            struct tm *receiver_tm = localtime(&receiver_time);
            
            struct tm *sender_tm = localtime(&shared_data->timestamp);
            
            printf("┌─────────────────────────────────────────┐\n");
            printf("│ Время получения:  %s", asctime(receiver_tm));
            printf("│ PID получателя:   %d\n", getpid());
            printf("│ PID отправителя:  %d\n", shared_data->sender_pid);
            printf("│ Время отправки:   %s", asctime(sender_tm));
            printf("│ Сообщение:        %s\n", shared_data->message);
            printf("└─────────────────────────────────────────┘\n\n");
            
            shared_data->timestamp = 0;
        }
        
        sem_signal(semid);
        
        usleep(100000);
    }
    
    shmdt(shared_data);
    
    return 0;
}
