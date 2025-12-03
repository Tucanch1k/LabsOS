#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>


#define SHM_KEY 0x12345
#define SHM_SIZE 256


int shmid;
char *data;


void cleanup(int sig) {
    if (data != (char*) -1) shmdt(data);
    shmctl(shmid, IPC_RMID, NULL);
    printf("Sender: shared memory removed. Exiting...\n");
    exit(0);
}


int main() {
    shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shmid != -1) {
        printf("Sender already running or shared memory exists. Exiting.\n");
        return 1;
    }

    shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }


    data = (char *)shmat(shmid, NULL, 0);
    if (data == (char*) -1) {
        perror("shmat");
        exit(1);
    }


    signal(SIGINT, cleanup);


    printf("Sender started. PID=%d\n", getpid());


    while (1) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);


        char buf[SHM_SIZE];
        snprintf(buf, sizeof(buf), "Sender PID=%d | Time=%02d:%02d:%02d",getpid(), tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);


        strncpy(data, buf, SHM_SIZE);
        data[SHM_SIZE - 1] = '\0';

        usleep(300000);
    }


    return 0;
}
