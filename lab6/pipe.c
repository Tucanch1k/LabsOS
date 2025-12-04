#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define BUFFER_CAPACITY 256
#define READER_DELAY_SECONDS 5

int main(int argc, char* argv[]) {
    
    int communication_pipe[2];
    char message_buffer[BUFFER_CAPACITY];
   
    if (pipe(communication_pipe) == -1) {
        perror("Ошибка создания канала");
        return EXIT_FAILURE;
    }

    pid_t child_process_id = fork();

    if (child_process_id == -1) {
        perror("Ошибка создания дочернего процесса");
        return EXIT_FAILURE;
    }
    
    if (child_process_id == 0) {
        close(communication_pipe[1]);  

        printf("Дочерний процесс ожидает %d секунд...\n", READER_DELAY_SECONDS);
        sleep(READER_DELAY_SECONDS);

        ssize_t bytes_read = read(communication_pipe[0], message_buffer, sizeof(message_buffer));
        close(communication_pipe[0]);
        
        if (bytes_read > 0) {
            time_t child_receive_time;
            time(&child_receive_time);

            printf("\n===== ДОЧЕРНИЙ ПРОЦЕСС =====\n");
            printf("Время получения: %s", ctime(&child_receive_time));
            printf("Полученное сообщение: %s\n", message_buffer);
        } else {
            printf("Дочерний процесс: не удалось прочитать данные\n");
        }
    } else {
        close(communication_pipe[0]); 

        time_t parent_send_time;
        time(&parent_send_time);
        snprintf(message_buffer, 
                 sizeof(message_buffer), 
                 "Родительский процесс - время отправки: %sИдентификатор родителя: %d", 
                 ctime(&parent_send_time), 
                 getpid());

        ssize_t bytes_written = write(communication_pipe[1], message_buffer, sizeof(message_buffer));
        close(communication_pipe[1]);

        if (bytes_written > 0) {
            printf("Родительский процесс отправил %ld байт\n", bytes_written);
        } else {
            printf("Родительский процесс: ошибка при отправке\n");
        }

        wait(NULL);
        printf("\nРодительский процесс завершен\n");
    }

    return EXIT_SUCCESS;
}
