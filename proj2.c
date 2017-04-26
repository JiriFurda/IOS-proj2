/*
[IOS] Projekt 2 - Zprava procesu
Autor: Jiri Furda (xfurda00)
*/

#include <stdio.h>

int main(int argc, char *argv[])
{
	int aParam;
	int cParam;
	int agtParam;
	int cgtParam;
	int watParam;
	int cwtParam;
	
	if(argc != 7)
	{
		fprintf(stderr,"Error: Invalid argument count\n");
		return 1;
	}
	
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

	return 0;
}
