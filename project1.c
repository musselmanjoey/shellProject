#include <stdio.h>
#include <stdlib.h>

int main( int argc, char *argv[] )  
	{
	
	char *myUSER = getenv("USER");	//declaring variables need for comandline
	char *myPWD = getenv("PWD");
	char *myMACHINE = getenv("MACHINE");
	//check to make sure they all got valid data
	if (myUSER != NULL && myPWD != NULL && myMACHINE != NULL)	
		printf("%s@%s :: %s ->\n", myUSER, myMACHINE, myPWD);
	else 
		printf("get env returned null");

	}
