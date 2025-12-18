#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define READERS_COUNT 10
#define ARRAY_SIZE 100
#define MAX_ITERATIONS 20

typedef struct {
    char *shared_array;        
    pthread_rwlock_t *rwlock;   
    int *write_counter;         
    int *stop_flag;            
} thread_data_t;

void *reader_thread(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    
    while (!(*data->stop_flag)) {
        pthread_rwlock_rdlock(data->rwlock);
        
        printf("Reader TID: %lu, Counter: %d, Array: %s\n", 
               pthread_self(), *data->write_counter, data->shared_array);
        
        pthread_rwlock_unlock(data->rwlock);
        
        usleep(100000);
    }
    
    printf("Reader TID: %lu finished\n", pthread_self());
    return NULL;
}

void *writer_thread(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int iteration = 0;
    
    while (iteration < MAX_ITERATIONS) {
        pthread_rwlock_wrlock(data->rwlock);
        
        (*data->write_counter)++;
        
        snprintf(data->shared_array, ARRAY_SIZE, 
                "Writer iteration: %d, Write counter: %d", 
                iteration + 1, *data->write_counter);
        
        printf("Writer TID: %lu wrote: %s\n", 
               pthread_self(), data->shared_array);
        
        pthread_rwlock_unlock(data->rwlock);
        
        iteration++;
        
        usleep(300000);
    }
    
    *data->stop_flag = 1;
    printf("Writer TID: %lu finished\n", pthread_self());
    return NULL;
}

int main() {
    pthread_t readers[READERS_COUNT];
    pthread_t writer;
    pthread_rwlock_t rwlock;
    
    char *shared_array = (char *)malloc(ARRAY_SIZE * sizeof(char));
    if (!shared_array) {
        perror("Failed to allocate memory for shared array");
        return 1;
    }
    
    strcpy(shared_array, "Initial state");
    
    if (pthread_rwlock_init(&rwlock, NULL) != 0) {
        perror("Failed to initialize rwlock");
        free(shared_array);
        return 1;
    }
    
    int write_counter = 0;
    int stop_flag = 0;    

    thread_data_t data = {
        .shared_array = shared_array,
        .rwlock = &rwlock,
        .write_counter = &write_counter,
        .stop_flag = &stop_flag
    };
    
    printf("=== Starting program with %d readers and 1 writer ===\n", READERS_COUNT);
    
    for (int i = 0; i < READERS_COUNT; i++) {
        if (pthread_create(&readers[i], NULL, reader_thread, &data) != 0) {
            perror("Failed to create reader thread");
            for (int j = 0; j < i; j++) {
                pthread_join(readers[j], NULL);
            }
            pthread_rwlock_destroy(&rwlock);
            free(shared_array);
            return 1;
        }
        printf("Created reader thread %d with TID: %lu\n", i, readers[i]);
    }
    
    if (pthread_create(&writer, NULL, writer_thread, &data) != 0) {
        perror("Failed to create writer thread");
        stop_flag = 1;
        for (int i = 0; i < READERS_COUNT; i++) {
            pthread_join(readers[i], NULL);
        }
        pthread_rwlock_destroy(&rwlock);
        free(shared_array);
        return 1;
    }
    printf("Created writer thread with TID: %lu\n", writer);
    
    pthread_join(writer, NULL);
    printf("Writer thread joined\n");
    
    usleep(200000);
    
    for (int i = 0; i < READERS_COUNT; i++) {
        pthread_join(readers[i], NULL);
        printf("Reader thread %d joined\n", i);
    }
    
    pthread_rwlock_destroy(&rwlock);
    
    free(shared_array);
    
    printf("=== Program finished successfully ===\n");
    return 0;
}
