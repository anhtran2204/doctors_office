#include <iostream>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

/* Constants */
const int TIMER = 1;         // seconds of pause between steps
const int MAX_PATIENTS = 15; // max number of patient threads
const int MAX_DOCTORS = 3;   // max number of doctor threads

vector<pthread_t> doctors;
vector<pthread_t> nurses;
vector<pthread_t> patients;

/* Shared Semaphores */
// sem_t consumeReceptionist;
// sem_t produceReceptionist;
// sem_t waitingRoomSem;
// sem_t enteredSem;
// sem_t doctorSem;
// sem_t nurseSem;

/* Test Semaphores */
sem_t demoSem;
sem_t doctorSem;
sem_t nurseSem;
sem_t receptionistSem;
sem_t patientSem;

sem_t exited;

int NUM_DOCTORS;
int NUM_PATIENTS;

void init_sem()
{
    if (sem_init(&demoSem, 0, 1) == -1)
    {
        cerr << "Error: semaphore" << endl;
    }
    if (sem_init(&doctorSem, 0, 0) == -1)
    {
        cerr << "Error: semaphore" << endl;
    }
    if (sem_init(&nurseSem, 0, 0) == -1)
    {
        cerr << "Error: semaphore" << endl;
    }
    if (sem_init(&receptionistSem, 0, 1) == -1)
    {
        cerr << "Error: semaphore" << endl;
    }
    if (sem_init(&patientSem, 0, 0) == -1)
    {
        cerr << "Error: semaphore" << endl;
    }
    if (sem_init(&exited, 0, 1) == -1)
    {
        cerr << "Error: semaphore" << endl;
    }
}

void *run(void *id)
{
    sem_wait(&demoSem);
    printf("thread #%ld created!\n", (long)id);
    sleep(TIMER);
    sem_post(&demoSem);
    sem_wait(&exited);
    printf("finished thread %ld activity!\n", (long)id);
    sleep(TIMER);
    sem_post(&exited);
}

void *doctor(void *arg)
{
    long id = (long)arg;
    while (true)
    {
        sem_wait(&doctorSem);
        sleep(TIMER);
        printf("doctor %ld checking...\n", (long)id++);
        // sem_post(&doctorSem);
    }
}

void *nurse(void *arg)
{
    long id = (long)arg;
    while (true)
    {
        sem_wait(&nurseSem);
        sleep(TIMER);
        printf("nurse %ld attending...\n", (long)id++);
        sem_post(&nurseSem);
    }
}

void *receptionist(void *arg)
{
    long id = (long)arg;
    while (true)
    {
        sem_wait(&patientSem);
        printf("receptionist %ld registering...\n", (long)id++);
        sleep(TIMER);
        sem_post(&receptionistSem);
        sem_post(&exited);
    }
    return 0;
}

void *patient(void *arg)
{
    /* patient entering section */
    sem_post(&patientSem);
    printf("patient %ld entering...\n", (long)arg);
    sleep(TIMER);
    sem_wait(&receptionistSem);

    /* nurse section */
    // sem_post(&nurseSem);

    /* doctor section */

    /* patient leaving section */
    sem_wait(&exited);
    printf("patient %ld leaving...\n", (long)arg);
    sleep(TIMER);
    return 0;
}

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <number_of_doctors> <number_of_patients>", argv[0]);
        exit(-1);
    }
    if ((NUM_DOCTORS = atoi(argv[1])) > MAX_DOCTORS)
    {
        printf("Error: Too many doctors!\n");
        exit(-1);
    }
    if ((NUM_PATIENTS = atoi(argv[2])) > MAX_PATIENTS)
    {
        printf("Error: Too many patients!\n");
        exit(-1);
    }

    printf("Run with %d patients, %d nurses, %d doctors\n\n", NUM_PATIENTS, NUM_DOCTORS, NUM_DOCTORS);

    /* section for testing functions */
    init_sem();
    int rc;

    /* Todo:
        -   add an inifinite loop for recptionist() to make it
            continuously doing work for the patient thread
        -   probably add (??) loops also for doctor() and nurse()
        -   change the main loop to only create patient thread (??)
     */

    pthread_t receptionistThread;
    if (rc = pthread_create(&receptionistThread, NULL, receptionist, (void *)1L))
    {
        printf("Error: pthread_create() failed\n");
        exit(EXIT_FAILURE);
    }
    sleep(TIMER);
    // for (long i = 0; i < NUM_DOCTORS; i++)
    // {
    //     pthread_t doctorThread;
    //     if (rc = pthread_create(&doctorThread, NULL, doctor, (void *)1L))
    //     {
    //         printf("Error: pthread_create() failed\n");
    //         exit(EXIT_FAILURE);
    //     }
    //     sleep(TIMER);
    //     doctors.push_back(doctorThread);
    // }

    for (long i = 0; i < NUM_PATIENTS; i++)
    {
        // pthread_t thread;
        // if (rc = pthread_create(&thread, NULL, run, (void *)(i + 1)))
        // {
        //     printf("Error: pthread_create() failed\n");
        //     exit(EXIT_FAILURE);
        // }
        // sleep(TIMER);
        pthread_t patientThread;
        if (rc = pthread_create(&patientThread, NULL, patient, (void *)(i + 1)))
        {
            printf("Error: pthread_create() failed\n");
            exit(EXIT_FAILURE);
        }
        sleep(TIMER);
        // patients.push_back(thread);
        patients.push_back(patientThread);
    }

    // for (int i = 0; i < doctors.size(); i++)
    // {
    //     pthread_join(doctors.at(i), NULL);
    // }
    for (int i = 0; i < patients.size(); i++)
    {
        pthread_join(patients.at(i), NULL);
    }

    sem_destroy(&demoSem);
    sem_destroy(&doctorSem);
    sem_destroy(&nurseSem);
    sem_destroy(&patientSem);
    sem_destroy(&receptionistSem);
    printf("thread creation complete...\n");
    /*********************************/

    return 0;
}