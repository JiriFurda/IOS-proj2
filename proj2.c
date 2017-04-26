/*
[IOS] Projekt 2 - Zprava procesu
Autor: Jiri Furda (xfurda00)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[])
{
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
	
	
	int pid = fork();
	
	if(pid == 0)
	{
		printf("I'm child (%d)\n",pid);
		// exec(....), exit(exitcode)
	}
	else if(pid == -1)
	{
		fprintf(stderr,"Error: Fork failed\n");
		errnum = errno;
		fprintf(stderr, "Error #%d opening file: %s\n", errnum, strerror(errnum));
		return 1;
	}
	else
	{
		printf("I'm parent (child=%d)\n",pid);
		// pid2 = wait(&stav);
	}

	return 0;
}
