#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define MAX_TASKS 50
#define TIME_BUFFER_SIZE 20

// Structure to represent a task
typedef struct {
    int taskNo;
    int burstTime;
    time_t arrivalTime;
    time_t serviceTime;
    time_t completionTime;
} Task;

// Global variables
Task* readyQ;
int readyQ_size;
int readyQ_capacity;
int readyQ_front = 0;
int readyQ_rear = 0;
int readyQ_count = 0;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

FILE* logFile;
int totalTasks = 0;
double totalWaitTime = 0;
double totalTurnTime = 0;
int tasksRemaining = 1;  // Flag to indicate if more tasks are to be processed

// Function to get formatted time string
void getTimeString(time_t t, char* buffer) {
    struct tm* timeinfo = localtime(&t);
    strftime(buffer, TIME_BUFFER_SIZE, "%H:%M:%S", timeinfo);
}

// Function to add task to ReadyQ
void enqueue(Task task) {
    readyQ[readyQ_rear] = task;
    readyQ_rear = (readyQ_rear + 1) % readyQ_capacity;
    readyQ_count++;
}

// Function to remove task from ReadyQ
Task dequeue() {
    Task task = readyQ[readyQ_front];
    readyQ_front = (readyQ_front + 1) % readyQ_capacity;
    readyQ_count--;
    return task;
}

// Task producer thread function
void* task_function(void* arg) {
    FILE* taskFile = fopen((char*)arg, "r");
    if (taskFile == NULL) {
        perror("Error opening TaskFile");
        exit(1);
    }

    int taskNo, burstTime;
    char timeStr[TIME_BUFFER_SIZE];

    while (fscanf(taskFile, "%d %d", &taskNo, &burstTime) == 2) {
        pthread_mutex_lock(&queue_mutex);
        
        while (readyQ_count >= readyQ_capacity) {
            pthread_cond_wait(&not_full, &queue_mutex);
        }

        Task newTask = {
            .taskNo = taskNo,
            .burstTime = burstTime,
            .arrivalTime = time(NULL)
        };

        enqueue(newTask);
        getTimeString(newTask.arrivalTime, timeStr);
        
        fprintf(logFile, "TaskNo %d: %d\nArrival time: %s\n\n", 
                taskNo, burstTime, timeStr);
        
        pthread_cond_broadcast(&not_empty);
        pthread_mutex_unlock(&queue_mutex);
    }

    pthread_mutex_lock(&queue_mutex);
    tasksRemaining = 0;
    pthread_cond_broadcast(&not_empty);
    pthread_mutex_unlock(&queue_mutex);

    fclose(taskFile);
    return NULL;
}

// CPU consumer thread function
void* cpu_function(void* arg) {
    int cpu_id = *((int*)arg);
    char timeStr[TIME_BUFFER_SIZE];

    while (1) {
        pthread_mutex_lock(&queue_mutex);
        
        while (readyQ_count == 0 && tasksRemaining) {
            pthread_cond_wait(&not_empty, &queue_mutex);
        }

        if (readyQ_count == 0 && !tasksRemaining) {
            pthread_mutex_unlock(&queue_mutex);
            break;
        }

        Task task = dequeue();
        task.serviceTime = time(NULL);
        
        getTimeString(task.arrivalTime, timeStr);
        fprintf(logFile, "Statistics for CPU-%d:\nTaskNo %d\nArrival time: %s\n", 
                cpu_id, task.taskNo, timeStr);
        
        getTimeString(task.serviceTime, timeStr);
        fprintf(logFile, "Service time: %s\n\n", timeStr);

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&queue_mutex);

        // Simulate task execution
        sleep(task.burstTime);
        
        task.completionTime = time(NULL);
        
        // Calculate statistics
        double waitTime = difftime(task.serviceTime, task.arrivalTime);
        double turnTime = difftime(task.completionTime, task.arrivalTime);

        pthread_mutex_lock(&queue_mutex);
        totalTasks++;
        totalWaitTime += waitTime;
        totalTurnTime += turnTime;

        getTimeString(task.arrivalTime, timeStr);
        fprintf(logFile, "Statistics for CPU-%d:\nTaskNo %d\nArrival time: %s\n", 
                cpu_id, task.taskNo, timeStr);
        
        getTimeString(task.completionTime, timeStr);
        fprintf(logFile, "Completion time: %s\n\n", timeStr);
        
        pthread_mutex_unlock(&queue_mutex);
    }

    free(arg);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s TaskFile m n\n", argv[0]);
        return 1;
    }

    readyQ_capacity = atoi(argv[2]);  // m
    int num_cpus = atoi(argv[3]);     // n

    if (readyQ_capacity > 3 || readyQ_capacity >= num_cpus || num_cpus > 5) {
        printf("Invalid parameters: m ≤ 3 and m < n ≤ 5\n");
        return 1;
    }

    // Initialize ReadyQ
    readyQ = (Task*)malloc(readyQ_capacity * sizeof(Task));
    
    // Open log file
    logFile = fopen("LogFile", "w");
    if (logFile == NULL) {
        perror("Error opening LogFile");
        return 1;
    }

    // Create task thread
    pthread_t task_thread;
    pthread_create(&task_thread, NULL, task_function, argv[1]);

    // Create CPU threads
    pthread_t* cpu_threads = (pthread_t*)malloc(num_cpus * sizeof(pthread_t));
    for (int i = 0; i < num_cpus; i++) {
        int* cpu_id = (int*)malloc(sizeof(int));
        *cpu_id = i + 1;
        pthread_create(&cpu_threads[i], NULL, cpu_function, cpu_id);
    }

    // Wait for all threads to complete
    pthread_join(task_thread, NULL);
    for (int i = 0; i < num_cpus; i++) {
        pthread_join(cpu_threads[i], NULL);
    }

    // Write final statistics
    fprintf(logFile, "Number of tasks: %d\n", totalTasks);
    fprintf(logFile, "Average waiting time: %.2f seconds\n", totalWaitTime/totalTasks);
    fprintf(logFile, "Average turnaround time: %.2f seconds\n", totalTurnTime/totalTasks);

    // Cleanup
    free(readyQ);
    free(cpu_threads);
    fclose(logFile);
    pthread_mutex_destroy(&queue_mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);

    return 0;
}