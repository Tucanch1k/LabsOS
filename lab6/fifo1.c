#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define FIFO_FILE_PATH "ipc_fifo_channel"
#define MESSAGE_BUFFER_SIZE 1024
#define FILE_PERMISSIONS 0666

int main() {
    int fifo_file_descriptor;
    char transmission_buffer[MESSAGE_BUFFER_SIZE];
    
    if (mkfifo(FIFO_FILE_PATH, FILE_PERMISSIONS) == -1) {
        if (errno != EEXIST) {
            perror("Ошибка создания именованного канала");
            return EXIT_FAILURE;
        }
        printf("Именованный канал уже существует, используется существующий\n");
    }
    
    printf("Процесс-отправитель: открываю канал для передачи данных...\n");
    
    fifo_file_descriptor = open(FIFO_FILE_PATH, O_WRONLY);
    if (fifo_file_descriptor == -1) {
        perror("Ошибка открытия канала для записи");
        return EXIT_FAILURE;
    }
    
    time_t transmission_timestamp;
    time(&transmission_timestamp);
    
    snprintf(transmission_buffer, MESSAGE_BUFFER_SIZE, 
            "Данные от процесса-отправителя:\n"
            "Время передачи: %s"
            "Идентификатор процесса: %d\n", 
            ctime(&transmission_timestamp), 
            getpid());
    
    printf("Процесс-отправитель: отправляю данные...\n");
    
    ssize_t bytes_transferred = write(fifo_file_descriptor, 
                                     transmission_buffer, 
                                     strlen(transmission_buffer) + 1);
    
    if (bytes_transferred == -1) {
        perror("Ошибка записи в канал");
        close(fifo_file_descriptor);
        return EXIT_FAILURE;
    }
    
    printf("Процесс-отправитель: успешно передано %ld байт\n", bytes_transferred);
    
    close(fifo_file_descriptor);
    
    printf("Процесс-отправитель: данные отправлены через именованный канал\n");
    
    if (unlink(FIFO_FILE_PATH) == -1) {
        perror("Ошибка удаления именованного канала");
        return EXIT_FAILURE;
    }
    
    printf("Процесс-отправитель: именованный канал удален\n");
    printf("Процесс-отправитель: завершение работы\n");
    
    return EXIT_SUCCESS;
}
