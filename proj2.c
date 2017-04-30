/*
[IOS] Projekt 2 - Zprava procesu
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

sem_t *sharedMemorySem;
int lineCounterId = 0;
int *lineCounter = NULL;

void exitError(char *msg);
void clean();
void init();
void childFactory();
void adultFactory();
void appendToFile(char *msg);

int main(int argc, char *argv[])
{
	// Argument processing
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
	
	// Clearing output file
	FILE *file = fopen("proj2.out", "w");
	if (file == NULL)
		exitError("Opening file for clear failed");
	fclose(file);
	
	init();
	
	appendToFile("kokot");
	appendToFile("kokot");
	appendToFile("kokot");
	appendToFile("kokot");
	appendToFile("kokot");
	
	// Adult processes
	pid_t pid = fork();
    
    if(pid == -1)
	{
        exitError("Fork failed");
	}
	
	
	if(pid == 0)
	{
		childFactory(cParam);
	}
	else
	{
		printf("I'm main (child factory = %d)\n",pid);
		
		pid_t pid = fork();
		if(pid == -1)
		{
			exitError("Fork 2 failed");
		}
		
		if(pid == 0)
		{
			adultFactory(aParam);
		}
		else
		{
			printf("I'm still main (adult factory = %d)\n",pid);
			
			int status;
			//pid_t pid2 = wait(&status);
			wait(&status);
		}
	}

    clean();
    
	return 0;
}

void childFactory(int childCount)
{
	printf("I'm child factory\n");
	
	for(int i = 0; i < childCount; i++)
	{
		pid_t pid = fork();
		
		if(pid == 0)
		{
			printf("I'm child #%d (%d)\n",i,pid);
			exit(0);
		}
		else
		{
			printf("I'm still child factory (child = %d)\n",pid);
		}
	}

	exit(0);
}

void adultFactory(int adultCount)
{
	printf("I'm adult factory\n");
	
	for(int i = 0; i < adultCount; i++)
	{
		pid_t pid = fork();
		
		if(pid == 0)
		{
			printf("I'm adult #%d (%d)\n",i,pid);
			exit(0);
		}
		else
		{
			printf("I'm still adult factory (adult = %d)\n",pid);
		}
	}

	exit(0);	
}

void init()
{
    if ((lineCounterId = shmget(IPC_PRIVATE, sizeof (int), IPC_CREAT | 0666)) == -1) 
    { 
        exitError("Creating shared memory failed");
    }
    
    if ((lineCounter = (int *) shmat(lineCounterId, NULL, 0)) == NULL) 
    { 
        exitError("Loading shared memory failed");
    }

    *lineCounter = 1;
    
    if ((sharedMemorySem = sem_open("/proj2", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) 
    { 
        exitError("Creating shared memory semaphore failed");
    }
}

void appendToFile(char *msg)
{
	sem_wait(sharedMemorySem);
	
	FILE *file = fopen("proj2.out", "a");
	if (file == NULL)
		exitError("Opening file failed");
		
	fprintf(file, "%d\t:%s\n", (*lineCounter)++,  msg);
	fclose(file);
	
	sem_post(sharedMemorySem);
}

void exitError(char *msg)
{
	fprintf(stderr,"Error: %s\n", msg);
	int errnum = errno;
	fprintf(stderr, "Error #%d details: %s\n", errnum, strerror(errnum));
	exit(2);
}

void clean()
{
    shmctl(lineCounterId, IPC_RMID, NULL);
    sem_close(sharedMemorySem);
    sem_unlink("/proj2");
}
