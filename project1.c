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
char **resolve_paths(char **args);
char *expand_path(char *path, int cmd_p);
int is_command(char **args, int i);
int expand(char* var);
char* expand_arg(char* str);


int main( int argc, char *argv[] )
        {

        char *myUSER;  //declaring variables need for comandline
        char *myPWD;
        char *myMACHINE;
	char *myHOME;
	char *myROOT;

	//Setting home directory
	myHOME = getenv("HOME");
	//myROOT = getenv("PWD");
	
	
	//The following code is the parser help code from canvas
	char token[256];	//holds instruction token
	char ** bucket;		//array that holds all instruction tokens
	char temp[256];		//used to split tokens containing special characters

	while(7){
		myUSER = getenv("USER");  //ensure everthing is right before running
        	myPWD = getenv("PWD");
        	myMACHINE = getenv("MACHINE");
        	//check to make sure they all got valid data
        	if (myUSER != NULL && myPWD != NULL && myMACHINE != NULL)
                	printf("%s@%s :: %s -> ", myUSER, myMACHINE, myPWD);
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
		bucket = resolve_paths(bucket);
		printTokens(bucket, numI);
		//Resolve paths and execute here
		
	}	//until "exit" is read in
	free(bucket);		//free dynamic array

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




char **resolve_paths(char **args){
	int i;
	for(i = 0; args[i] != NULL; i++){
		args[i] = expand_path(args[i], is_command(args, i));
	}

	return args;
}

int is_command(char **args, int i){
	//returns 0 for argument, 1 for external command, 2 for cd, 3 for other built-in commands
	if(i != 0){
		return 0;
	}
	else if(strcmp(args[i], "cd") == 0){
		return 2;
	}
	else if(strcmp(args[i], "exit") == 0 || strcmp(args[i], "echo") == 0 || strcmp(args[i], "io") == 0){
		return 3;
	}
	else{
		return 1;
	}
}

char *expand_path(char *path, int cmd_p){
	//returns expanded argument, does nothing in most cases (determined by is_command)
	char *userPath;
	char *USER;
	char *currentPath;
	char *homePath;
	char *newPath;
	homePath = getenv("HOME");
	USER = getenv("USER");
	userPath = "/user/";

	int argLen = strlen(path);

	char *path1;
	char *path2;
	char *path3;
	char *path4;
	char *path5;

	switch(cmd_p){
		case 0:
			//arguments expand if they are a file
			//are f
			if(expand(path) == 1)
                		strcpy(path,expand_arg(path));
			else
                		printf("does not need to be expanded\n");
			
			break;
		case 1:
			//external commands must be expanded
			userPath = malloc(strlen(userPath) + strlen(USER) + 1);
			userPath = strcat(userPath, USER);
			userPath = malloc(5 + 1);
			userPath = strcat(userPath, "/bin/");

			homePath = malloc(strlen(homePath) + 9 + 1);
			homePath = strcat(homePath, "/bin/git/");

			path1 = malloc(strlen(homePath) + argLen + 1);
			path2 = malloc(25 + argLen + 1);
			path3 = malloc(5 + argLen + 1);
			path4 = malloc(9 + argLen + 1);
			path5 = malloc(strlen(userPath) + argLen + 1);
			strcpy(path1, homePath);
			path1 = strcat(path1, path);
			strcpy(path2, "/usr/local/bin/");
			path2 = strcat(path2, path);
			strcpy(path3, "/bin/");
			path3 = strcat(path3, path);
			strcpy(path4, "/usr/bin/");
			path4 = strcat(path4, path);
			strcpy(path5, userPath);
			path5 = strcat(path5, path);
			

			if(access(path1, F_OK) == 0){
				newPath = malloc(strlen(path1) + 1);
				strcpy(newPath, path1);
				return newPath;
			}
			else if(access(path2, F_OK) == 0){
				newPath = malloc(strlen(path2) + 1);
				strcpy(newPath, path2);
				return newPath;
			}
			else if(access(path3, F_OK) == 0){
				newPath = malloc(strlen(path3) + 1);
				strcpy(newPath, path3);
				return newPath;
			}
			else if(access(path4, F_OK) == 0){
				newPath = malloc(strlen(path4) + 1);
				strcpy(newPath, path4);
				return newPath;
			}
			else if(access(path5, F_OK) == 0){
				newPath = malloc(strlen(path5) + 1);
				strcpy(newPath, path5);
				return newPath;
			}
			else{
				printf("ERROR: %s is not a valid command.\n", path);
				return path;
			}
			break;
		case 2:
			//cd is the only exception to the rule with built-in commands
			currentPath = getenv("PWD");
			printf("You typed cd!\n");
				return path;
			
			break;
		case 3:
			if(strcmp(path, "exit") == 0){
				printf("Exiting...\n");
				exit(777);
			}
			break;
	}
}

int expand(char* var)
        {
        if(var[0] == '/' || var[0] == '~' || var[0] == '.')
                return 1;
        else
                return 0;
        }

char* expand_arg(char* str)
        {
        char* strm1 = str + 1;          //strings that are missing first and second characte$
        char* strm2 = str + 2;
        char *myPWD = getenv("PWD");
        char *myHOME = getenv("HOME");
        //getting path to directory above
        int end = strlen(myPWD)-1;      //finding end of myPWD used for getting .. later
        int constend = end;
        char check;
        int howfar = 0;
        while (check != '/')            //finds end of myPWD (strlen was giving me the wrong$
                {
                check = myPWD[end];
                end--;
                howfar++;
                }
        int where = constend-howfar+1;  //error checking
        if(str[0] == '/'){}
                //this is a root
        else if(str[0] == '.' && str[1] != '.')
                {
                //relative to current
                strcat(myPWD,strm1);
                strcpy(str,myPWD);
                }
        else if(str[0] == '.' && str[1] == '.')
                {
                //relative to one behind
                char temp[255];
                int i;
                for( i = 0; i < where; i++)//copies myPWD without last directory
                        {
                        temp[i] = myPWD[i];
                        }
		temp[where] = '\0';
                strcat(temp,strm2);
                strcpy(str,temp);
                }
        else if(str[0] == '~')
                {
                //relative to home
                strcat(myHOME,strm1);
                strcpy(str,myHOME);
                }
        else
                {
                //relative to current
                strcat(myPWD,"/");
                strcat(myPWD,str);
                strcpy(str,myPWD);
                }
        printf("%s\n", str);
        return str;
        }

