#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 256

typedef struct {
    char buffer[BUFFER_SIZE];
    sem_t semaphore;
    int counter;
    int running;
} shared_data_t;

void* writer_thread(void* arg) {
    shared_data_t* data = (shared_data_t*)arg;
    
    while (data->running) {
        sem_wait(&data->semaphore);
        
        snprintf(data->buffer, BUFFER_SIZE, 
                 "Запись #%d | Время: %ld", 
                 data->counter++, 
                 (long)time(NULL));
        
        printf("Писатель записал: %s\n", data->buffer);
        
        sem_post(&data->semaphore);
        
        sleep(1);
    }
    
    pthread_exit(NULL);
}

void* reader_thread(void* arg) {
    shared_data_t* data = (shared_data_t*)arg;
    pthread_t tid = pthread_self();
    
    while (data->running) {
        sem_wait(&data->semaphore);
        
        printf("Читатель [TID: %lu]: %s\n", 
               (unsigned long)tid, 
               data->buffer);
        
        sem_post(&data->semaphore);
        
        usleep(500000);
    }
    
    pthread_exit(NULL);
}

int main() {
    pthread_t writer, reader;
    shared_data_t shared_data;
    
    memset(shared_data.buffer, 0, BUFFER_SIZE);
    strcpy(shared_data.buffer, "Начальное состояние");
    shared_data.counter = 1;
    shared_data.running = 1;
    
    if (sem_init(&shared_data.semaphore, 0, 1) != 0) {
        perror("Ошибка инициализации семафора");
        return EXIT_FAILURE;
    }
    
    printf("=== Программа запущена ===\n");
    printf("Писатель записывает данные каждую секунду\n");
    printf("Читатель читает данные каждые 0.5 секунд\n");
    printf("Нажмите Ctrl+C для выхода\n\n");
    
    if (pthread_create(&writer, NULL, writer_thread, &shared_data) != 0) {
        perror("Ошибка создания потока писателя");
        sem_destroy(&shared_data.semaphore);
        return EXIT_FAILURE;
    }
    
    if (pthread_create(&reader, NULL, reader_thread, &shared_data) != 0) {
        perror("Ошибка создания потока читателя");
        sem_destroy(&shared_data.semaphore);
        return EXIT_FAILURE;
    }
    
    sleep(10);
    
    shared_data.running = 0;
    
    pthread_join(writer, NULL);
    pthread_join(reader, NULL);
    
    sem_destroy(&shared_data.semaphore);
    
    printf("\n=== Программа завершена ===\n");
    printf("Всего выполнено записей: %d\n", shared_data.counter - 1);
    
    return EXIT_SUCCESS;
}
