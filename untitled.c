




Queue *padAEmergencyQueue;
Queue *padBEmergencyQueue;


// create a mutex for each queue

pthread_mutex_t padAEmergencyQueueMutex;
pthread_mutex_t padBEmergencyQueueMutex;


// create log file mutex
pthread_mutex_t logFileMutex;

// pthread sleeper function



    padAEmergencyQueue = ConstructQueue(1000);
    padBEmergencyQueue = ConstructQueue(1000);



    // initialize mutexes

    pthread_mutex_init(&padAEmergencyQueueMutex, NULL);
    pthread_mutex_init(&padBEmergencyQueueMutex, NULL);




// the function that controls the air traffic
void *ControlTower(void *arg)
{
    while (time(NULL) < deadline)
    {
        // check if there is a emergency job
        // lock the emergency queue
        pthread_mutex_lock(&emergencyQueueMutex);
        // if there is a emergency job
        if (emergencyQueue->size > 0)
        {
            // lock the padA emergency queue
            pthread_mutex_lock(&padAEmergencyQueueMutex);

            Enqueue(padAEmergencyQueue, Dequeue(emergencyQueue));
            
            // unlock the emergency queue and padA emergency queue
            pthread_mutex_unlock(&padAEmergencyQueueMutex);
        }
        if (emergencyQueue->size > 0)
        {
            // lock the padB emergency queue
            pthread_mutex_lock(&padBEmergencyQueueMutex);

            Enqueue(padBEmergencyQueue, Dequeue(emergencyQueue));
            
            // unlock the emergency queue and padB emergency queue
            pthread_mutex_unlock(&padBEmergencyQueueMutex);
        }
        // unlock the emergency queue
        pthread_mutex_unlock(&emergencyQueueMutex);


        // lock the landing, launch, and assembly queues
        pthread_mutex_lock(&launchQueueMutex);
        pthread_mutex_lock(&assemblyQueueMutex);

        // if the launch or assembly queue has less than 3 jobs, then empty the landing queue
        if (launchQueue->size < 3 && assemblyQueue->size < 3)
        {
            // unlock the launch and assembly queues
            pthread_mutex_unlock(&launchQueueMutex);
            pthread_mutex_unlock(&assemblyQueueMutex);

            // lock the landing queue
            pthread_mutex_lock(&landingQueueMutex);
            // empty the landing queue
            while (!isEmpty(landingQueue))
            {
                // lock padAQueue and padBQueue
                pthread_mutex_lock(&padAQueueMutex);
                pthread_mutex_lock(&padBQueueMutex);
                if (padAQueue->duration <= padBQueue->duration)
                {
                    Enqueue(padAQueue, Dequeue(landingQueue));
                }
                else
                {
                    Enqueue(padBQueue, Dequeue(landingQueue));
                }
                // unlock padAQueue and padBQueue
                pthread_mutex_unlock(&padAQueueMutex);
                pthread_mutex_unlock(&padBQueueMutex);
            }
            // unlock the landing queue
            pthread_mutex_unlock(&landingQueueMutex);

            // lock the padAQueue and launchQueue
            pthread_mutex_lock(&padAQueueMutex);
            pthread_mutex_lock(&launchQueueMutex);

            if (!isEmpty(launchQueue))
            {
                Enqueue(padAQueue, Dequeue(launchQueue));
            }
            // unlock the padAQueue and launchQueue
            pthread_mutex_unlock(&padAQueueMutex);
            pthread_mutex_unlock(&launchQueueMutex);

            // lock the padBQueue and assemblyQueue
            pthread_mutex_lock(&padBQueueMutex);
            pthread_mutex_lock(&assemblyQueueMutex);

            if (!isEmpty(assemblyQueue))
            {
                Enqueue(padBQueue, Dequeue(assemblyQueue));
            }
            // unlock the padBQueue and assemblyQueue
            pthread_mutex_unlock(&padBQueueMutex);
            pthread_mutex_unlock(&assemblyQueueMutex);
        }
        else // Take one job from each queue
        {
            // lock padAQueue and padBQueue
            pthread_mutex_lock(&padAQueueMutex);
            pthread_mutex_lock(&padBQueueMutex);

            // if launchQueue is not empty, take the first job from launchQueue and put it in padAQueue
            if (!isEmpty(launchQueue))
            {
                Enqueue(padAQueue, Dequeue(launchQueue));
            }
            // unlock launchQueue
            pthread_mutex_unlock(&launchQueueMutex);

            // if assemblyQueue is not empty, take the first job from assemblyQueue and put it in padBQueue
            if (!isEmpty(assemblyQueue))
            {
                Enqueue(padBQueue, Dequeue(assemblyQueue));
            }
            // unlock assemblyQueue
            pthread_mutex_unlock(&assemblyQueueMutex);

            // if landingQueue is not empty, take the first job from landingQueue and put it in shortest pad
            // lock landingQueue
            pthread_mutex_lock(&landingQueueMutex);
            if (!isEmpty(landingQueue))
            {
                if (padAQueue->duration <= padBQueue->duration)
                {
                    Enqueue(padAQueue, Dequeue(landingQueue));
                }
                else
                {
                    Enqueue(padBQueue, Dequeue(landingQueue));
                }
            }

            // unlock padAQueue, padBQueue, and landingQueue
            pthread_mutex_unlock(&padAQueueMutex);
            pthread_mutex_unlock(&padBQueueMutex);
            pthread_mutex_unlock(&landingQueueMutex);
        }
    }

    return NULL;
}

void *PadA(void *arg)
{
    while (time(NULL) < deadline)
    {
        // lock padAEmergencyQueue
        pthread_mutex_lock(&padAEmergencyQueueMutex);
        if(padAEmergencyQueue->size > 0) {

            int sleepTime = padAEmergencyQueue->head->data.duration;
            // unlock padAEmergencyQueue
            pthread_mutex_unlock(&padAEmergencyQueueMutex);

            pthread_sleep(sleepTime);

            pthread_mutex_lock(&padAEmergencyQueueMutex);
            Job j = Dequeue(padAEmergencyQueue);
            pthread_mutex_unlock(&padAEmergencyQueueMutex);

            // create the log string
            time_t end_time = time(NULL) - simulationStartTime;
            char log[100];
            sprintf(log, "%-5d %5d %10d %10d %10d %10c\n", j.ID, j.type, j.arrivalTime, end_time, end_time - j.arrivalTime, 'A');
            WriteLog(log);
            continue;
        }
        else {
            // unlock padAEmergencyQueue
            pthread_mutex_unlock(&padAEmergencyQueueMutex);
        }
        // lock padAQueue
        pthread_mutex_lock(&padAQueueMutex);
        if (isEmpty(padAQueue))
        {
            pthread_mutex_unlock(&padAQueueMutex);
            // sleep for UNIT_TIME seconds
            pthread_sleep(UNIT_TIME);
        }
        else
        {
            int sleepTime = padAQueue->head->data.duration;
            // unlock padAQueue
            pthread_mutex_unlock(&padAQueueMutex);

            pthread_sleep(sleepTime);

            pthread_mutex_lock(&padAQueueMutex);
            Job j = Dequeue(padAQueue);
            pthread_mutex_unlock(&padAQueueMutex);

            // create the log string
            time_t end_time = time(NULL) - simulationStartTime;
            char log[100];
            sprintf(log, "%-5d %5d %10d %10d %10d %10c\n", j.ID, j.type, j.arrivalTime, end_time, end_time - j.arrivalTime, 'A');
            WriteLog(log);
        }
    }
    return NULL;
}

void *PadB(void *arg)
{
    while (time(NULL) < deadline)
    {
        // lock padBEmergencyQueue
        pthread_mutex_lock(&padBEmergencyQueueMutex);
        if(padBEmergencyQueue->size > 0) {
            int sleepTime = padBEmergencyQueue->head->data.duration;
            // unlock padBQueue
            pthread_mutex_unlock(&padBEmergencyQueueMutex);

            pthread_sleep(sleepTime);

            pthread_mutex_lock(&padBEmergencyQueueMutex);
            Job j = Dequeue(padBEmergencyQueue);
            pthread_mutex_unlock(&padBEmergencyQueueMutex);

            // create the log string
            time_t end_time = time(NULL) - simulationStartTime;
            char log[100];
            sprintf(log, "%-5d %5d %10d %10d %10d %10c\n", j.ID, j.type, j.arrivalTime, end_time, end_time - j.arrivalTime, 'A');
            WriteLog(log);
            continue;
        }
        else {
            // unlock padBEmergencyQueue
            pthread_mutex_unlock(&padBEmergencyQueueMutex);
        }
        // lock padBQueue
        pthread_mutex_lock(&padBQueueMutex);
        if (isEmpty(padBQueue))
        {
            pthread_mutex_unlock(&padBQueueMutex);
            // sleep for UNIT_TIME seconds
            pthread_sleep(UNIT_TIME);
        }
        else
        {
            int sleepTime = padBQueue->head->data.duration;
            // unlock padBQueue
            pthread_mutex_unlock(&padBQueueMutex);
            pthread_sleep(sleepTime);

            pthread_mutex_lock(&padBQueueMutex);
            Job j = Dequeue(padBQueue);
            pthread_mutex_unlock(&padBQueueMutex);

            // create the log string
            time_t end_time = time(NULL) - simulationStartTime;
            char log[100];
            sprintf(log, "%-5d %5d %10d %10d %10d %10c\n", j.ID, j.type, j.arrivalTime, end_time, end_time - j.arrivalTime, 'B');
            WriteLog(log);
        }
    }
    return NULL;
}

// open log.txt, protect it with mutex, and write the log to it
void *WriteLog(char *log)
{
    // lock the log file
    pthread_mutex_lock(&logFileMutex);
    // open the log file
    FILE *fp = fopen("log.txt", "a");
    // write the log to the log file
    fprintf(fp, "%s", log);
    // close the log file
    fclose(fp);
    // unlock the log file
    pthread_mutex_unlock(&logFileMutex);

    return NULL;
}

void *PrintCurrentQueues(void *arg)
{
    while (time(NULL) < deadline)
    {
        // sleep for 1 seconds
        pthread_sleep(1);
        int current_time = time(NULL) - simulationStartTime;
        if (n <= current_time)
        {
            // lock the landing queue
            pthread_mutex_lock(&landingQueueMutex);
            // print the landing queue
            printf("At %d sec landing: ", current_time);
            PrintQueue(landingQueue);
            // unlock the landing queue
            pthread_mutex_unlock(&landingQueueMutex);

            // lock the launch queue
            pthread_mutex_lock(&launchQueueMutex);
            // print the launch queue
            printf("At %d sec launch: ", current_time);
            PrintQueue(launchQueue);
            // unlock the launch queue
            pthread_mutex_unlock(&launchQueueMutex);

            // lock the assembly queue
            pthread_mutex_lock(&assemblyQueueMutex);
            // print the assembly queue
            printf("At %d sec assembly: ", current_time);
            PrintQueue(assemblyQueue);
            // unlock the assembly queue
            pthread_mutex_unlock(&assemblyQueueMutex);

            // lock the padA queue
            pthread_mutex_lock(&padAQueueMutex);
            // print the padA queue
            printf("At %d sec padA: ", current_time);
            PrintQueue(padAQueue);
            // unlock the padA queue
            pthread_mutex_unlock(&padAQueueMutex);

            // lock the padB queue
            pthread_mutex_lock(&padBQueueMutex);
            // print the padB queue
            printf("At %d sec padB: ", current_time);
            PrintQueue(padBQueue);
            // unlock the padB queue
            pthread_mutex_unlock(&padBQueueMutex);

            // lock the padA emergency queue
            pthread_mutex_lock(&padAEmergencyQueueMutex);
            // print the padA emergency queue
            printf("At %d sec padA emergency: ", current_time);
            PrintQueue(padAEmergencyQueue);
            // unlock the padA emergency queue
            pthread_mutex_unlock(&padAEmergencyQueueMutex);

            // lock the padB emergency queue
            pthread_mutex_lock(&padBEmergencyQueueMutex);
            // print the padB emergency queue
            printf("At %d sec padB emergency: ", current_time);
            PrintQueue(padBEmergencyQueue);
            // unlock the padB emergency queue
            pthread_mutex_unlock(&padBEmergencyQueueMutex);
        }
    }
    return NULL;
}

void *PrintQueue(Queue *q)
{
    if (isEmpty(q))
    {
        printf("empty\n");
    }
    else
    {
        NODE *curr = q->head;
        while (curr != NULL)
        {
            printf("%d ", curr->data.ID);
            curr = curr->prev;
        }
        printf("\n");
    }
}