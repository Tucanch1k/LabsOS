#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#define FIFO_FILE_PATH "ipc_fifo_channel"
#define MESSAGE_BUFFER_SIZE 1024
#define RECEIVER_DELAY_SECONDS 10

int main() {
    int fifo_file_descriptor;
    char reception_buffer[MESSAGE_BUFFER_SIZE];
    
    time_t process_start_time;
    time(&process_start_time);
    
    printf("Процесс-получатель: запущен\n");
    printf("Процесс-получатель: ожидание %d секунд перед чтением...\n", RECEIVER_DELAY_SECONDS);
    
    sleep(RECEIVER_DELAY_SECONDS);
    
    printf("Процесс-получатель: открываю именованный канал для чтения...\n");
    
    fifo_file_descriptor = open(FIFO_FILE_PATH, O_RDONLY);
    if (fifo_file_descriptor == -1) {
        perror("Ошибка открытия канала для чтения");
        fprintf(stderr, "Убедитесь, что процесс-отправитель уже запущен\n");
        return EXIT_FAILURE;
    }
    
    printf("Процесс-получатель: канал открыт, читаю данные...\n");
    
    ssize_t bytes_received = read(fifo_file_descriptor, 
                                  reception_buffer, 
                                  MESSAGE_BUFFER_SIZE - 1);
    
    close(fifo_file_descriptor);
    
    if (bytes_received == -1) {
        perror("Ошибка чтения из канала");
        return EXIT_FAILURE;
    }
    
    reception_buffer[bytes_received] = '\0';
    
    time_t data_receive_time;
    time(&data_receive_time);
    
    printf("\n========================================\n");
    printf("ПРОЦЕСС-ПОЛУЧАТЕЛЬ\n");
    printf("========================================\n");
    printf("Время запуска процесса:  %s", ctime(&process_start_time));
    printf("Время получения данных:  %s", ctime(&data_receive_time));
    printf("Идентификатор процесса:  %d\n", getpid());
    printf("Размер полученных данных: %ld байт\n", bytes_received);
    printf("----------------------------------------\n");
    printf("СОДЕРЖАНИЕ СООБЩЕНИЯ:\n");
    printf("----------------------------------------\n");
    printf("%s\n", reception_buffer);
    printf("========================================\n");
    
    printf("Процесс-получатель: успешно завершил работу\n");
    
    return EXIT_SUCCESS;
}
