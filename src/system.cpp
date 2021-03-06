#include "system.h"

pthread_mutex_t System::ioMutex;
pthread_mutex_t System::protectMutex;     // include protectMutex by kevin
pthread_barrier_t System::syncBarr;       // include syncBarr by kevin
pthread_spinlock_t System::protectLock;   // include protectLock by kevin
sem_t**** System::syncSem;                // include syncSem by kevin for part+

/**
 * Set up the threadSet dependent on the inputfile.
 *
 */
System::System ()
{
    /*~~~~~~~~~kevin code(PART+) create 4d Semaphores~~~~~~~~*/
    System::syncSem  = new sem_t*** [PROGRAM_NUM]; // each flag for task to wait
    for (int i = 0; i < PROGRAM_NUM; i++)
    {
        System::syncSem[i]  = new sem_t** [THREAD_NUM]; // each flag for task to wait
        for (int j = 0; j < THREAD_NUM; j++)
        {
            System::syncSem[i][j]  = new sem_t* [PROGRAM_NUM]; // diff task state in flag
            for (int k = 0; k < PROGRAM_NUM; k++)
                System::syncSem[i][j][k] = new sem_t [THREAD_NUM];  // diff task state in flag
        }
    }
    /*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/

    init ();
    setUpMatrix (); // Inital singleResult, multiResult, matrix, and inpoutMatrix

    threadSet = new Thread* [PROGRAM_NUM];
    check = new Check [PROGRAM_NUM];

    for (int prog_index = 0; prog_index < PROGRAM_NUM; prog_index++) {

	    threadSet [prog_index] = new Thread [THREAD_NUM];
        check [prog_index].initialCheck (prog_index,
                                         singleResult [prog_index], 
                                         multiResult [prog_index],
                                         MATRIX_SIZE);

    }

    for (int prog_index = 0; prog_index < PROGRAM_NUM; prog_index++) {

        for (int thread_index = 0; thread_index < THREAD_NUM; thread_index++) {

	    	threadSet [prog_index][thread_index].initialThread (prog_index,
                                                                thread_index,
                                                                MATRIX_SIZE,
                                                                singleResult [prog_index],
                                                                multiResult [prog_index],
                                                                matrix [prog_index], 
                                                                inputMatrix [prog_index],
                                                                &sharedSum [prog_index]);

            threadSet [prog_index][thread_index].setThreadCore (thread_index);

            threadSet [prog_index][thread_index].setStartCalculatePoint (thread_index * MATRIX_SIZE / THREAD_NUM);
            threadSet [prog_index][thread_index].setEndCalculatePoint ((thread_index + 1) * MATRIX_SIZE / THREAD_NUM);

            threadSet [prog_index][thread_index].setUpIOMutex (&System::ioMutex);
            threadSet [prog_index][thread_index].setUpCountMutex (&System::protectMutex); // setUpCountMutex by kevin
            threadSet [prog_index][thread_index].setUpBarrier (&System::syncBarr); // setUpBarrier by kevin
            threadSet [prog_index][thread_index].setUpLock (&System::protectLock); // setUpLock by kevin
            threadSet [prog_index][thread_index].setUpSem (System::syncSem); // setUpSem by kevin for part+

        }

    }

}


/**
 * Set up 2-D matrix for singleResult, multiResult,
 * matrix, and inputMatrix as the size of 
 * matrix_size * matrix_size. Also inclue a initial
 * value of inputMatrix.
 *
 */
void
System::setUpMatrix ()
{
	singleResult  = new int** [PROGRAM_NUM];
	multiResult   = new int** [PROGRAM_NUM];
	matrix        = new int** [PROGRAM_NUM];
	inputMatrix   = new int** [PROGRAM_NUM];

    for (int prog_index = 0; prog_index < PROGRAM_NUM; prog_index++) {

	    singleResult [prog_index]  = new int* [MATRIX_SIZE];
	    multiResult [prog_index]   = new int* [MATRIX_SIZE];
	    matrix [prog_index]        = new int* [MATRIX_SIZE];
	    inputMatrix [prog_index]   = new int* [MATRIX_SIZE];

        for (int i = 0; i < MATRIX_SIZE; i++) {
        	singleResult [prog_index][i] = new int [MATRIX_SIZE];
        	multiResult [prog_index][i]  = new int [MATRIX_SIZE];
        	matrix [prog_index][i]       = new int [MATRIX_SIZE];
        	inputMatrix [prog_index][i]  = new int [MATRIX_SIZE];
        
        	for (int j = 0; j < MATRIX_SIZE; j++)
        		inputMatrix [prog_index][i][j] = rand () % 2;
        }

    }
    

}


void
System::init ()
{

    std::cout << "\n========================System Info===========================" << std::endl; 
    std::cout << "Protect Shared Resource: ";
#if PROTECT_SHARED_RESOURCE == MUTEX
    std::cout << "Mutex" << std::endl;
#else
    std::cout << "Spinlock" << std::endl;
#endif

    std::cout << "Synchronize: ";
#if SYNCHRONIZE == BARRIER
    std::cout << "Barrier" << std::endl;
#else
    std::cout << "Semaphore" << std::endl;
#endif

    sharedSum = new int [PROGRAM_NUM];

	/*~~~~~~~~~Your code(PART1&PART3)~~~~~~~~*/
    pthread_barrier_init (&System::syncBarr, NULL, THREAD_NUM * PROGRAM_NUM); // barrier_init by kevin
    pthread_spin_init (&System::protectLock, PTHREAD_PROCESS_PRIVATE); // spin_init by kevin
	/*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
    /*~~~~~~~~~kevin code(PART+)~~~~~~~~*/
    for (int i = 0; i < PROGRAM_NUM; i++)
        for(int j = 0; j < THREAD_NUM; j++)
            for(int k = 0; k < PROGRAM_NUM; k++)
                for(int x = 0; x < THREAD_NUM; x++)
                    sem_init (&System::syncSem[i][j][k][x], 0, 0);
    /*~~~~~~~~~~~~~~~~~~END~~~~~~~~~~~~~~~~~~*/
}


/**
 * Execute the single thread matrix multiplication and 
 * store the result at singleResult.
 *
 */
void
System::singleCoreMatrixMulti ()
{
    std::cout << "\n===========Start Single Thread Matrix Multiplication==========" << std::endl; 

    for (int prog_index = 0; prog_index < PROGRAM_NUM; prog_index++)
        threadSet [prog_index][0].resetMatrix (); // copy the data from inputMatrix to matrix

    setStartTime ();
    for (int prog_index = 0; prog_index < PROGRAM_NUM; prog_index++)
	    threadSet [prog_index][0].singleMatrixMultiplication ();
    setEndTime ();

    std::cout << "Single-thread spend time : " << timeUse << std::endl;
}


/**
 * Creating multi-thread for matrix multiplication.
 *
 */
void
System::multiCoreMatrixMulti ()
{
    std::cout << "\n===========Start Multi-Thread Matrix Multiplication===========" << std::endl; 
    for (int prog_index = 0; prog_index < PROGRAM_NUM; prog_index++)
        threadSet [prog_index][0].resetMatrix (); // copy the data from inputMatrix to matrix

    setStartTime ();

    for (int prog_index = 0; prog_index < PROGRAM_NUM; prog_index++) {

	    for (int thread_index = 0; thread_index < THREAD_NUM; thread_index++) {

	    	pthread_create (&(threadSet [prog_index][thread_index].pthreadThread),
                            NULL,
                            (THREADFUNCPTR)&Thread::matrixMultiplication,
                            &threadSet [prog_index][thread_index]);
        }

    }

    for (int prog_index = 0; prog_index < PROGRAM_NUM; prog_index++) {

	    for (int thread_index = 0; thread_index < THREAD_NUM; thread_index++) 
	    	pthread_join (threadSet [prog_index][thread_index].pthreadThread, NULL);

    }

    setEndTime ();

    std::cout << "Multi-thread spend time : " << timeUse << std::endl;

    std::cout << "\n============================Result============================" << std::endl; 
    for (int prog_index = 0; prog_index < PROGRAM_NUM; prog_index++) {
        check [prog_index].compareResult ();
    }

}


/**
 * Set the current time as start time.
 *
 */
void
System::setStartTime ()
{
	gettimeofday (&start, NULL);
}


/**
 * Set the current time as end time. Calculate the time 
 * interval between start time and end time.
 *
 */
void
System::setEndTime ()
{
	gettimeofday (&end, NULL);
	timeUse = (end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec)/1000000.0;
}
