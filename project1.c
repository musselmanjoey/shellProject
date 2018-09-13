#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>

char** addToken(char** instr, char* tok, int numTokens);
void printTokens(char** instr, int numTokens);
void my_execute(char **cmd);

int main( int argc, char *argv[] )
        {

        char *myUSER;  //declaring variables need for comandline
        char *myPWD;
        char *myMACHINE;
	//this was added
	
	//The following code is the parser help code from canvas
	char token[256];	//holds instruction token
	char ** bucket;		//array that holds all instruction tokens
	char temp[256];		//used to split tokens containing special characters
	int exit;
	exit = 0;		//used to track if program should end

	while(exit == 0){
		myUSER = getenv("USER");  //ensure everthing is right before running
        	myPWD = getenv("PWD");
        	myMACHINE = getenv("MACHINE");
        	//check to make sure they all got valid data
        	if (myUSER != NULL && myPWD != NULL && myMACHINE != NULL)
                	printf("%s@%s :: %s ->\n", myUSER, myMACHINE, myPWD);
        	else
                	printf("get env returned null");

		int numI = 0;	//number of tokens in instruction
		do{			//reads character sequences separated by whitespace
			scanf("%s", token);
			int i;
			int start;
			start = 0;

			for(i = 0; i < strlen(token); i++){
				if(token[i] == '|' || token[i] == '>' || token[i] == '<' || token[i] == '&'){
					if(i-start > 0){
						memcpy(temp, token + start, i - start);
						temp[i-start] = '\0';
						bucket = addToken(bucket, temp, numI);
						numI++;
					}
	
					char specialChar[2];
					specialChar[0] = token[i];
					specialChar[1] = '\0';

					bucket = addToken(bucket, specialChar, numI);
					numI++;

					start = i + 1;
				}else if(token[i] == 'e' && token[i+1] == 'x' && token[i+2] == 'i' && token[i+3] == 't'){
					printf("You made it!\n");
					exit = 1;
				}
			}
			if(start < strlen(token)){
				memcpy(temp, token + start, strlen(token) - start);
				temp[i-start] = '\0';
				bucket = addToken(bucket, temp, numI);
				numI++;
			}
		}while('\n' != getchar());	//until end of line is reached
		
		printTokens(bucket, numI);
	}	//until "exit" is read in
	free(bucket);		//free dynamic array
	printf("Exiting...''\n");

	return 777;	//Jackpot bb
}

//reallocates instr array to holds another token. Returns new pointer to instr array
//Taken from parser help on canvas
char** addToken(char** instr, char* tok, int numTokens){
	int i;

	char** new_arr;
	new_arr = (char**)malloc((numTokens+1) * sizeof(char*));

	//copy values into new array
	for(i = 0; i < numTokens; i++){
		new_arr[i] = (char*)malloc((strlen(tok)+1) * sizeof(char));
		strcpy(new_arr[i], instr[i]);
	}

	//add new token
	new_arr[numTokens] = (char*)malloc((strlen(tok)+1) * sizeof(char));
	strcpy(new_arr[numTokens], tok);

	if(numTokens > 0)
		free(instr);

	return new_arr;
}

void printTokens(char** instr, int numTokens){
	int i;
	printf("Tokens:\n");
	for(i = 0; i < numTokens; i++)
		printf("#%s#\n", instr[i]);
}

void my_execute(char **cmd) 
	{
	int status;
	pid_t pid = fork();
	if (pid == -1) 
		{
		//Error	
		exit(1);
		}
	else if (pid == 0) 
		{
		//Child
		execv(cmd[0], cmd);
		printf("Problem executing %s\n", cmd[0]);
		exit(1);
		}
	else 
		{
		//Parent
		waitpid(pid, &status, 0);
		}
	}
