/*
[IOS] Projekt 2 - Synchronizace procesu
Autor: Jiri Furda (xfurda00)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/types.h> // why?
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>


// Semaphores
sem_t *lineSemaphore;
sem_t *adultCounterSemaphore; // jen counter nebo celkove semafor?
sem_t *childCounterSemaphore;
sem_t *adultsInCenterSemaphore;
sem_t *childrenInCenterSemaphore;
sem_t *centerChangeSemaphore;
sem_t *adultLeaveSemaphore;
sem_t *childEnterSemaphore;

// Global variables
int lineCounterId = 0;
int adultCounterId = 0;
int childCounterId = 0;
int adultsInCenterId = 0;
int childrenInCenterId = 0;
int noMoreAdultsId = 0;
int *lineCounter = NULL;
int *adultCounter = NULL;
int *childCounter = NULL;
int *adultsInCenter = NULL;
int *childrenInCenter = NULL;
int *noMoreAdults = NULL;

// Function prototypes
void exitError(char *msg);
void clean();
void init();
void randSleep(int time);
void createSharedMemory(int id, int *content);
void childFactory(int childCount, int agt, int awt);
void adultFactory(int adultCount, int cgt, int cwt);
void appendToFile(char type, int id, char *msg);
void appendWaitingToFile(char type, int id, int *adults, int *children);


/**
 * Main function
 */
int main(int argc, char *argv[])
{
	// --- Argument processing ---
	if(argc != 7)
	{
		fprintf(stderr,"Error: Invalid argument count\n");
		return 1;
	}
	
	int aParam = atoi(argv[1]); // test when not a number
	int cParam = atoi(argv[2]);
	int agtParam = atoi(argv[3]);
	int cgtParam = atoi(argv[4]);
	int awtParam = atoi(argv[5]);
	int cwtParam = atoi(argv[6]);
	
	if(!(aParam > 0))
	{
		fprintf(stderr,"Error: Parameter \"A\" must be positive number\n");
		return 1;
	}
	else if(!(cParam > 0))
	{
		fprintf(stderr,"Error: Parameter \"C\" must be positive number\n");
		return 1;
	}
	else if(!(agtParam >= 0 && agtParam < 5001))
	{
		fprintf(stderr,"Error: Parameter \"AGT\" must be number between 0 and 5001\n");
		return 1;
	}
	else if(!(cgtParam >= 0 && cgtParam < 5001))
	{
		fprintf(stderr,"Error: Parameter \"CGT\" must be number between 0 and 5001\n");
		return 1;
	}
	else if(!(awtParam >= 0 && awtParam < 5001))
	{
		fprintf(stderr,"Error: Parameter \"AWT\" must be number between 0 and 5001\n");
		return 1;
	}
	else if(!(cwtParam >= 0 && cwtParam < 5001))
	{
		fprintf(stderr,"Error: Parameter \"CWT\" must be number between 0 and 5001\n");
		return 1;
	}
	
	
	
	// --- Clearing output file ---
	FILE *file = fopen("proj2.out", "w");
	if (file == NULL)
		exitError("Opening file for clear failed");
	fclose(file);
	
	
	// --- Initializing ---
	init();
	srand(time(NULL));
	
	
	// --- Adult factory ---
	pid_t adultFactoryPID = fork();
	if(adultFactoryPID == -1)
		exitError("Adult factory fork failed");
	
	if(adultFactoryPID == 0)
	{
		adultFactory(aParam,agtParam,awtParam);
	}
	
	
	
	// --- Child factory ---
	pid_t childFactoryPID = fork();
    
    if(childFactoryPID == -1)
        exitError("Child factory fork failed");
	
	if(childFactoryPID == 0)
	{
		childFactory(cParam,cgtParam,cwtParam);
	}
		
		
		
	// --- Ending main process ---
	waitpid(childFactoryPID, NULL, 0);
	waitpid(adultFactoryPID, NULL, 0);
    clean();
    
	return 0;
}


/**
 * Adult factory creating adult processes using fork
 * @param adultCount Count of processes to create
 * @param agt Maximal length between generating
 * @param awt Maximal length for waiting
 */
void adultFactory(int adultCount, int agt, int awt)
{	
	for(int i = 0; i < adultCount; i++)
	{	
		pid_t pid = fork();
		
		if(pid == 0)
		{
			// Starting adult process
			sem_wait(adultCounterSemaphore);
			int id = (*adultCounter)++;
			sem_post(adultCounterSemaphore);

			appendToFile('A',id,"started");
			
			
			// Entering child center
			sem_wait(centerChangeSemaphore);
			
			sem_wait(adultsInCenterSemaphore);
			(*adultsInCenter)++;
			sem_post(adultsInCenterSemaphore);
			
			if(!((*childrenInCenter)+1 > (*adultsInCenter) * 3))
				sem_post(childEnterSemaphore);
			
			sem_post(centerChangeSemaphore);
			
			appendToFile('A',id,"enter");
			
			
			// Sleeping
			randSleep(awt);
			appendToFile('A',id,"trying to leave");
			

			// Checking child center requirements
			sem_wait(centerChangeSemaphore);
			if(*childrenInCenter > ((*adultsInCenter)-1) * 3)
			{
				sem_post(centerChangeSemaphore);

				appendWaitingToFile('A',id,childrenInCenter,adultsInCenter);
				
				sem_wait(adultLeaveSemaphore);
			}


			// Leaving child center
			sem_wait(adultsInCenterSemaphore);
			(*adultsInCenter)--;
			sem_post(adultsInCenterSemaphore);
			
			if(*noMoreAdults == -1 && *adultsInCenter == 0)
				*noMoreAdults = 1;
			
			sem_post(centerChangeSemaphore);
			
			appendToFile('A',id,"leave");
			
			
			// Finishing adult process
			appendToFile('A',id,"finished");
			exit(0);
		}
		
		// Sleeping
		randSleep(agt);
	}
	*noMoreAdults = -1;

	exit(0);	
}


/**
 * Child factory creating child processes using fork
 * @param childCount Count of processes to create
 * @param cgt Maximal length between generating
 * @param cwt Maximal length for waiting
 */
void childFactory(int childCount, int cgt, int cwt)
{
	for(int i = 0; i < childCount; i++)
	{
		pid_t pid = fork();
		
		if(pid == 0)
		{
			// Starting child process
			sem_wait(childCounterSemaphore);
			int id = (*childCounter)++;
			sem_post(childCounterSemaphore);
			
			appendToFile('C',id,"started");
			
			sem_wait(centerChangeSemaphore);
			
			
			// Checking child center requirements
			if(*noMoreAdults != 1 && (*childrenInCenter)+1 > (*adultsInCenter) * 3) // asi se bude resit jinak, ale jak???
			{
				sem_post(centerChangeSemaphore);
				
				// Waiting
				appendWaitingToFile('C',id,childrenInCenter,adultsInCenter);
				
				sem_wait(childEnterSemaphore);
				
			}
			
			
			// Entering child centre
			sem_wait(childrenInCenterSemaphore);
			(*childrenInCenter)++;
			sem_post(childrenInCenterSemaphore);
			
			sem_post(centerChangeSemaphore);
			
			appendToFile('C',id,"enter"); 
			
			
			
			// Sleeping
			randSleep(cwt);
			appendToFile('C',id,"trying to leave"); 
			
			
			// Leaving child center
			sem_wait(centerChangeSemaphore);
			
			sem_wait(childrenInCenterSemaphore);
			(*childrenInCenter)--;
			sem_post(childrenInCenterSemaphore);
			
			if(!(*childrenInCenter > ((*adultsInCenter)-1) * 3))
				sem_post(adultLeaveSemaphore);
			
			sem_post(centerChangeSemaphore);
			
			appendToFile('C',id,"leave"); 
			
			
			// Finishing child process
			appendToFile('C',id,"finished");
			exit(0); 
		}
		
		// Sleeping
		randSleep(cgt);
	}

	exit(0);
}


/**
 * Initilizing shared memory (+ it's default vaules) and creating semaphores
 */
void init()
{
	// Creating shared memory (could use own function)
	if((lineCounterId = shmget(IPC_PRIVATE, sizeof (int), IPC_CREAT | 0666)) == -1) 
			exitError("Creating shared memory failed");
	if((lineCounter = (int *) shmat(lineCounterId, NULL, 0)) == NULL) 
			exitError("Loading shared memory failed");
			
	if((adultCounterId = shmget(IPC_PRIVATE, sizeof (int), IPC_CREAT | 0666)) == -1) 
			exitError("Creating shared memory failed");
	if((adultCounter = (int *) shmat(adultCounterId, NULL, 0)) == NULL) 
			exitError("Loading shared memory failed");
			
	if((childCounterId = shmget(IPC_PRIVATE, sizeof (int), IPC_CREAT | 0666)) == -1) 
			exitError("Creating shared memory failed");
	if((childCounter = (int *) shmat(childCounterId, NULL, 0)) == NULL) 
			exitError("Loading shared memory failed");

	if((adultsInCenterId = shmget(IPC_PRIVATE, sizeof (int), IPC_CREAT | 0666)) == -1) 
			exitError("Creating shared memory failed");
	if((adultsInCenter = (int *) shmat(adultsInCenterId, NULL, 0)) == NULL) 
			exitError("Loading shared memory failed");
			
	if((childrenInCenterId = shmget(IPC_PRIVATE, sizeof (int), IPC_CREAT | 0666)) == -1) 
			exitError("Creating shared memory failed");
	if((childrenInCenter = (int *) shmat(childrenInCenterId, NULL, 0)) == NULL) 
			exitError("Loading shared memory failed"); 
			
	 if((noMoreAdultsId = shmget(IPC_PRIVATE, sizeof (int), IPC_CREAT | 0666)) == -1) 
			exitError("Creating shared memory failed");
	if((noMoreAdults = (int *) shmat(noMoreAdultsId, NULL, 0)) == NULL) 
			exitError("Loading shared memory failed"); 


	// Setting default value of shared memory
	*lineCounter = 1;
	*adultCounter = 1;
	*childCounter = 1;
	
	*adultsInCenter = 0;
	*childrenInCenter = 0;
	
	*noMoreAdults = 0;
	
	// Creating semaphores
	if((lineSemaphore = sem_open("line", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) 
			exitError("Creating line semaphore failed");
	if((adultCounterSemaphore = sem_open("adultCounter", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) 
			exitError("Creating adult counter semaphore failed");
	if((childCounterSemaphore = sem_open("childCounter", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) 
			exitError("Creating child counter semaphore failed");
	if((adultsInCenterSemaphore = sem_open("adultsInCenter", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) 
			exitError("Creating child counter semaphore failed");
	if((childrenInCenterSemaphore = sem_open("childrenInCenter", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) 
			exitError("Creating child counter semaphore failed");
	if((centerChangeSemaphore = sem_open("centerChange", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) 
			exitError("Creating child counter semaphore failed");
	if((adultLeaveSemaphore = sem_open("adultLeave", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) 
			exitError("Creating child counter semaphore failed");
	if((childEnterSemaphore = sem_open("childEnter", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) 
			exitError("Creating child counter semaphore failed");
}


/**
 * Append new line at the end of output file with action log
 * @param type Character 'C' for child process or character 'A' for adult process
 * @param id ID of the process (starting from 1)
 * @param msg Type of action
 */
void appendToFile(char type, int id, char *msg)
{
	sem_wait(lineSemaphore);
	
	// Opening file
	FILE *file = fopen("proj2.out", "a");
	if (file == NULL)
		exitError("Opening file failed");
	
	// Writing to file and closing
	fprintf(file, "%d\t: %c %d\t: %s\n", (*lineCounter)++, type, id,  msg);
	fclose(file);
	
	sem_post(lineSemaphore);
}


/**
 * Append new line at the end of output file with action log about waiting
 * @param type Character 'C' for child process or character 'A' for adult process
 * @param id ID of the process (starting from 1)
 * @param msg Type of action
 */
void appendWaitingToFile(char type, int id, int *adults, int *children)
{
	sem_wait(lineSemaphore);
	
	// Opening file
	FILE *file = fopen("proj2.out", "a");
	if (file == NULL)
		exitError("Opening file failed");
	
	// Writing to file and closing
	fprintf(file, "%d\t: %c %d\t: waiting : %d : %d\n", (*lineCounter)++, type, id, *adults, *children);
	fclose(file);
	
	sem_post(lineSemaphore);
}


/**
 * Sleep for random time
 * @param time Maximal length of sleep in miliseconds
 */
void randSleep(int time)
{
	if(time > 0)
		usleep(((time * 2) - (rand() % time)) * 1000);
}

/**
 * Print brief error message, error number and more details to stderr and then exits program with value 2
 * @param msg Brief error message
 */
void exitError(char *msg)
{
	fprintf(stderr,"Error: %s\n", msg);
	int errnum = errno;
	fprintf(stderr, "Error #%d details: %s\n", errnum, strerror(errnum));
	exit(2);
}


/**
 * Clear resources
 */
void clean()
{
	// Cleaning shared memory
    shmctl(lineCounterId, IPC_RMID, NULL);
    shmctl(adultCounterId, IPC_RMID, NULL);
    shmctl(childCounterId, IPC_RMID, NULL);
    shmctl(adultsInCenterId, IPC_RMID, NULL);
    shmctl(childrenInCenterId, IPC_RMID, NULL);
    
    // Closing semaphores
    sem_close(lineSemaphore);
    sem_close(adultCounterSemaphore);
    sem_close(childCounterSemaphore);
    sem_close(adultsInCenterSemaphore);
    sem_close(childrenInCenterSemaphore);
    sem_close(centerChangeSemaphore);
    sem_close(adultLeaveSemaphore);
    sem_close(childEnterSemaphore);
    
    // Unlinking semaphores
    sem_unlink("line");
    sem_unlink("adultCounter");
    sem_unlink("childCounter");
    sem_unlink("adultsInCenter");
    sem_unlink("childrenInCenter");
    sem_unlink("centerChange");
    sem_unlink("adultLeave");
    sem_unlink("childEnter");
}
