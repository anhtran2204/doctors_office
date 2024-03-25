#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <unistd.h>

using namespace std;

// Global Variables
int registerPatientID;
int officeNumber;
int doctor_num;
int nurse_num;
int patient_num;
vector<int> waiting_for_office(3, 0);
vector<int> in_office(3, 0);

// POSIX Semaphores
sem_t patient_enters;
sem_t receptionist_ready;
sem_t receptionist_registers;
sem_t declared_office;
sem_t receptionist_leaves;
vector<sem_t> patient_waiting;
vector<sem_t> nurse_notified;
vector<sem_t> nurse_available;
vector<sem_t> nurse_directs_to_office;
vector<sem_t> patient_waits_in_office;
vector<sem_t> doctor_notified;
vector<sem_t> doctor_available;
vector<sem_t> doctor_listens_to_symptoms;
vector<sem_t> doctor_advises;
vector<sem_t> patient_leaves;

// Function Prototypes
void* Receptionist(void* arg);
void* Doctor(void* arg);
void* Patient(void* arg);
void* Nurse(void* arg);
void init_semaphores();
void destroy_semaphores();

int main(int argc, char* argv[]) {
    if (argc != 3)
    {
        printf("Usage: %s <number_of_doctors> <number_of_patients>", argv[0]);
        return -1;
    }

    doctor_num = atoi(argv[1]);
    if (doctor_num > 3) {
        cerr << "Error: Too many doctors!" << endl;
        return -1;
    }

    patient_num = atoi(argv[2]);
    if (patient_num > 30) {
        cerr << "Error: Too many patients!" << endl;
        return -1;
    }

    nurse_num = doctor_num; // Nurse number should match doctor

    cout << "Run with " << doctor_num << " doctors, " << nurse_num << " nurses, " << patient_num << " patients" << endl << endl;

    srand(time(NULL)); // Seed the random number generator
    init_semaphores();

    pthread_t receptionistThread;
    pthread_create(&receptionistThread, nullptr, Receptionist, nullptr);
    sleep(1);

    vector<pthread_t> doctorThreads(doctor_num);
    vector<pthread_t> nurseThreads(nurse_num);
    vector<pthread_t> patientThreads(patient_num);

    for (int i = 0; i < doctor_num; i++) {
        pthread_create(&doctorThreads[i], nullptr, Doctor, (void*)(intptr_t)i);
        pthread_create(&nurseThreads[i], nullptr, Nurse, (void*)(intptr_t)i);
        sleep(1);
    }

    for (int i = 0; i < patient_num; i++) {
        pthread_create(&patientThreads[i], nullptr, Patient, (void*)(intptr_t)i);
        sleep(1);
    }

    // Terminating all threads 

    for (int i = 0; i < patient_num; i++) {
        pthread_join(patientThreads[i], nullptr);
    }
    cout << "Simulation complete..." << endl; 

    return 0;
}

void init_semaphores() {
    sem_init(&patient_enters, 0, 0);
    sem_init(&receptionist_ready, 0, 0);
    sem_init(&receptionist_registers, 0, 0);
    sem_init(&declared_office, 0, 0);
    sem_init(&receptionist_leaves, 0, 0);

    patient_waiting.resize(doctor_num);
    nurse_notified.resize(doctor_num);
    nurse_available.resize(doctor_num);
    nurse_directs_to_office.resize(doctor_num);
    patient_waits_in_office.resize(doctor_num);
    doctor_notified.resize(doctor_num);
    doctor_available.resize(doctor_num);
    doctor_listens_to_symptoms.resize(doctor_num);
    doctor_advises.resize(doctor_num);
    patient_leaves.resize(doctor_num);

    for (int i = 0; i < doctor_num; i++) {
        sem_init(&patient_waiting[i], 0, 0);
        sem_init(&nurse_notified[i], 0, 0);
        sem_init(&nurse_available[i], 0, 0);
        sem_init(&nurse_directs_to_office[i], 0, 0);
        sem_init(&patient_waits_in_office[i], 0, 0);
        sem_init(&doctor_notified[i], 0, 0);
        sem_init(&doctor_available[i], 0, 0);
        sem_init(&doctor_listens_to_symptoms[i], 0, 0);
        sem_init(&doctor_advises[i], 0, 0);
        sem_init(&patient_leaves[i], 0, 1); // Initialized to 1 because the office is initially empty
    }
}

void destroy_semaphores() {
    // Destroy global semaphores
    sem_destroy(&patient_enters);
    sem_destroy(&receptionist_ready);
    sem_destroy(&receptionist_registers);
    sem_destroy(&declared_office);
    sem_destroy(&receptionist_leaves);

    // Destroy semaphores within vectors
    for (int i = 0; i < doctor_num; i++) {
        sem_destroy(&patient_waiting[i]);
        sem_destroy(&nurse_notified[i]);
        sem_destroy(&nurse_available[i]);
        sem_destroy(&nurse_directs_to_office[i]);
        sem_destroy(&patient_waits_in_office[i]);
        sem_destroy(&doctor_notified[i]);
        sem_destroy(&doctor_available[i]);
        sem_destroy(&doctor_listens_to_symptoms[i]);
        sem_destroy(&doctor_advises[i]);
        sem_destroy(&patient_leaves[i]);
    }
}

void* Receptionist(void* arg) {
    while (true) {
        sem_post(&receptionist_ready);
        sem_wait(&patient_enters);

        sem_wait(&receptionist_registers);
        cout << "Receptionist registers patient " << registerPatientID << "." << endl;
        sleep(1);

        officeNumber = rand() % doctor_num;
        sem_post(&declared_office);

        sem_post(&nurse_notified[officeNumber]);

        sem_wait(&receptionist_leaves);
    }
    return nullptr;
}

void* Doctor(void* arg) {
    int doctorID = (intptr_t)arg;
    while (true) {
        sem_post(&doctor_available[doctorID]);

        sem_wait(&doctor_notified[doctorID]);
        sem_wait(&patient_waits_in_office[doctorID]);

        sem_wait(&doctor_listens_to_symptoms[doctorID]);
        cout << "Doctor " << doctorID << " listens to symptoms from patient " << in_office[doctorID] << "." << endl;
        sleep(1);

        sem_post(&doctor_advises[doctorID]);
    }
    return nullptr;
}

void* Nurse(void* arg) {
    int nurseID = (intptr_t)arg;
    while (true) {
        sem_post(&nurse_available[nurseID]);

        sem_wait(&patient_waiting[nurseID]);
        sem_wait(&nurse_notified[nurseID]);

        sem_wait(&patient_leaves[nurseID]);

        cout << "Nurse " << nurseID << " takes patient " << waiting_for_office[nurseID] << " to doctor's office." << endl;
        sleep(1);
        
        sem_post(&nurse_directs_to_office[nurseID]);

        sem_post(&doctor_notified[nurseID]);
    }
    return nullptr;
}

void* Patient(void* arg) {
    int patientID = (intptr_t)arg;
    sem_wait(&receptionist_ready);
    
    sem_post(&patient_enters);
    cout << "Patient " << patientID << " enters waiting room, waits for receptionist." << endl;
    sleep(1);
    
    registerPatientID = patientID;
    sem_post(&receptionist_registers);

    sem_wait(&declared_office);
    int assignedOffice = officeNumber;
    cout << "Patient " << patientID << " leaves receptionist and sits in waiting room." << endl;
    sleep(1);

    sem_post(&receptionist_leaves);

    sem_post(&patient_waiting[assignedOffice]);
    sem_wait(&nurse_available[assignedOffice]);
    waiting_for_office[assignedOffice] = patientID;
    sem_wait(&nurse_directs_to_office[assignedOffice]);

    cout << "Patient " << patientID << " enters doctor " << assignedOffice << "'s office." << endl;
    sleep(1);

    in_office[assignedOffice] = patientID;
    sem_post(&patient_waits_in_office[assignedOffice]);
    sem_wait(&doctor_available[assignedOffice]);

    sem_post(&doctor_listens_to_symptoms[assignedOffice]);
    sem_wait(&doctor_advises[assignedOffice]);
    cout << "Patient " << patientID << " receives advice from doctor " << assignedOffice << "." << endl;
    sleep(1);

    cout << "Patient " << patientID << " leaves." << endl;
    sleep(1);

    sem_post(&patient_leaves[assignedOffice]);

    return nullptr;
}