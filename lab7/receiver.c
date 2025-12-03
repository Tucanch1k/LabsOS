#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>


#define SHM_KEY 0x12345
#define SHM_SIZE 256


int main() {
    int shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shmid == -1) {
        printf("Shared memory not found. Run sender first.\n");
        return 1;
    }


    char *data = (char *)shmat(shmid, NULL, 0);
    if (data == (char*) -1) {
    perror("shmat");
    exit(1);
    }


    printf("Receiver started. PID=%d\n", getpid());


    while (1) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);


        printf("Receiver PID=%d | Time=%02d:%02d:%02d | Received: %s\n", getpid(), tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, data);

        usleep(500000);
    }


    return 0;
}
