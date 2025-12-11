#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

pthread_mutex_t shared_lock = PTHREAD_MUTEX_INITIALIZER;

void* writer_function(void* parameter) {
    
    int* number_array = (int*)parameter;

    for (int counter = 0; counter < 5; ++counter) {
        
        pthread_mutex_lock(&shared_lock);
        number_array[counter] = counter + 1;
        pthread_mutex_unlock(&shared_lock);
        usleep(5);
    }

    pthread_exit(NULL);
}

void* reader_function(void* parameter) {
    
    int* data_buffer = (int*)parameter;
    pthread_t current_thread_id = pthread_self();

    while (data_buffer[4] == 0) {
        pthread_mutex_lock(&shared_lock);
        fprintf(stdout, "[Thread#%lu] %d %d %d %d %d\n", 
                (unsigned long)current_thread_id, 
                data_buffer[0], data_buffer[1], 
                data_buffer[2], data_buffer[3], 
                data_buffer[4]);
        pthread_mutex_unlock(&shared_lock);
        usleep(1);
    }

    pthread_exit(NULL);
}

int main(int argument_count, char** argument_values) {
    pthread_t writer_thread_id;
    pthread_t reader_thread_ids[10];
    
    int* shared_data = calloc(sizeof(int), 5);

    pthread_create(&writer_thread_id, NULL, writer_function, shared_data);
    
    for (int index = 0; index < 10; ++index) {
        pthread_create(&reader_thread_ids[index], NULL, reader_function, shared_data);
    }

    pthread_join(writer_thread_id, NULL);
    
    for (int index = 0; index < 10; ++index) {
        pthread_join(reader_thread_ids[index], NULL);
    }
    
    free(shared_data);
    return 0;
}
