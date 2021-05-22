#ifndef _THREAD_H_
#define _THREAD_H_

#include <cstring>
#include <iostream>
#include <unistd.h>

#include <pthread.h>
#include <sys/syscall.h>

#include "config.h"

#include <semaphore.h> //include semaphore by kevin

class Thread
{

  public:
	void initialThread (int, int, int, int**, int**, int**, int**, int*);
    void setUpIOMutex (pthread_mutex_t* tmp_mutex) {ioMutex = tmp_mutex;};
    void setUpCountMutex (pthread_mutex_t* tmp_mutex) {protectMutex = tmp_mutex;}; // create setUpCountMutex by kevin
    void setUpBarrier (pthread_barrier_t* tmp_barr) {syncBarr = tmp_barr;};        // create setUpBarrier by kevin
    void setUpLock (pthread_spinlock_t* tmp_lock) {protectLock = tmp_lock;};       // create setUpLock by kevin
    void setUpSem (sem_t**** tmp_sem) {syncSem = tmp_sem;};                        // create setUpSem by kevin part+

	void setThreadCore (int);            // Set core thread pinned on
    void setStartCalculatePoint (int);   // Set start calculate point for mul
    void setEndCalculatePoint (int);     // Set the end calculate point for mul 
	
	void setUpCPUAffinityMask ();        // Pined the thread on specify core
    void resetMatrix ();                 // Reset matrix value
	void printInformation ();            // Print out the thread informaiton.

    void synchronize ();                 // Synchronize between thread
    void enterCriticalSection ();
    void exitCriticalSection ();

	void singleMatrixMultiplication ();  // Single thread matrix multiplication.
    static void* matrixMultiplication (void*); // Multi-thread matrix multiplication


  public:
    pthread_t pthreadThread;

  private:
    int programID;
	int ID;                              // ID that user assign
	int PID;                             // Process ID which system assign

	int setCore = -1;                    // Core thread pinned on (user assign)
	int core;                            // Core thread executed on (system assign)

	int matrixSize;	                     // Matrix width or height
    int startCalculatePoint;             // Start point for matrix multiplicatio
    int endCalculatePoint;               // End poiint for matrix multiplication

	int **matrix;                        // Matrix for multiplication
	int **inputMatrix;                   // Store for original input matrix 
	int **singleResult;                  // Single-core matrix multiplication result
	int **multiResult;		             // Multi-core matrix multiplication result

    pthread_mutex_t* ioMutex;            // IO mutex
    pthread_mutex_t* protectMutex;       // create pthread_mutex_t by kevin
    pthread_barrier_t* syncBarr;         // create pthread_barrier_t by kevin
    pthread_spinlock_t* protectLock;     // create pthread_spinlock_t by kevin
    sem_t**** syncSem;                   // create sem_t by kevin part+

    int* sharedSum;                      // Shared resource

};
#endif
