#include "queue.c"

#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>

int simulationTime = 120;    // simulation time
int seed = 10;               // seed for randomness
int emergencyFrequency = 5; // frequency of emergency
float p = 0.2;               // probability of a ground job (launch & assembly)

// int or other variables to use
time_t deadline4job = 0;
time_t startTime4Simulation = 0;
int simulationStartTime = 0;

#define LANDING 1
#define LAUNCH 2
#define ASSEMBLY 3
#define EMERGENCY 4

#define LANDING_DURATION 2
#define LAUNCH_DURATION 4
#define ASSEMBLY_DURATION 12
#define EMERGENCY_DURATION 2


void* LandingJob(void *arg);
void* LaunchJob(void *arg);
void* EmergencyJob(void *arg);
void* AssemblyJob(void *arg);
void* ControlTower(void *arg);

// new voids
void* PadA(void *arg);
void* PadB(void *arg);
void* WriteLog(char* log);
void* QueuePrinter(void *arg);
void* PrintQueue(Queue* queue);


// new queues

Queue *landingQueue;
Queue *launchQueue;
Queue *emergencyQueue;
Queue *assemblyQueue;
Queue *padAQueue;
Queue *padBQueue;


// mutexs for queues
pthread_mutex_t landingQueueMutex;
pthread_mutex_t launchQueueMutex;
pthread_mutex_t emergencyQueueMutex;
pthread_mutex_t assemblyQueueMutex;
pthread_mutex_t padAQueueMutex;
pthread_mutex_t padBQueueMutex;
pthread_mutex_t logFileMutex;


// pthread sleeper function
int pthread_sleep (int seconds)
{
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    struct timespec timetoexpire;
    if(pthread_mutex_init(&mutex,NULL))
    {
        return -1;
    }
    if(pthread_cond_init(&conditionvar,NULL))
    {
        return -1;
    }
    struct timeval tp;
    //When to expire is an absolute time, so get the current time and add it to our delay time
    gettimeofday(&tp, NULL);
    timetoexpire.tv_sec = tp.tv_sec + seconds;
    timetoexpire.tv_nsec = tp.tv_usec * 1000;

    pthread_mutex_lock (&mutex);
    int res =  pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
    pthread_mutex_unlock (&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&conditionvar);

    //Upon successful completion, a value of zero shall be returned
    return res;
}

int main(int argc,char **argv) {
    // -p (float) => sets p
    // -t (int) => simulation time in seconds
    // -s (int) => change the random seed
    for(int i=1; i<argc; i++) {
        if(!strcmp(argv[i], "-p")) {
            p = atof(argv[++i]);
        }
        else if(!strcmp(argv[i], "-t")) {
            simulationTime = atoi(argv[++i]);
        }
        else if(!strcmp(argv[i], "-s"))  {
            seed = atoi(argv[++i]);
        }
    }

    srand(seed); // feed the seed

    /* Queue usage example
        Queue *myQ = ConstructQueue(1000);
        Job j;
        j.ID = myID;
        j.type = 2;
        Enqueue(myQ, j);
        Job ret = Dequeue(myQ);
        DestructQueue(myQ);
    */

    // your code goes here

    startTime4Simulation = time(NULL);

    FILE *logFile = fopen("log2.txt", "w");
    fprintf(logFile, "EventID, Status, Request Time, End Time, Turnaround Time, Pad\n");
    fclose(logFile);

    deadline4job = time(NULL) + simulationTime;

    landingQueue = ConstructQueue(1000);
    launchQueue = ConstructQueue(1000);
    assemblyQueue = ConstructQueue(1000);
    emergencyQueue = ConstructQueue(1000);
    padAQueue = ConstructQueue(1000);
    padBQueue = ConstructQueue(1000);

    pthread_t landingThread;
    pthread_t launchThread;
    pthread_t emergencyThread;
    pthread_t assemblyThread;
    pthread_t controlTowerThread;
    pthread_t padAThread;
    pthread_t padBThread;
    pthread_t QueuePrinterThread;


    pthread_mutex_init(&landingQueueMutex, NULL);
    pthread_mutex_init(&launchQueueMutex, NULL);
    pthread_mutex_init(&emergencyQueueMutex, NULL);
    pthread_mutex_init(&assemblyQueueMutex, NULL);
    pthread_mutex_init(&padAQueueMutex, NULL);
    pthread_mutex_init(&padBQueueMutex, NULL);
    pthread_mutex_init(&logFileMutex, NULL);

    pthread_create(&landingThread, NULL, LandingJob, NULL);
    pthread_create(&launchThread, NULL, LaunchJob, NULL);
    pthread_create(&emergencyThread, NULL, EmergencyJob, NULL);
    pthread_create(&assemblyThread, NULL, AssemblyJob, NULL);
    pthread_create(&controlTowerThread, NULL, ControlTower, NULL);
    pthread_create(&padAThread, NULL, PadA, NULL);
    pthread_create(&padBThread, NULL, PadB, NULL);
    pthread_create(&QueuePrinterThread, NULL, QueuePrinter, NULL);


    pthread_join(landingThread, NULL);
    pthread_join(launchThread, NULL);
    pthread_join(emergencyThread, NULL);
    pthread_join(assemblyThread, NULL);
    pthread_join(controlTowerThread, NULL);
    pthread_join(padAThread, NULL);
    pthread_join(padBThread, NULL);
    pthread_join(QueuePrinterThread, NULL);

    DestructQueue(landingQueue);
    DestructQueue(launchQueue);
    DestructQueue(emergencyQueue);
    DestructQueue(assemblyQueue);
    DestructQueue(padAQueue);
    DestructQueue(padBQueue);

    return 0;
}

// the function that creates plane threads for landing
void* LandingJob(void *arg) {

    while(time(NULL) < deadline4job) {
        pthread_sleep(2);
        if(rand()%100 < 100-p*100) {
            Job j;
            j.ID = rand()%1000;
            j.type = LANDING;
            j.durationTime = LANDING_DURATION;
            j.arrivalTime = time(NULL) - startTime4Simulation;

            pthread_mutex_lock(&landingQueueMutex);
            Enqueue(landingQueue, j);
            pthread_mutex_unlock(&landingQueueMutex);
        }
    }
    return NULL;

}

// the function that creates plane threads for departure
void* LaunchJob(void *arg) {

    while(time(NULL) < deadline4job) {
        pthread_sleep(2);
        if(rand()%100 < (p/2)*100) {
            Job j;
            j.ID = rand()%1000;
            j.type = LAUNCH;
            j.durationTime = LAUNCH_DURATION;
            j.arrivalTime = time(NULL) - startTime4Simulation;

            pthread_mutex_lock(&launchQueueMutex);
            Enqueue(launchQueue, j);
            pthread_mutex_unlock(&launchQueueMutex);
        }
    }
    return NULL;

}

// the function that creates plane threads for emergency landing
void* EmergencyJob(void *arg) {
    while (time(NULL) < deadline4job) {
        pthread_sleep(emergencyFrequency * 2);
        for (int i = 0; i < 2; i++)
        {
            Job j;
            j.ID = rand()%1000;
            j.type = EMERGENCY;
            j.durationTime = EMERGENCY_DURATION;
            j.arrivalTime = time(NULL) - simulationStartTime;

            pthread_mutex_lock(&emergencyQueueMutex); 
            Enqueue(emergencyQueue, j);
            pthread_mutex_unlock(&emergencyQueueMutex);
        }
    }
    return NULL;
}

// the function that creates plane threads for emergency landing
void* AssemblyJob(void *arg) {

    while(time(NULL) < deadline4job) {
        pthread_sleep(2);
        if(rand()%100 < (p/2)*100) {
            Job j;
            j.ID = rand()%1000;
            j.type = ASSEMBLY;
            j.durationTime = ASSEMBLY_DURATION;
            j.arrivalTime = time(NULL) - startTime4Simulation;

            pthread_mutex_lock(&assemblyQueueMutex);
            Enqueue(assemblyQueue, j);
            pthread_mutex_unlock(&assemblyQueueMutex);
        }
    }
    return NULL;

}

// the function that controls the air traffic
void* ControlTower(void *arg) {

    while(time(NULL) < deadline4job) {

        pthread_mutex_lock(&launchQueueMutex);
        pthread_mutex_lock(&assemblyQueueMutex);


        if (assemblyQueue->size < 3 && launchQueue->size < 3)
        {
            pthread_mutex_unlock(&launchQueueMutex);
            pthread_mutex_unlock(&assemblyQueueMutex);
            pthread_mutex_lock(&landingQueueMutex);

            while(!isEmpty(landingQueue)) {
            pthread_mutex_lock(&padAQueueMutex);
            pthread_mutex_lock(&padBQueueMutex);
            if(padAQueue->durationTime <= padBQueue->durationTime) {
                Enqueue(padAQueue, Dequeue(landingQueue));
            }
            else {
                Enqueue(padBQueue, Dequeue(landingQueue));
            }

            pthread_mutex_unlock(&padAQueueMutex);
            pthread_mutex_unlock(&padBQueueMutex);
            }

            pthread_mutex_unlock(&landingQueueMutex);
            pthread_mutex_lock(&padAQueueMutex);
            pthread_mutex_lock(&launchQueueMutex);

        }else{
            pthread_mutex_lock(&padAQueueMutex);
            pthread_mutex_lock(&padBQueueMutex);
            pthread_mutex_unlock(&launchQueueMutex);
            pthread_mutex_unlock(&assemblyQueueMutex);
            pthread_mutex_lock(&landingQueueMutex);

            pthread_mutex_unlock(&padAQueueMutex);
            pthread_mutex_unlock(&padBQueueMutex);
            pthread_mutex_unlock(&landingQueueMutex);

        }
    }

    return NULL;

}

void* PadA(void *arg) {
    while(time(NULL) < deadline4job) {
        pthread_mutex_lock(&padAQueueMutex);
        if(isEmpty(padAQueue)) {
            pthread_mutex_unlock(&padAQueueMutex);
            pthread_sleep(2);
        }
        else {
            int sleepTime = padAQueue->head->data.durationTime;

            pthread_mutex_unlock(&padAQueueMutex);
            pthread_sleep(padAQueue->head->data.durationTime);

            pthread_mutex_lock(&padAQueueMutex);
            Job j = Dequeue(padAQueue);
            pthread_mutex_unlock(&padAQueueMutex);

            time_t end_time = time(NULL) - startTime4Simulation;
            char log[100];
            sprintf(log, "%-5d %5d %10d %10ld %10ld %10c\n", j.ID, j.type, j.arrivalTime, end_time, end_time-j.arrivalTime, 'A');
            WriteLog(log);
        }
    }
    return NULL;
}

void* PadB(void *arg) {
    while(time(NULL) < deadline4job) {
        pthread_mutex_lock(&padBQueueMutex);
        if(isEmpty(padBQueue)) {
            pthread_mutex_unlock(&padBQueueMutex);
            pthread_sleep(2);
        }
        else {
            int sleepTime = padBQueue->head->data.durationTime;
            pthread_mutex_unlock(&padBQueueMutex);
            pthread_sleep(padBQueue->head->data.durationTime);

            pthread_mutex_lock(&padBQueueMutex);
            Job j = Dequeue(padBQueue);
            pthread_mutex_unlock(&padBQueueMutex);

            time_t end_time = time(NULL) - startTime4Simulation;
            char log[100];
            sprintf(log, "%-5d %5d %10d %10ld %10ld %10c\n", j.ID, j.type, j.arrivalTime, end_time, end_time-j.arrivalTime, 'B');
            WriteLog(log);
        }
    }
    return NULL;
}


void* PrintQueue(Queue* q) {
    if(isEmpty(q)) {
        printf("empty\n");
    }
    else {
        NODE* curr = q->head;
        while(curr != NULL) {
            printf("%d ", curr->data.ID);
            curr = curr->prev;
        }
        printf("\n");
    }
}


void* WriteLog(char* log){
    pthread_mutex_lock(&logFileMutex);

    FILE* fp = fopen("log2.txt", "a");
    fprintf(fp, "%s", log);
    fclose(fp);

    pthread_mutex_unlock(&logFileMutex);
    
    return NULL;
}


void* QueuePrinter(void *arg){
    while(time(NULL) < deadline4job){

        pthread_sleep(2);
        int current_time = time(NULL) - startTime4Simulation;
        if(30 <= current_time){

            pthread_mutex_lock(&landingQueueMutex);
            printf("At %d sec landing: ", current_time);
            PrintQueue(landingQueue);
            pthread_mutex_unlock(&landingQueueMutex);


            pthread_mutex_lock(&launchQueueMutex);
            printf("At %d sec launch: ", current_time);
            PrintQueue(launchQueue);
            pthread_mutex_unlock(&launchQueueMutex);


            pthread_mutex_lock(&assemblyQueueMutex);
            printf("At %d sec assembly: ", current_time);
            PrintQueue(assemblyQueue);
            pthread_mutex_unlock(&assemblyQueueMutex);

            pthread_mutex_lock(&padAQueueMutex);
            printf("At %d sec padA: ", current_time);
            PrintQueue(padAQueue);
            pthread_mutex_unlock(&padAQueueMutex);


            pthread_mutex_lock(&padBQueueMutex);
            printf("At %d sec padB: ", current_time);
            PrintQueue(padBQueue);
            pthread_mutex_unlock(&padBQueueMutex);
        }
    }
    return NULL;
}

