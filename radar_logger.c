#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 5

// Radar data structure
typedef struct {
    int id;
    float distance;
    float velocity;
} RadarData;

// Shared memory buffer
RadarData raw_buffer[BUFFER_SIZE];
RadarData processed_buffer[BUFFER_SIZE];
int raw_in = 0, raw_out = 0, raw_count = 0;
int proc_in = 0, proc_out = 0, proc_count = 0;

// Sync primitives
pthread_mutex_t raw_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t proc_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t raw_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t raw_not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t proc_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t proc_not_full = PTHREAD_COND_INITIALIZER;

// Radar Generator
void* radar_thread(void* arg) {
    int radar_id = 1;
    while (1) {
        RadarData data = {
            .id = radar_id++,
            .distance = (rand() % 100) + 1,
            .velocity = (rand() % 60) - 30
        };

        pthread_mutex_lock(&raw_mutex);
        while (raw_count == BUFFER_SIZE)
            pthread_cond_wait(&raw_not_full, &raw_mutex);

        raw_buffer[raw_in] = data;
        raw_in = (raw_in + 1) % BUFFER_SIZE;
        raw_count++;

        pthread_cond_signal(&raw_not_empty);
        pthread_mutex_unlock(&raw_mutex);

        usleep(100000); // 100ms
    }
    return NULL;
}

// Data Transfer Processing Thread
void* transfer_thread(void* arg) {
    while (1) {
        RadarData data;

        // Read from raw buffer
        pthread_mutex_lock(&raw_mutex);
        while (raw_count == 0)
            pthread_cond_wait(&raw_not_empty, &raw_mutex);

        data = raw_buffer[raw_out];
        raw_out = (raw_out + 1) % BUFFER_SIZE;
        raw_count--;

        pthread_cond_signal(&raw_not_full);
        pthread_mutex_unlock(&raw_mutex);

        // Simulate processing 
        data.distance *= 0.98;
        data.velocity *= 1.05;

        // Write to processed buffer
        pthread_mutex_lock(&proc_mutex);
        while (proc_count == BUFFER_SIZE)
            pthread_cond_wait(&proc_not_full, &proc_mutex);

        processed_buffer[proc_in] = data;
        proc_in = (proc_in + 1) % BUFFER_SIZE;
        proc_count++;

        pthread_cond_signal(&proc_not_empty);
        pthread_mutex_unlock(&proc_mutex);

        usleep(50000); // 50ms processing delay
    }
    return NULL;
}

// Logger Thread
void* logger_thread(void* arg) {
    FILE* logfile = fopen("radar_log.txt", "w");
    if (!logfile) {
        perror("Log file error");
        return NULL;
    }

    while (1) {
        RadarData data;

        pthread_mutex_lock(&proc_mutex);
        while (proc_count == 0)
            pthread_cond_wait(&proc_not_empty, &proc_mutex);

        data = processed_buffer[proc_out];
        proc_out = (proc_out + 1) % BUFFER_SIZE;
        proc_count--;

        pthread_cond_signal(&proc_not_full);
        pthread_mutex_unlock(&proc_mutex);

        time_t now = time(NULL);
        char* ts = strtok(ctime(&now), "\n"); 

        fprintf(logfile, "[%s] ID=%d | Dist=%.2f m | Vel=%.2f m/s\n",
                ts, data.id, data.distance, data.velocity);
        fflush(logfile);

        usleep(100000); // 100ms log delay
    }

    fclose(logfile);
    return NULL;
}

int main() {
    srand(time(NULL));

    pthread_t t1, t2, t3;

    pthread_create(&t1, NULL, radar_thread, NULL);
    pthread_create(&t2, NULL, transfer_thread, NULL);
    pthread_create(&t3, NULL, logger_thread, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    return 0;
}
